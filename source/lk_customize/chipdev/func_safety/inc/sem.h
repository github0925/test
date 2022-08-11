/*
 * sem.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef __SEM_H__
#define __SEM_H__

#include <stdbool.h>

/*
 * SEM1 and SEM2 are the same IPs with same input signals. The
 * differences are:
 *  1) SEM2 does not have monitor input.
 *  2) SEM2 has only one output interrupt: SEM_INTR_CPU.
 *  3) SEM2 does not control the global enable signal of EIC modules.
 */
enum sem {
    SEM1 = 0,
    SEM2,
};

/* SEM interrupt output targets. */
enum sem_intr {
    SEM_INTR_PAD0 = 0, /* PAD0, recoverable error. */
    SEM_INTR_PAD1,     /* PAD1, fatal error. */
    SEM_INTR_CPU,      /* GIC 1~5. */
    SEM_INTR_SYS,      /* global reset. */
    SEM_INTR_DC0,      /* DC output 0. */
    SEM_INTR_DC1,      /* DC output 1. */
    SEM_INTR_DC2,      /* DC output 2. */
    SEM_INTR_DC3,      /* DC output 3. */
};

enum sem_signal;
enum sem_monitor_sig;

/* Generated SEM input signals, and monitor signals. */
#include "sem_hw.h"

void sem_enable_intr(enum sem sem, enum sem_intr intr, bool enable);
void sem_trigger_intr(enum sem sem, enum sem_intr intr);
bool sem_signal_status(enum sem sem, enum sem_signal input);
void sem_map_signal(enum sem sem, enum sem_signal input,
                    enum sem_intr intr, bool enable);

void sem_mon_enable(bool enable);
void sem_mon_enable_intr(enum sem_intr intr, bool enable);
void sem_mon_compare_sig(enum sem_monitor_sig signal,
                         bool compare, bool target);

void sem_enable_eic(bool enable);

#endif
