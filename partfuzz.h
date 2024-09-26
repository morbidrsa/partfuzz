/*
 * Copyright (c) 2016-2024 Johannes Thumshirn <jth@kernel.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _PARTFUZZ_H
#define _PARTFUZZ_H

#include "byteorder.h"

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

enum partition_type {
	ULTRIX_PARTITION_TYPE,
	OSF_PARTITION_TYPE,
	SYSV68_PARTITION_TYPE,
};

struct partition_table {
	size_t size;
	void *part;
	unsigned int sector;
	unsigned int offset;
};

struct {
	bool dbg;
} cfg;

#define debug(cfg, fmt, ...) do {		\
	if (cfg.dbg)				\
		printf(fmt, __VA_ARGS__);	\
} while(0)

struct partition_table *generate_osf_partition(void);
struct partition_table *generate_sysv68_partition(void);
struct partition_table *generate_ultrix_partition(void);

#endif
