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

struct osf_disklabel {
#define MAX_OSF_PARTITIONS 18
#define OSF_MAGIC 0x82564557UL
	uint32_t magic;
	uint16_t type;
	uint16_t subtype;
	uint8_t typename[16];
	uint8_t packname[16];
	uint32_t secsize;
	uint32_t nsectors;
	uint32_t ntracks;
	uint32_t ncylinders;
	uint32_t secpercyl;
	uint32_t secprtunit;
	uint16_t sparespertrack;
	uint16_t sparespercyl;
	uint32_t acylinders;
	uint16_t rpm;
	uint16_t interleave;
	uint16_t trackskew;
	uint16_t cylskew;
	uint32_t headswitch;
	uint32_t trkseek;
	uint32_t flags;
	uint32_t drivedata[5];
	uint32_t spare[5];
	uint32_t magic2;
	uint16_t checksum;
	uint16_t npartitions;
	uint32_t bbsize;
	uint32_t sbsize;
	struct osf_partition {
		uint32_t size;
		uint32_t offset;
		uint32_t fsize;
		uint8_t  fstype;
		uint8_t  frag;
		uint16_t cpg;
	} partitions[MAX_OSF_PARTITIONS];


} __attribute__((packed));

struct partition_table *generate_osf_partition(struct pf_ctx ctx)
{
	struct partition_table *table;
	struct osf_disklabel *label;
	int numparts;
	int i;

	(void)ctx;

	table = calloc(sizeof(struct partition_table), 1);
	if (!table) {
		errno = ENOMEM;
		return NULL;
	}

	table->size = sizeof(struct osf_disklabel);
	table->sector = 0;
	table->offset = 64;

	label = calloc(table->size, 1);
	if (!label)
		goto free_table;

	label->magic = OSF_MAGIC;
	label->type = rand() % USHRT_MAX;
	label->subtype = rand() % USHRT_MAX;
	memcpy(label->typename, "PARTFUZZ", sizeof(label->typename));
	memcpy(label->packname, "PARTFUZZ", sizeof(label->packname));
	label->secsize = rand() % UINT_MAX;
	label->nsectors = rand() % UINT_MAX;
	label->ntracks = rand() % UINT_MAX;
	label->ncylinders = rand() % UINT_MAX;
	label->secpercyl = rand() % UINT_MAX;
	label->secprtunit = rand() % UINT_MAX;
	label->sparespertrack = rand() % USHRT_MAX;
	label->sparespercyl = rand() % USHRT_MAX;
	label->acylinders = rand() % UINT_MAX;
	label->rpm = rand() % UINT_MAX;
	label->interleave = rand() % UINT_MAX;
	label->trackskew = rand() % UINT_MAX;
	label->cylskew = rand() % UINT_MAX;
	label->headswitch = rand() % UINT_MAX;
	label->trkseek = rand() % UINT_MAX;
	label->flags = rand() % UINT_MAX;
	for (i = 0; i < 5; i++)
		label->drivedata[i] = rand() % UINT_MAX;
	for (i = 0; i < 5; i++)
		label->spare[i] = rand() % UINT_MAX;
	label->magic2 = OSF_MAGIC;
	label->checksum = rand() % UINT_MAX; /* TODO: Generate a valid checksum */
	label->npartitions = rand() % MAX_OSF_PARTITIONS;
	label->bbsize = rand() % UINT_MAX;
	label->sbsize = rand() % UINT_MAX;

	numparts = rand() % label->npartitions;

	for (i = 0; i < numparts; i++) {
		label->partitions[i].size = rand() % UINT_MAX;
		label->partitions[i].offset = rand() % UINT_MAX;
		label->partitions[i].fsize = rand() % UINT_MAX;
		label->partitions[i].fstype = rand() % UCHAR_MAX;
		label->partitions[i].frag = rand() % UCHAR_MAX;
		label->partitions[i].cpg = rand() % USHRT_MAX;
	}

	table->part = label;

	return table;

free_table:
	free(table);
	return NULL;
}

