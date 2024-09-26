/*
 * Copyright (c) 2016 - 2024 Johannes Thumshirn <jth@kernel.org>
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

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>

#include "partfuzz.h"

struct ultrix_disklabel {
#define ULTRIX_PARTITION_MAGIC 0x032957
#define ULTRIX_PARTITION_VALID 1
	int32_t pt_magic;
	int32_t pt_valid;
	struct pt_info {
		int32_t		pi_nblocks;
		uint32_t	pi_blkoff;
	} pt_part[8];
} __attribute__((packed));

struct partition_table *generate_ultrix_partition(struct pf_ctx ctx)
{
	struct partition_table *table;
	struct ultrix_disklabel *label;
	size_t alloc_size;
	int numparts;
	int i;

	alloc_size = sizeof(struct partition_table);
	table = calloc(alloc_size, 1);
	if (!table) {
		errno = ENOMEM;
		return NULL;
	}

	table->size = sizeof(struct ultrix_disklabel);
	table->sector = (16384 - table->size)/512;
	table->offset = 512 - table->size;

	label = calloc(sizeof(struct ultrix_disklabel), 1);
	if (!label)
		goto free_table;

	label->pt_magic = ULTRIX_PARTITION_MAGIC;
	label->pt_valid = ULTRIX_PARTITION_VALID;
	debug(ctx, "label->pt_magic: 0x%x, label->pt_valid: %d\n",
	      label->pt_magic, label->pt_valid);

	numparts = rand() % 8;

	for (i = 0; i < numparts; i++) {
		label->pt_part[i].pi_nblocks = rand() % INT_MAX;
		debug(ctx, "label->pt_part[%d].pi_nblocks: 0x%08x (%d)\n",
		      i,label->pt_part[i].pi_nblocks,
		      label->pt_part[i].pi_nblocks);
		label->pt_part[i].pi_blkoff = rand() % UINT_MAX;
		debug(ctx, "label->pt_part[%d].pi_blkoff: 0x%08x (%u)\n",
		      i, label->pt_part[i].pi_blkoff,
		      label->pt_part[i].pi_blkoff);
	}

	table->part = label;

	return table;

free_table:
	free(table);
	return 0;
}
