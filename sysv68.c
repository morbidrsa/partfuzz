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
#include <string.h>

#include "partfuzz.h"

struct sysv68_dkblk0 {
	struct volumeid {
		uint8_t vid_unused[248];
		uint8_t vid_mac[8]; /* ASCII string "MOTOROLA" */
	} dk_vid;
	struct dkconfig {
		uint8_t ios_unused0[128];
		uint32_t ios_slcblk;
		uint16_t ios_slccnt;
		uint8_t ios_unused1[122];
	} dk_ios;
	struct sysv68_slice {
		uint32_t nblocks;
		uint32_t blkoff;
	} slices[0];
} __attribute__((packed));

struct partition_table *generate_sysv68_partition(struct pf_ctx ctx)
{
	struct partition_table *table;
	struct sysv68_dkblk0 *label;
	struct volumeid *dk_vid;
	uint16_t slices;
	uint16_t slices2;
	size_t alloc_size;
	uint32_t slice_offset;
	int i;

	(void) ctx;

	table = calloc(sizeof(struct partition_table), 1);
	if (!table) {
		errno = ENOMEM;
		return NULL;
	}

	slices = rand() % USHRT_MAX;

	slice_offset = 512 + sizeof(struct sysv68_dkblk0);
	alloc_size = sizeof(struct sysv68_dkblk0) + slice_offset +
		     (sizeof(struct sysv68_slice) * slices);
	label = calloc(alloc_size, 1);
	if (!label) {
		errno = ENOMEM;
		goto out_free_table;


	}

	dk_vid = &label->dk_vid;
	memcpy(dk_vid->vid_mac, "MOTOROLA", sizeof(dk_vid->vid_mac));

	label->dk_ios.ios_slccnt = cpu_to_be16(slices);
	label->dk_ios.ios_slcblk = cpu_to_be32(1);

	slices2 = rand() % slices;
	for (i = 0; i < slices2; i++) {
		label->slices[i].nblocks = cpu_to_be32(rand() % UINT_MAX);
		label->slices[i].blkoff = cpu_to_be32(rand() % UINT_MAX);
	}

	return table;

out_free_table:
	free(table);
	return NULL;
}
