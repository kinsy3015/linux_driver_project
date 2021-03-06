#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/ptrace.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/mm_types.h>

#define DEV_NAME		"mmap"

#define DATA_SIZE		(1 * (1 << PAGE_SHIFT))


static const unsigned int MINOR_BASE = 0;
static const unsigned int MINOR_NUM = 1;
static unsigned int mdev_major;
static struct cdev *mdev_cdev = NULL;
static struct class *mdev_class = NULL;

static int *data = NULL;
static atomic_t counter = ATOMIC_INIT(0);


static int mmap_open(struct inode *inode, struct file *filp);
static int mmap_release(struct inode *inode, struct file *filp);
static ssize_t mmap_read(struct file *filp, char __user *buf, size_t count, loff_t *offset);
static ssize_t mmap_write(struct file *filp, const char __user *buf, size_t count, loff_t *offset);
static void mmap_vma_open(struct vm_area_struct *vma);
static void mmap_vma_close(struct vm_area_struct *vma);
static int mmap_vm_fault(struct vm_fault *vmf, struct vm_area_struct *vma);
static int mmap_remap(struct file *filp, struct vm_area_struct *vma);
static loff_t mmap_lseek(struct file *filp, loff_t offset, int org);


static struct vm_operations_struct vma_ops = {
	.open = mmap_vma_open,
	.close = mmap_vma_close,
	.fault = mmap_vm_fault,
};

struct file_operations mmap_fops = {
	.open = mmap_open,
	.release = mmap_release,
	.read = mmap_read,
	.write = mmap_write,
	.mmap = mmap_remap,
	.llseek = mmap_lseek
};

static void mmap_vma_open(struct vm_area_struct *vma){
	atomic_inc(&counter);
	printk("%s: %d\n", __func__, atomic_read(&counter));
 
	printk("vm_pgoff: %08lx\n", vma->vm_pgoff);
	printk("vm_start: %08lx\n", vma->vm_start);
	printk("vm_end  : %08lx\n", vma->vm_end);
}
 
static void mmap_vma_close(struct vm_area_struct *vma){
	atomic_dec(&counter);
	printk("%s: %d\n", __func__, atomic_read(&counter));
}
 
static int mmap_vm_fault(struct vm_fault *vmf, struct vm_area_struct *vma){
	struct page *page = NULL;
	unsigned long offset = 0;
	void *page_ptr = NULL;
 
	printk("%s\n", __func__);
	if (vmf == NULL)
		return VM_FAULT_SIGBUS;
 
	offset = vmf->virtual_address - vma->vm_start;
	if (offset >= DATA_SIZE)
		return VM_FAULT_SIGBUS;
 
	page_ptr = data + offset;
	page = vmalloc_to_page(page_ptr);
	get_page(page);
	vmf->page = page;
	return 0;
}


static int mmap_open(struct inode *inode, struct file *filp){
	return 0;
}
static int mmap_release(struct inode *inode, struct file *filp){
	return 0;
}
static ssize_t mmap_read(struct file *filp, char __user *buf, size_t count, loff_t *offset){
	if(*offset > DATA_SIZE)
		return -EIO;
	copy_to_user(buf, (void *)(data + *offset), count);
	*offset +=1;
	filp->f_pos = *offset;
	return count;
}
static ssize_t mmap_write(struct file *filp, const char __user *buf, size_t count, loff_t *offset){
	copy_from_user((void *)(data + *offset), buf, count);
	*offset +=1;
	filp->f_pos = *offset;
	return count;	
}
static int mmap_remap(struct file *filp, struct vm_area_struct *vma){
	printk("%s\n", __func__);
	
	vma->vm_flags |= VM_IO;
	vma->vm_ops = &vma_ops;
	mmap_vma_open(vma);
	return 0;
}
static loff_t mmap_lseek(struct file *filp, loff_t offset, int org){
	loff_t ret;
	printk("%s\n", __func__);
	switch (org)
	{
		case SEEK_SET:
			filp->f_pos = offset;
			ret = filp->f_pos;
			force_successful_syscall_return();
			break;
		case SEEK_CUR:
			filp->f_pos += offset;
			ret = filp->f_pos;
			force_successful_syscall_return();
			break;
		default:
			ret = -EINVAL;
	}
}



static int __init _mmap_init(void){
	int alloc_ret = 0, cdev_err = 0;
	dev_t dev;
 
	mdev_cdev = cdev_alloc();
	alloc_ret = alloc_chrdev_region(&dev, MINOR_BASE, MINOR_NUM, DEV_NAME);
	if (alloc_ret != 0) {
		printk(KERN_ERR "alloc_chrdev_region = %d\n", alloc_ret);
		return -1;
	}
 
	mdev_major = MAJOR(dev);
	dev = MKDEV(mdev_major, MINOR_BASE);
 
	cdev_init(mdev_cdev, &mmap_fops);
	mdev_cdev->owner = THIS_MODULE;
 
	cdev_err = cdev_add(mdev_cdev, dev, MINOR_NUM);
	if (cdev_err != 0) {
		printk(KERN_ERR "cdev_add = %d\n", cdev_err);
		goto OUT2;
	}
 
	mdev_class = class_create(THIS_MODULE, "mmap_device");
	if (IS_ERR(mdev_class)) {
		printk(KERN_ERR "class_create\n");
		goto OUT;
	}
 
	device_create(mdev_class, NULL, MKDEV(mdev_major, MINOR_BASE), NULL, DEV_NAME);
	data = vmalloc(DATA_SIZE);
	if (data == NULL) {
		printk(KERN_ERR "vmalloc failed\n");
		goto OUT;
	}
	memset(data, 0, DATA_SIZE);
	return 0;
 
OUT:
	cdev_del(mdev_cdev);
OUT2:
	unregister_chrdev_region(dev, MINOR_NUM);
	return -1;

}

static void __exit _mmap_exit(void){
	dev_t dev = MKDEV(mdev_major, MINOR_BASE);
	device_destroy(mdev_class, dev);
	class_destroy(mdev_class);
	cdev_del(mdev_cdev);
	unregister_chrdev_region(dev, MINOR_NUM);
	vfree(data);
}


module_init(_mmap_init);
module_exit(_mmap_exit);
MODULE_LICENSE("GPL"); 
