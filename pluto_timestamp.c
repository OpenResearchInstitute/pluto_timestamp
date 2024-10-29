// SPDX-License-Identifier: MIT-0
/*
 * Hardware Timestamps for ADALM Pluto
 *
 * Copyright (C) 2024 Open Research Institute
 **/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "pluto_timestamp.h"

/* Register addresses */
#define PERIPH_BASE 0xf8f00000
#define GLOBAL_TMR_UPPER_OFFSET 0x0204
#define GLOBAL_TMR_LOWER_OFFSET 0x0200

static volatile uint32_t *timer_register_map;

void initialize_timer_register_access_block()
{
    int ddr_memory = open("/dev/mem", O_RDWR | O_SYNC);
    if (!ddr_memory) {
        printf("Failed to open /dev/mem\n");
        exit(1);
    }

    timer_register_map = mmap(NULL, 65535, PROT_READ | PROT_WRITE, MAP_SHARED, ddr_memory, PERIPH_BASE);
    if (timer_register_map == MAP_FAILED) {
        printf("Failed top map timer registers\n");
        close(ddr_memory);
        exit(1);
    }
}

#ifdef LOCAL_DMA_ROUTINES
unsigned int read_dma(volatile uint32_t *virtual_addr, int offset)
{
    return virtual_addr[offset>>2];
}

unsigned int write_dma(volatile uint32_t *virtual_addr, int offset, unsigned int value)
{
    virtual_addr[offset>>2] = value;
    return 0;
}
#endif

uint64_t get_timestamp(void) {
    uint32_t high, low;

    // Reading global timer counter register
    /* This is the method used in the library code for XTime_GetTime().
       It handles the case where the first read of the two timer regs
       spans a carry between the two timer words. */
    do {
        high = read_dma(timer_register_map, GLOBAL_TMR_UPPER_OFFSET);
        low = read_dma(timer_register_map, GLOBAL_TMR_LOWER_OFFSET);
        // printf("%08x %08x\n", high, low);
    } while (read_dma(timer_register_map, GLOBAL_TMR_UPPER_OFFSET) != high);
    return((((uint64_t) high) << 32U) | (uint64_t) low);
}

#ifdef MAIN
int main() {
    uint64_t t_start, t_end;

    initialize_timer_register_access_block();
    //printf("Initialized register access.\n");

    t_start = get_timestamp();
    //printf("Doing a printf to use up some time between timestamps!\n");
    //usleep(0);    // minimal delay
    usleep(10e6);   // 10 seconds
    t_end = get_timestamp();

    printf("t_start = 0x%016llx, t_end = 0x%016llx\n", t_start, t_end);
    //printf("%llu clock cycles.\n", 2*(t_end - t_start));
    printf("%.2f us.\n", 2.0 * (t_end - t_start) / (COUNTS_PER_SECOND/1000000));

    return 0;
}
#endif