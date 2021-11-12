#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/freezer.h>
#include <linux/pm.h>
//#include <linux/suspend.h>

#include <asm/io.h>
#include <asm/irq.h>

#include <linux/swap.h>
#include <linux/notifier.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <asm/errno.h>


//#include <asm/arch/regs-gpio.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <mach/regs-gpio.h>

#include <linux/workqueue.h>

#include <linux/cdev.h> //cdev_init
#include <linux/wait.h> //wait_event_interruptible
#include <asm/uaccess.h> //user access
#include <linux/fs.h> //file_operatios 

#include <linux/platform_device.h> //platform_driver_register()


#define device_name        "touch-sus0"

static ssize_t tssus_touch_open(struct inode* inode, struct file* file);
static ssize_t tssus_touch_release(struct inode* inode, struct file* file);
static ssize_t tssus_touch_read(struct inode* inode, struct file* file);

static void tssus_touch_probe(struct platform_device* dev);
static void tssus_touch_remove(struct platform_device* dev);
static ssize_t tssus_touch_suspend(struct platform_device* dev);
static ssize_t tssus_touch_resume(struct platform_device* dev);


static void tssus_device_release(struct device* dev);


static void tssus_class_release(struct class_device* dev){
	printk("%s start....\n", __func__);
	printk(KERN_INFO "touch_class_release(struct class_device*)\n"); 
}




static ssize_t touch_major = 0;
static const ssize_t touch_minor_base = 0;
static const ssize_t touch_minor = 1;
static ssize_t flags;
static ssize_t result;
static ssize_t tsrror;
static dev_t tssus_touch_dev = 0;
struct cdev tssus_touch_cdev;

 
static DECLARE_WAIT_QUEUE_HEAD(wq);

static struct file_operations mds2450_touch_fops = {
        .owner          = THIS_MODULE,
        .open           = tssus_touch_open,
        .release        = tssus_touch_release,
		.read			= tssus_touch_read,
};

static struct dev_pm_ops tssus_sus_ops = {
	.suspend	= tssus_touch_suspend,
	.resume		= tssus_touch_resume,
};

static struct platform_driver tssus_touch_driver = {
        
    .probe          = tssus_touch_probe,
    .remove         = tssus_touch_remove,
	.suspend	= tssus_touch_suspend,
	.resume 	= tssus_touch_resume,
	.driver         = {
                .name   = device_name,
                .owner  = THIS_MODULE,
				.pm = &tssus_sus_ops,
        },

};

static struct class tssus_touch_class = {
	.name	="touch_sus0",
	.class_release = tssus_class_release,
	
};

static struct device tssus_touch_device = { 
        .init_name = "touch_sus0",
        .class = &tssus_touch_class,
        .release = tssus_device_release,
};
static ssize_t tssus_touch_open(struct inode* inode, struct file* file){
	printk("%s start....\n", __func__);
	//pm_power_off();
	//pm_suspend(PM_SUSPEND_STANDBY);
	
	printk(KERN_INFO "touch_open(inode,file)\n");
	return 0;
}

static ssize_t tssus_touch_release(struct inode* inode, struct file* file){	
	printk("%s start....\n", __func__);
	printk(KERN_INFO "touch_release(inode,file)\n");
	return 0;
}

static ssize_t tssus_touch_read(struct inode* inode, struct file* file){
	printk("%s start....\n", __func__);
	pm_power_off();
	//freeze_processes();
	//pm_suspend(PM_SUSPEND_ON);
	printk(KERN_INFO "touch_read(inode,file)\n");
	return 0;
}

static void tssus_touch_probe(struct platform_device* dev){
	printk("%s start....\n", __func__);
	printk(KERN_INFO "touch_probe(struct platform_device*)\n");
}

static void tssus_touch_remove(struct platform_device* dev){	
	printk("%s start....\n", __func__);
	printk(KERN_INFO "touch_remove(struct platform_device*)\n");
}

static ssize_t tssus_touch_suspend(struct platform_device* dev){
	printk("%s start....\n", __func__);
    printk(KERN_INFO "touch_suspend(struct paltform_device*)\n");

    local_irq_save(flags);
    local_irq_disable();
	return 0;
}

static ssize_t tssus_touch_resume(struct platform_device* dev){
	printk("%s start....\n", __func__);
    printk(KERN_INFO "touch_resume(struct paltform_device*)\n");

	local_irq_restore(flags);

	return 0;
}
static void tssus_device_release(struct device* dev){	
	printk("%s start....\n", __func__);
	printk(KERN_INFO "touch_cdevice_release(struct device*)\n");
}




static ssize_t tssus_register_cdev(void){
	printk("%s start....\n", __func__);

	if(touch_major) {
		tssus_touch_dev = MKDEV(touch_major, touch_minor);
		tsrror = register_chrdev_region(tssus_touch_dev, 1, device_name);
	} else {
		tsrror = alloc_chrdev_region(&tssus_touch_dev, touch_minor, 1, device_name);
		touch_major = MAJOR(tssus_touch_dev);
	}

	if (tsrror < 0) {
		printk(KERN_WARNING "touch: can't get major %d\n", touch_major);
		return tsrror;
	}

	printk("major number=%d\n", touch_major);

	cdev_init(&tssus_touch_cdev, &mds2450_touch_fops);
	tssus_touch_cdev.owner = THIS_MODULE;
	tsrror = cdev_add(&tssus_touch_cdev, tssus_touch_dev, 1);
	
	if(tsrror) 
		printk(KERN_NOTICE "touch Register Error %d\n", tsrror);

	return tsrror;
}

static ssize_t __init tssus_suspend_init(void){
	printk("%s start....\n", __func__);
	tssus_register_cdev();
	tssus_touch_device.devt = tssus_touch_dev;

    result = class_register(&tssus_touch_class);
    if(result)
            return result;
    result = device_register(&tssus_touch_device); 
    if(result)
            return result;
    result = platform_driver_register(&tssus_touch_driver);
	if(result)
			return result;

	//suspend_set_ops(&tssus_sus_ops);

	printk(KERN_INFO "%s successfully init...\n", device_name);
	return 0;
}

static void __exit tssus_suspend_exit(void){
	printk("%s start....\n", __func__);
	platform_driver_unregister(&tssus_touch_driver);
	device_unregister(&tssus_touch_device);
    class_unregister(&tssus_touch_class);
}

module_init(tssus_suspend_init);
module_exit(tssus_suspend_exit);
MODULE_LICENSE("GPL");
