This is an example driver code for l2 prefetcher of StarFive jh7110 SoC (sifive U74 core inside) and is tested on the StarFive VisionFive2 SBC.

To use the driver, you should

1. Add device description to dts file.

```shell
$ cd linux
$ vim arch/riscv/boot/dts/starfive/jh7110.dtsi
soc: soc {
				......
                cachectrl: cache-controller@2010000 {
				......
                };

+               l2pf0: l2pf0@2032000 {
+                       compatible = "sifive,l2pf";
+                       reg = <0x0 0x2032000 0x0 0x2000>;
+               };
+
+               l2pf1: l2pf1@2034000 {
+                       compatible = "sifive,l2pf";
+                       reg = <0x0 0x2034000 0x0 0x2000>;
+               };
+
+               l2pf2: l2pf2@2036000 {
+                       compatible = "sifive,l2pf";
+                       reg = <0x0 0x2036000 0x0 0x2000>;
+               };
+
+               l2pf3: l2pf3@2038000 {
+                       compatible = "sifive,l2pf";
+                       reg = <0x0 0x2038000 0x0 0x2000>;
+               };
```

2. Add the `CPUHP_L2PREFETCH_PREPARE` flag to `cpuhotplug.h`.

```shell
$ vim include/linux/cpuhotplug.h
        CPUHP_TOPOLOGY_PREPARE,
+       CPUHP_L2PREFETCH_PREPARE,
        CPUHP_NET_IUCV_PREPARE,
```

3. Copy the `sifive_l2_prefetcher.c` in this repo to the driver directory.

```shell
$ cp sifive_l2_prefetcher.c $(YOUR_LINUX_DIR)/drivers/soc/sifive/
```

4. Add the `sifive_l2_prefetcher.o` to the Makefile.

```shell
$ vim $(YOUR_LINUX_DIR)/drivers/soc/sifive/Makefile
	# SPDX-License-Identifier: GPL-2.0

	obj-$(CONFIG_SIFIVE_L2)	+= sifive_l2_cache.o
+	obj-$(CONFIG_SIFIVE_L2)	+= sifive_l2_prefetcher.o
```

5. Compile the kernel and replace the vmlinuz file.

6. Do read/write tests of the parameters of the l2 prefetcher.

```shell
# cat /sys/devices/system/cpu/cpu0/l2_prefetch/prefetch_enable 
1
# echo "0" > /sys/devices/system/cpu/cpu0/l2_prefetch/prefetch_enable 
# cat /sys/devices/system/cpu/cpu0/l2_prefetch/prefetch_enable 
0
# cat /sys/devices/system/cpu/cpu1/l2_prefetch/prefetch_enable 
1
```

There are some references you can refer to if you have any troubles.

Kernel compile: https://gitee.com/tinylab/riscv-linux/blob/master/articles/20230227-vf2-kernel-compile.md
...
