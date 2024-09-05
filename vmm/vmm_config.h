/*
 * Copyright 2024, DornerWorks
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#pragma once

#include <microkit.h>

#define GUEST_RAM_SIZE 0x40000000

#define GUEST_DTB_VADDR           0x820000000
#define GUEST_INIT_RAM_DISK_VADDR 0x820100000

#define MAX_IRQS 3

struct mk_irq {
    int irq;
    microkit_channel channel;
};

struct mk_irq mk_irqs[MAX_IRQS] = {
    // Serial
    {
        .irq = 53,
        .channel = 1,
    },
    // Ethernet
    {
        .irq = 95,
        .channel = 2,
    },
    // MMC
    {
        .irq = 81,
        .channel = 3,
    },
};
