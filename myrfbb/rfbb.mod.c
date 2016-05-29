#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xc54e0a01, "module_layout" },
	{ 0xb78c61e8, "param_ops_bool" },
	{ 0x29537c9e, "alloc_chrdev_region" },
	{ 0x82f776b7, "gpio_export" },
	{ 0x403f9529, "gpio_request_one" },
	{ 0xb38a8029, "cdev_add" },
	{ 0x69c77033, "cdev_init" },
	{ 0xd8e484f0, "register_chrdev_region" },
	{ 0xfe990052, "gpio_free" },
	{ 0xe61a6d2f, "gpio_unexport" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0x68d7dd81, "cdev_del" },
	{ 0xfa2a45e, "__memzero" },
	{ 0xfbc74f64, "__copy_from_user" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x67c2fa54, "__copy_to_user" },
	{ 0x97255bdf, "strlen" },
	{ 0x9d669763, "memcpy" },
	{ 0x43b0c9c3, "preempt_schedule" },
	{ 0x27e1a049, "printk" },
	{ 0x1d2e87c6, "do_gettimeofday" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0x515b9273, "module_put" },
	{ 0x432fd7f6, "__gpio_set_value" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "092440709472516FB8873E4");
