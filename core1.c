/*
 * Copyright 2025, UNSW
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <stdint.h>
#include <microkit.h>

#define print(str) do { microkit_dbg_puts(microkit_name); microkit_dbg_puts(": "); microkit_dbg_puts(str); } while (0)

void init(void)
{
    // volatile int i = 0;
    // for (i = 0; i < 10000000; i++);
    print("hello, world (from core 1)\n");
}

int notified_count = 100;

void notified(microkit_channel ch)
{
    if (ch == 0) {
        if (notified_count > 0) {
            print("vmm got an irq: notified: ");
            microkit_dbg_put32(ch);
            microkit_dbg_puts(" (cross core)\n");
            notified_count--;
            if (notified_count == 0) {
                print("stopping after 100 notifications\n");
            }
        }
    } else {
        microkit_dbg_puts(" (unknown)\n");
    }
}
