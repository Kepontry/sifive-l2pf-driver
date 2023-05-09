#include <linux/of_address.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cpu.h>

#define SIFIVE_L2_PF_BASIC_CTRL 0x00
#define SIFIVE_L2_PF_ADD_CTRL 0x04
#define NUM_CPUS 4

static u32 reg_basic_ctrl[NUM_CPUS], reg_add_ctrl[NUM_CPUS], temp[NUM_CPUS];
static void __iomem *l2_pf_base[NUM_CPUS];

#define basic_attr_func(name, high, low)					\
static u32 name##_mask = (((1 << (high - low + 1)) - 1) << low);	\
static ssize_t name##_show(struct device *dev,				\
			   struct device_attribute *attr, char *buf)	\
{									\
	reg_basic_ctrl[dev->id] = readl(l2_pf_base[dev->id]		\
		+ SIFIVE_L2_PF_BASIC_CTRL);		\
	return sprintf(buf, "%u\n", (reg_basic_ctrl[dev->id]	\
		& name##_mask) >> low);		\
}									\
static ssize_t name##_store(struct device *dev,		\
			   struct device_attribute *attr,		\
			   const char *buf, size_t size)		\
{									\
	sscanf(buf, "%u", &temp[dev->id]);		\
	reg_basic_ctrl[dev->id] = (reg_basic_ctrl[dev->id] & ~name##_mask)	\
		| ((temp[dev->id] << low) & name##_mask);	\
	writel(reg_basic_ctrl[dev->id], l2_pf_base[dev->id] + SIFIVE_L2_PF_BASIC_CTRL);	\
	return size;	\
}									\
static DEVICE_ATTR_RW(name);

basic_attr_func(prefetch_enable, 0, 0)
basic_attr_func(cross_page_opt_dis, 1, 1)
basic_attr_func(distance, 7, 2)
basic_attr_func(max_allow_dist, 13, 8)
basic_attr_func(line_to_exp_thrd, 19, 14)
basic_attr_func(age_out_enable, 20, 20)
basic_attr_func(num_loads_to_age_out, 27, 21)
basic_attr_func(cross_page_enable, 28, 28)

#define add_attr_func(name, high, low)					\
static u32 name##_mask = (((1 << (high - low + 1)) - 1) << low);	\
static ssize_t name##_show(struct device *dev,				\
			   struct device_attribute *attr, char *buf)	\
{									\
	reg_add_ctrl[dev->id] = readl(l2_pf_base[dev->id]		\
		+ SIFIVE_L2_PF_ADD_CTRL);		\
	return sprintf(buf, "%u\n", (reg_add_ctrl[dev->id]		\
		& name##_mask) >> low);	\
}									\
static ssize_t name##_store(struct device *dev,		\
			   struct device_attribute *attr,		\
			   const char *buf, size_t size)		\
{									\
	sscanf(buf, "%u", &temp[dev->id]);		\
	reg_add_ctrl[dev->id] = (reg_add_ctrl[dev->id] & ~name##_mask)	\
		| ((temp[dev->id] << low) & name##_mask);	\
	writel(reg_add_ctrl[dev->id], l2_pf_base[dev->id] + SIFIVE_L2_PF_ADD_CTRL);	\
	return size;	\
}									\
static DEVICE_ATTR_RW(name);

add_attr_func(q_full_thrd, 3, 0)
add_attr_func(hit_cache_thrd, 8, 4)
add_attr_func(hit_mshr_thrd, 12, 9)
add_attr_func(window, 18, 13)

static struct attribute *l2_prefetch_attrs[] = {
	&dev_attr_prefetch_enable.attr,
	&dev_attr_cross_page_opt_dis.attr,
	&dev_attr_distance.attr,
	&dev_attr_max_allow_dist.attr,
	&dev_attr_line_to_exp_thrd.attr,
	&dev_attr_age_out_enable.attr,
	&dev_attr_num_loads_to_age_out.attr,
	&dev_attr_cross_page_enable.attr,
	&dev_attr_q_full_thrd.attr,
	&dev_attr_hit_cache_thrd.attr,
	&dev_attr_hit_mshr_thrd.attr,
	&dev_attr_window.attr,
	NULL,
};

static const struct attribute_group l2_prefetch_attr_group = {
	.attrs = l2_prefetch_attrs,
	.name = "l2_prefetch"
};

static int l2_prefetch_add_dev(unsigned int cpu)
{
	struct device_node *np;
	char buf[10];
	
	sprintf(buf, "l2pf%u", cpu);
	np = of_find_node_by_name(NULL, buf);
	if (!np)
		return -ENODEV;

	l2_pf_base[cpu] = of_iomap(np, 0);
	if (!l2_pf_base[cpu])
		return -ENOMEM;

	reg_basic_ctrl[cpu] = readl(l2_pf_base[cpu] + SIFIVE_L2_PF_BASIC_CTRL);
	reg_add_ctrl[cpu] = readl(l2_pf_base[cpu] + SIFIVE_L2_PF_ADD_CTRL);
	
	struct device *dev = get_cpu_device(cpu);
	if(cpu != dev->id)
		pr_err("L2PF: cpu %u != dev_id %u", cpu, dev->id);
	return sysfs_create_group(&dev->kobj, &l2_prefetch_attr_group);
}

static int l2_prefetch_remove_dev(unsigned int cpu)
{
	struct device *dev = get_cpu_device(cpu);
	sysfs_remove_group(&dev->kobj, &l2_prefetch_attr_group);
	return 0;
}

static int __init sifive_l2_pf_init(void)
{
	return cpuhp_setup_state(CPUHP_L2PREFETCH_PREPARE,
				 "soc/l2prefetch:prepare", l2_prefetch_add_dev,
				 l2_prefetch_remove_dev);
}

device_initcall(sifive_l2_pf_init);
