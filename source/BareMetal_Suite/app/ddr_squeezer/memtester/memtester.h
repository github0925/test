/*
 * Very simple (yet, for some reason, very effective) memory tester.
 * Originally by Simon Kirby <sim@stormix.com> <sim@neato.org>
 * Version 2 by Charles Cazabon <charlesc-memtester@pyropus.ca>
 * Version 3 not publicly released.
 * Version 4 rewrite:
 * Copyright (C) 2004-2012 Charles Cazabon <charlesc-memtester@pyropus.ca>
 * Licensed under the terms of the GNU General Public License version 2 (only).
 * See the file COPYING for details.
 *
 * This file contains the declarations for external variables from the main file.
 * See other comments in that file.
 *
 */

#ifndef _MEMTESTER_H_
#define _MEMTESTER_H_

#include <common_hdr.h>
#include <debug.h>
#include <string.h>
#include <stdlib.h>
#include <arch.h>
#include "types.h"
#include "tests.h"
#include "sizes.h"

typedef struct {
    addr_t start;
    size_t sz;
} mem_range_t;

extern int memtester_main(void *args, uint32_t loops);

#endif
