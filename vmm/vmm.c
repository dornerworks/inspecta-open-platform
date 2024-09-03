/*
 * Copyright 2023, UNSW
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <stddef.h>
#include <stdint.h>
#include <microkit.h>
#include <libvmm/util/util.h>
#include <libvmm/arch/aarch64/vgic/vgic.h>
#include <libvmm/arch/aarch64/linux.h>
#include <libvmm/arch/aarch64/fault.h>
#include <libvmm/arch/aarch64/smc.h>
#include <libvmm/guest.h>
#include <libvmm/virq.h>
#include <libvmm/tcb.h>
#include <libvmm/vcpu.h>

// @ivanv: ideally we would have none of these hardcoded values
// initrd, ram size come from the DTB
// We can probably add a node for the DTB addr and then use that.
// Part of the problem is that we might need multiple DTBs for the same example
// e.g one DTB for VMM one, one DTB for VMM two. we should be able to hide all
// of this in the build system to avoid doing any run-time DTB stuff.

/*
 * As this is just an example, for simplicity we just make the size of the
 * guest's "RAM" the same for all platforms. For just booting Linux with a
 * simple user-space, 0x10000000 bytes (256MB) is plenty.
 */
#define GUEST_RAM_SIZE 0x10000000

#define GUEST_DTB_VADDR 0x80f000000
#define GUEST_INIT_RAM_DISK_VADDR 0x80d700000

/* For simplicity we just enforce the serial IRQ channel number to be the same
 * across platforms. */
#define SERIAL_IRQ_CH 1
#define SERIAL_IRQ 53

#define ETHERNET_IRQ_CH 2
#define ETHERNET_IRQ 95

#define MMC_IRQ_CH 3
#define MMC_IRQ 81

/* Data for the guest's kernel image. */
extern char _guest_kernel_image[];
extern char _guest_kernel_image_end[];
/* Data for the device tree to be passed to the kernel. */
extern char _guest_dtb_image[];
extern char _guest_dtb_image_end[];
/* Data for the initial RAM disk to be passed to the kernel. */
extern char _guest_initrd_image[];
extern char _guest_initrd_image_end[];
/* Microkit will set this variable to the start of the guest RAM memory region. */
uintptr_t guest_ram_vaddr;

static void serial_ack(size_t vcpu_id, int irq, void *cookie) {
    /*
     * For now we by default simply ack the serial IRQ, we have not
     * come across a case yet where more than this needs to be done.
     */
    microkit_irq_ack(SERIAL_IRQ_CH);
}

static void ethernet_ack(size_t vcpu_id, int irq, void *cookie) {
    /*
     * For now we by default simply ack the ethernet IRQ, we have not
     * come across a case yet where more than this needs to be done.
     */
    microkit_irq_ack(ETHERNET_IRQ_CH);
}

static void mmc_ack(size_t vcpu_id, int irq, void *cookie) {
    /*
     * For now we by default simply ack the mmc IRQ, we have not
     * come across a case yet where more than this needs to be done.
     */
    microkit_irq_ack(MMC_IRQ_CH);
}

void init(void) {
    /* Initialise the VMM, the VCPU(s), and start the guest */
    LOG_VMM("starting \"%s\"\n", microkit_name);
    /* Place all the binaries in the right locations before starting the guest */
    size_t kernel_size = _guest_kernel_image_end - _guest_kernel_image;
    size_t dtb_size = _guest_dtb_image_end - _guest_dtb_image;
    size_t initrd_size = _guest_initrd_image_end - _guest_initrd_image;
    uintptr_t kernel_pc = linux_setup_images(guest_ram_vaddr,
                                      (uintptr_t) _guest_kernel_image,
                                      kernel_size,
                                      (uintptr_t) _guest_dtb_image,
                                      GUEST_DTB_VADDR,
                                      dtb_size,
                                      (uintptr_t) _guest_initrd_image,
                                      GUEST_INIT_RAM_DISK_VADDR,
                                      initrd_size
                                      );
    if (!kernel_pc) {
        LOG_VMM_ERR("Failed to initialise guest images\n");
        return;
    }
    /* Initialise the virtual GIC driver */
    bool success = virq_controller_init(GUEST_VCPU_ID);
    if (!success) {
        LOG_VMM_ERR("Failed to initialise emulated interrupt controller\n");
        return;
    }
    /* Initialise the SMC SIP Handler */
    success = smc_register_sip_handler(smc_sip_forward);
    if (!success) {
        LOG_VMM_ERR("Failed to initialise SMC SIP Handler\n");
        return;
    }
    success = virq_register(GUEST_VCPU_ID, SERIAL_IRQ, &serial_ack, NULL);
    success = virq_register(GUEST_VCPU_ID, ETHERNET_IRQ, &ethernet_ack, NULL);
    success = virq_register(GUEST_VCPU_ID, MMC_IRQ, &mmc_ack, NULL);
    /* Just in case there are already interrupts available to handle, we ack them here. */
    microkit_irq_ack(SERIAL_IRQ_CH);
    microkit_irq_ack(ETHERNET_IRQ_CH);
    microkit_irq_ack(MMC_IRQ_CH);
    /* Finally start the guest */
    guest_start(GUEST_VCPU_ID, kernel_pc, GUEST_DTB_VADDR, GUEST_INIT_RAM_DISK_VADDR);
}

void notified(microkit_channel ch) {
    switch (ch) {
        case SERIAL_IRQ_CH: {
            bool success = virq_inject(GUEST_VCPU_ID, SERIAL_IRQ);
            if (!success) {
                LOG_VMM_ERR("IRQ %d dropped on vCPU %d\n", SERIAL_IRQ, GUEST_VCPU_ID);
            }
            break;
        }
        case ETHERNET_IRQ_CH: {
            bool success = virq_inject(GUEST_VCPU_ID, ETHERNET_IRQ);
            if (!success) {
                LOG_VMM_ERR("IRQ %d dropped on vCPU %d\n", ETHERNET_IRQ, GUEST_VCPU_ID);
            }
            break;
        }
        case MMC_IRQ_CH: {
            bool success = virq_inject(GUEST_VCPU_ID, MMC_IRQ);
            if (!success) {
                LOG_VMM_ERR("IRQ %d dropped on vCPU %d\n", MMC_IRQ, GUEST_VCPU_ID);
            }
            break;
        }
        default:
            printf("Unexpected channel, ch: 0x%lx\n", ch);
    }
}

/*
 * The primary purpose of the VMM after initialisation is to act as a fault-handler.
 * Whenever our guest causes an exception, it gets delivered to this entry point for
 * the VMM to handle.
 */
seL4_Bool fault(microkit_child child, microkit_msginfo msginfo, microkit_msginfo *reply_msginfo) {
    bool success = fault_handle(child, msginfo);
    if (success) {
        /* Now that we have handled the fault successfully, we reply to it so
         * that the guest can resume execution. */
        *reply_msginfo = microkit_msginfo_new(0, 0);
        return seL4_True;
    }

    return seL4_False;
}
