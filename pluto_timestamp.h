// SPDX-License-Identifier: MIT-0
/*
 * Hardware Timestamps for ADALM Pluto
 *
 * Copyright (C) 2024 Open Research Institute
 **/

#include <stdint.h>

/* Global Timer runs on the CPU clock, divided by 2 */
#define COUNTS_PER_SECOND (666666687 / 2)

void initialize_timer_register_access_block();

uint64_t get_timestamp(void);
