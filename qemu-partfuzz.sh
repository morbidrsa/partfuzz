#!/bin/sh

set -e

die()
{
	echo "$*"
	exit 1
}

check_log()
{
	local FILE=$1
	grep -q -e "kernel BUG at" \
		-e "WARNING:" \
		-e "BUG:" \
		-e "Oops:" \
		-e "possible recursive locking detected" \
		-e "Internal error" \
		-e "INFO: suspicious RCU usage" \
		-e "INFO: possible circular locking dependency detected" \
		-e "general protection fault:" \
		"$FILE"
	if [[ $? -eq 0 ]]; then
		return 1 
	else
		return 0;
	fi
}

run_one_test()
{
	local kernel=$1; shift
	local initrd=$1; shift
	local pt=$1

	if [ ! -e $kernel ]; then
		die "Kernel image $kernel not found"
	fi

	if [ ! -e $initrd ]; then
		die "Initrd $initrd not found"
	fi

	FILE=`mktemp /dev/shm/partfuzz.XXXXXX`

	./partfuzz -t "$pt" $FILE || die "enerating partition table in $FILE failed"
	qemu-system-x86_64 -enable-kvm -m 512 -smp 2 -kernel $kernel \
		-initrd $initrd -append "console=ttyS0 init='sh -c \"echo o > /proc/sysrq-trigger\"" -nographic \
		-drive file=$FILE,id=D22,if=none,format=raw \
		-device nvme,drive=D22,serial=1234 \
		-serial mon:stdio | tee $FILE.log

	check_log $FILE.log || die "Test $i failed. Logfile $FILE.log, partition table $FILE"

	rm $FILE $FILE.log
}

main()
{
	if [ $# -ne 2 ]; then
		echo "Usage: `basename $0` kernel initrd"
		exit 1
	fi

	local kernel=$1
	local initrd=$2

	for pt in "ultrix osf sysv68"; do
		for i in `seq 1 10000`; do
			run_one_test $kernel $initrd $pt
		done
	done
}

main $*
