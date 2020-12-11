#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x7377b0b2, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x2ecda6f9, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0xd2b09ce5, __VMLINUX_SYMBOL_STR(__kmalloc) },
	{ 0xb7f969ab, __VMLINUX_SYMBOL_STR(single_open) },
	{ 0x964fcf43, __VMLINUX_SYMBOL_STR(param_ops_int) },
	{ 0x2067c26d, __VMLINUX_SYMBOL_STR(seq_open) },
	{ 0xf290f33a, __VMLINUX_SYMBOL_STR(single_release) },
	{ 0xb5dcab5b, __VMLINUX_SYMBOL_STR(remove_wait_queue) },
	{ 0x9584ca05, __VMLINUX_SYMBOL_STR(seq_printf) },
	{ 0x24fc041f, __VMLINUX_SYMBOL_STR(usb_deregister_dev) },
	{ 0xf8227ae6, __VMLINUX_SYMBOL_STR(remove_proc_entry) },
	{ 0x85fc8a4c, __VMLINUX_SYMBOL_STR(param_ops_bool) },
	{ 0x1492f84e, __VMLINUX_SYMBOL_STR(usb_autopm_get_interface) },
	{ 0xb5841bf0, __VMLINUX_SYMBOL_STR(usb_enable_autosuspend) },
	{ 0x9e862205, __VMLINUX_SYMBOL_STR(usb_disable_autosuspend) },
	{ 0x560967ba, __VMLINUX_SYMBOL_STR(seq_read) },
	{ 0x472a4d1e, __VMLINUX_SYMBOL_STR(usb_unanchor_urb) },
	{ 0xf432dd3d, __VMLINUX_SYMBOL_STR(__init_waitqueue_head) },
	{ 0x4f8b5ddb, __VMLINUX_SYMBOL_STR(_copy_to_user) },
	{ 0xffd5a395, __VMLINUX_SYMBOL_STR(default_wake_function) },
	{ 0x8d290721, __VMLINUX_SYMBOL_STR(PDE_DATA) },
	{ 0xe9c7a2d7, __VMLINUX_SYMBOL_STR(proc_mkdir) },
	{ 0xd54f8114, __VMLINUX_SYMBOL_STR(dev_err) },
	{ 0x8f64aa4, __VMLINUX_SYMBOL_STR(_raw_spin_unlock_irqrestore) },
	{ 0xb16bdecb, __VMLINUX_SYMBOL_STR(current_task) },
	{ 0x904ae87c, __VMLINUX_SYMBOL_STR(usb_deregister) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xa35a2dee, __VMLINUX_SYMBOL_STR(usb_set_interface) },
	{ 0x15ddbdc, __VMLINUX_SYMBOL_STR(wait_for_completion_interruptible) },
	{ 0x4c40b530, __VMLINUX_SYMBOL_STR(usb_autopm_put_interface_async) },
	{ 0xa1c76e0a, __VMLINUX_SYMBOL_STR(_cond_resched) },
	{ 0x9166fada, __VMLINUX_SYMBOL_STR(strncpy) },
	{ 0x497bf509, __VMLINUX_SYMBOL_STR(seq_putc) },
	{ 0x92c75d36, __VMLINUX_SYMBOL_STR(usb_register_dev) },
	{ 0xc7344aee, __VMLINUX_SYMBOL_STR(usb_driver_claim_interface) },
	{ 0x16305289, __VMLINUX_SYMBOL_STR(warn_slowpath_null) },
	{ 0x80a5d587, __VMLINUX_SYMBOL_STR(proc_mkdir_data) },
	{ 0x3e3b32e, __VMLINUX_SYMBOL_STR(usb_free_coherent) },
	{ 0x4656ac52, __VMLINUX_SYMBOL_STR(usb_submit_urb) },
	{ 0x441937ce, __VMLINUX_SYMBOL_STR(usb_get_dev) },
	{ 0xff1076fc, __VMLINUX_SYMBOL_STR(usb_kill_anchored_urbs) },
	{ 0xdb7305a1, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0x7a345e1d, __VMLINUX_SYMBOL_STR(usb_put_dev) },
	{ 0x1000e51, __VMLINUX_SYMBOL_STR(schedule) },
	{ 0x7de24c79, __VMLINUX_SYMBOL_STR(usb_driver_release_interface) },
	{ 0x2ea2c95c, __VMLINUX_SYMBOL_STR(__x86_indirect_thunk_rax) },
	{ 0x55e6781d, __VMLINUX_SYMBOL_STR(proc_get_parent_data) },
	{ 0x7e3097e, __VMLINUX_SYMBOL_STR(usb_find_interface) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
	{ 0x87b393c2, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0x9327f5ce, __VMLINUX_SYMBOL_STR(_raw_spin_lock_irqsave) },
	{ 0x99195078, __VMLINUX_SYMBOL_STR(vsnprintf) },
	{ 0xcf21d241, __VMLINUX_SYMBOL_STR(__wake_up) },
	{ 0x34f22f94, __VMLINUX_SYMBOL_STR(prepare_to_wait_event) },
	{ 0x4f68e5c9, __VMLINUX_SYMBOL_STR(do_gettimeofday) },
	{ 0x37c49ac8, __VMLINUX_SYMBOL_STR(proc_create_data) },
	{ 0x5860aad4, __VMLINUX_SYMBOL_STR(add_wait_queue) },
	{ 0x26fa4497, __VMLINUX_SYMBOL_STR(seq_lseek) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x69acdf38, __VMLINUX_SYMBOL_STR(memcpy) },
	{ 0xf936b407, __VMLINUX_SYMBOL_STR(usb_register_driver) },
	{ 0xfa66f77c, __VMLINUX_SYMBOL_STR(finish_wait) },
	{ 0xed7f8a78, __VMLINUX_SYMBOL_STR(usb_ifnum_to_if) },
	{ 0xce97d129, __VMLINUX_SYMBOL_STR(usb_anchor_empty) },
	{ 0x4b06d2e7, __VMLINUX_SYMBOL_STR(complete) },
	{ 0x28318305, __VMLINUX_SYMBOL_STR(snprintf) },
	{ 0xe8c09f79, __VMLINUX_SYMBOL_STR(seq_release) },
	{ 0xd11cae85, __VMLINUX_SYMBOL_STR(usb_alloc_coherent) },
	{ 0x4f6b400b, __VMLINUX_SYMBOL_STR(_copy_from_user) },
	{ 0x2a38ed40, __VMLINUX_SYMBOL_STR(usb_free_urb) },
	{ 0xf5d96135, __VMLINUX_SYMBOL_STR(usb_anchor_urb) },
	{ 0xf83cb267, __VMLINUX_SYMBOL_STR(device_set_wakeup_enable) },
	{ 0x4203e11, __VMLINUX_SYMBOL_STR(usb_alloc_urb) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("usb:v*p*d*dcE0dsc01dp01ic*isc*ip*in*");
MODULE_ALIAS("usb:v0A5Cp*d*dc*dsc*dp*icE0isc01ip01in*");
MODULE_ALIAS("usb:v0A5Cp*d*dcFFdsc01dp01ic*isc*ip*in*");
MODULE_ALIAS("usb:v04B4p*d*dc*dsc*dp*icFFisc01ip01in*");
MODULE_ALIAS("usb:v04B4p*d*dcFFdsc01dp01ic*isc*ip*in*");

MODULE_INFO(srcversion, "3DD88708A3B25B31CA7E22D");
