/*
 * Copyright (c) 2016 Johannes Thumshirn <jthumshirn@suse.de>
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


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>

extern int errno;
extern int optind;

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

enum partition_type {
	ULTRIX_PARTITION_TYPE,
};

static struct {
	enum partition_type ptype;
	char *name;
} pt_table[] = {
	{ .name = "ultrix", .ptype = ULTRIX_PARTITION_TYPE },
};

struct partition_table {
	size_t size;
	void *part;
	unsigned int offset;
};

static struct {
	bool dbg;
} cfg;

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#define debug(cfg, fmt, ...) do {		\
	if (cfg.dbg)				\
		printf(fmt, __VA_ARGS__);	\
} while(0)


static struct partition_table *generate_ultrix_partition(void)
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
	table->offset = ((16384 - table->size) / 512) + 512 - table->size;

	label = calloc(sizeof(struct ultrix_disklabel), 1);
	if (!label)
		goto free_table;

	label->pt_magic = ULTRIX_PARTITION_MAGIC;
	label->pt_valid = ULTRIX_PARTITION_VALID;
	debug(cfg, "label->pt_magic: 0x%x, label->pt_valid: %d\n",
	      label->pt_magic, label->pt_valid);

	numparts = rand() % 8;

	for (i = 0; i < numparts; i++) {
		label->pt_part[i].pi_nblocks = rand() % INT_MAX;
		debug(cfg, "label->pt_part[%d].pi_nblocks: 0x%08x (%d)\n",
		      i,label->pt_part[i].pi_nblocks,
		      label->pt_part[i].pi_nblocks);
		label->pt_part[i].pi_blkoff = rand() % UINT_MAX;
		debug(cfg, "label->pt_part[%d].pi_blkoff: 0x%08x (%u)\n",
		      i, label->pt_part[i].pi_blkoff,
		      label->pt_part[i].pi_blkoff);
	}

	table->part = label;

	return table;

free_table:
	free(table);
	return 0;
}

static const char *part2str(enum partition_type ptype)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(pt_table); i++)
		if (pt_table[i].ptype == ptype)
			return pt_table[i].name;

	return "UNKNOWN";
}

static struct partition_table *generate_partition(enum partition_type ptype)
{
	debug(cfg, "generating %s partition\n", part2str(ptype));

	switch (ptype) {
	case ULTRIX_PARTITION_TYPE:
		return generate_ultrix_partition();
	default:
		return NULL;
	}
}

static enum partition_type str2part(const char *name)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(pt_table); i++)
		if (!strncmp(name, pt_table[i].name, strlen(pt_table[i].name)))
			return pt_table[i].ptype;

	return ULTRIX_PARTITION_TYPE;
}

static struct option options[] = {
	{ "debug",		no_argument,       NULL, 'd'},
	{ "partition-type",	required_argument, NULL, 't'},
	{ "help",               no_argument,       NULL, 'h'},
	{ NULL,    0,           NULL,   0 },
};

static void usage(const char *pname)
{
	unsigned int i;

	printf("Usage: %s [options] tmpfile\n", pname);
	printf("\n");
	printf("\t-d --debug          enable debugging\n");
	printf("\t-t --partition-type generate a specific partition\n");
	printf("\tvalid partition types are:\n");
	for (i = 0; i < ARRAY_SIZE(pt_table); ++i)
		printf("\t    %s\n", pt_table[i].name);

}

int main(int argc, char **argv)
{
	char *tmpfile;
	char *progname;
	struct partition_table *partition;
	enum partition_type ptype = ULTRIX_PARTITION_TYPE;
	ssize_t written;
	int opt;
	int fd;

	srand(time(NULL));

	progname = basename(argv[0]);

	while (1) {
		opt = getopt_long(argc, argv, "dt:h", options, NULL);
		if (opt < 1)
			break;

		switch (opt) {
		case 'd':
			cfg.dbg = 1;
			break;
		case 't':
			ptype = str2part(optarg);
			break;
		case 'h':
		default:
			usage(progname);
			return 1;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc != 1) {
		usage(progname);
		return 1;
	}

	tmpfile = argv[0];

	fd = open(tmpfile, O_RDWR);
	if (fd < 0) {
		perror("open()");
		return 1;
	}

	partition = generate_partition(ptype);
	if (!partition) {
		perror("generate_partition()");
		goto out_close;
	}

	debug(cfg, "partition->offset: %d\n", partition->offset);
	debug(cfg, "partition->size: %lu\n", partition->size);
	lseek(fd, partition->offset, SEEK_SET);

	written = write(fd, partition->part, partition->size);
	if (written < 0) {
		perror("write()");
		goto out_free_part;
	}

out_free_part:
	free(partition->part);
	free(partition);
out_close:
	close(fd);

	return 0;
}
