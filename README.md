# Partfuzz 
Partfuzz is a partition table fuzzer intended to test the partition scanning
code in the Linux kernel.

It currently supports the following partition types:
1. Ultrix
2. OSF

A sample shell script to pass a generated disk image to Qemu is provided in
the qemu-partfuzz.sh shell script.

You can run it as follows:

## Generate a test.sh to be used in the VM

~~~bash
#!/bin/sh

set -x

modprobe nvme
sleep 10
lsblk
echo o > /proc/sysrq-trigger
~~~

## Build the kernel and modules

```
$ make -j `getconf _NPROCESSORS_ONLN` && make INSTALL_MOD_PATH=mods/
```

## Use dracut to build an initrd containing the modules and test.sh

```
$ dracut --no-compress --kver `make kernelrelease` --kmoddir mods/ --no-hostonly
--no-hostonly-cmdline --modules "bash base" --tmpdir `pwd`/myinitrd --force
myinitrd/initrd --add-drivers nvme --install "lsblk fdisk" --include
"`pwd`/test.sh" "/.profile"
```

## Start qemu-partfuzz.sh

```
$ qemu-partfuzz.sh ~/src/linux/arch/x86/boot/bzImage ~/src/linux/myinitrd/initrd
```
