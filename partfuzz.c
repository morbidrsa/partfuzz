/*
 * Copyright (c) 2016, 2017 Johannes Thumshirn <jthumshirn@suse.de>
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

#define _POSIX_C_SOURCE 200112L
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/loop.h>
#include <fcntl.h>
#include <getopt.h>

#include "partfuzz.h"

extern int errno;
extern int optind;

static struct {
	enum partition_type ptype;
	char *name;
} pt_table[] = {
	{ .name = "ultrix", .ptype = ULTRIX_PARTITION_TYPE },
	{ .name = "osf", .ptype = OSF_PARTITION_TYPE },
	{ .name = "sysv68", .ptype = SYSV68_PARTITION_TYPE },
};

static const char *part2str(enum partition_type ptype)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(pt_table); i++)
		if (pt_table[i].ptype == ptype)
			return pt_table[i].name;

	return "UNKNOWN";
}

static struct partition_table *generate_partition(struct pf_ctx ctx,
						  enum partition_type ptype)
{
	debug(ctx, "generating %s partition\n", part2str(ptype));

	switch (ptype) {
	case ULTRIX_PARTITION_TYPE:
		return generate_ultrix_partition(ctx);
	case OSF_PARTITION_TYPE:
		return generate_osf_partition(ctx);
	case SYSV68_PARTITION_TYPE:
		return generate_sysv68_partition(ctx);
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
	{ "seed",		required_argument, NULL, 's'},
	{ "iterations",		required_argument, NULL, 'i'},
	{ "help",               no_argument,       NULL, 'h'},
	{ NULL,    0,           NULL,   0 },
};

static void usage(const char *pname)
{
	unsigned int i;

	printf("Usage: %s [options] tmpfile\n", pname);
	printf("\n");
	printf("\t-d --debug          enable debugging\n");
	printf("\t-s --seed           set randomization seed to argument\n");
	printf("\t-i --iterations     set number of iterations to argument (default: 1)\n");
	printf("\t-t --partition-type generate a specific partition (default: ultrix)\n");
	printf("\tvalid partition types are:\n");
	for (i = 0; i < ARRAY_SIZE(pt_table); ++i)
		printf("\t    %s\n", pt_table[i].name);

}

static int loop_mount_partition(int memfd)
{
	struct loop_config loop_config = { };
	int ret;
	int loopfd;

	loopfd = open("/dev/loop0", O_RDWR);
	if (loopfd < 0) {
		perror("open() loopfd");
		return -1;
	}

	loop_config.fd = memfd;
	loop_config.block_size = 512;
	loop_config.info.lo_flags = LO_FLAGS_READ_ONLY | LO_FLAGS_PARTSCAN;

	ret = ioctl(loopfd, LOOP_CONFIGURE, &loop_config);
	if (ret < 0) {
		perror("ioctl() LOOP_CONFIGURE");
		goto out_close_loopfd;
	}

	ret = ioctl(loopfd, LOOP_CLR_FD, 0);
	if (ret < 0) {
		  perror("ioctl() LOOP_CTL_REMOVE");
		  goto out_close_loopfd;
	}

out_close_loopfd:
	close(loopfd);

	return ret;
}

static int write_part_to_memfd(struct pf_ctx ctx,
                               struct partition_table *partition)
{
	ssize_t written;
	off_t offset;
	off_t disksize = 2199023255552; /* 2TB */
	int memfd;

	memfd = memfd_create("partfuzz", 0);
	if (memfd < 0) {
		perror("memfd_create()");
		return -1;
	}

	if (ftruncate(memfd, disksize)) {
		perror("ftruncate()");
		goto out;
	}

	debug(ctx, "partition->offset: %d\n", partition->offset);
	debug(ctx, "partition->size: %lu\n", partition->size);
	offset = (partition->sector * 512) + partition->offset;
	lseek(memfd, offset, SEEK_SET);

	written = write(memfd, partition->part, partition->size);
	if (written < 0) {
		perror("write()");
		goto out;
	}

	return memfd;

out:
	close(memfd);
	return -1;
}

int main(int argc, char **argv)
{
	char *progname;
	struct partition_table *partition = NULL;
	enum partition_type ptype = ULTRIX_PARTITION_TYPE;
	struct pf_ctx ctx = { };
	int opt;
	int memfd;
	unsigned int seed = time(NULL);
	int ret = 0;
	int iterations = 1;

	progname = basename(argv[0]);

	while (1) {
		opt = getopt_long(argc, argv, "dt:s:h", options, NULL);
		if (opt < 1)
			break;

		switch (opt) {
		case 'd':
			ctx.dbg = 1;
			break;
		case 't':
			ptype = str2part(optarg);
			break;
		case 's':
			seed = atoi(optarg);
			break;
		case 'i':
			iterations = atoi(optarg);
			break;
		case 'h':
		default:
			usage(progname);
			return 1;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc != 0) {
		usage(progname);
		return 1;
	}

	debug(ctx, "seed=%d\n", seed);
	srand(seed);

	for (int i = 0; i < iterations; i++) {
		partition = generate_partition(ctx, ptype);
		if (!partition) {
			perror("generate_partition()");
			return 1;
		}

		memfd = write_part_to_memfd(ctx, partition);
		if (memfd < 1) {
			ret = memfd;
			goto out_free_part;
		}

		ret = loop_mount_partition(memfd);
	}

out_free_part:
	free(partition->part);
	free(partition);
	close(memfd);

	return ret;
}
