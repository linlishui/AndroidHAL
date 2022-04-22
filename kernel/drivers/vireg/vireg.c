#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>

#include "vireg.h"

static int vireg_major = 0;
static int vireg_minor = 0;

static struct class* vireg_class = NULL;
static struct virtual_reg_dev* vireg_dev = NULL;

static int vireg_open(struct inode* inode, struct file* filp);
static int vireg_release(struct inode* inode, struct file* filp);
static ssize_t vireg_read(struct file* filp, char __user *buf, size_t count, loff_t* f_pos);
static ssize_t vireg_write(struct file* filp, const char __user *buf, size_t count, loff_t* f_pos);

static struct file_operations vireg_fops = {
    .owner = THIS_MODULE,
    .open = vireg_open,
    .release = vireg_release,
    .read = vireg_read,
    .write = vireg_write,
};

#define SEQ_printf(m, x...)     \
    do {                \
        if (m)          \
            seq_printf(m, x);   \
        else            \
            pr_err(x);      \
    } while (0)

static int vireg_proc_show(struct seq_file *m, void *v)
{
    SEQ_printf(m, "%d\n", vireg_dev->val);
    return 0;
}

static int vireg_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, vireg_proc_show, inode->i_private);
}

static ssize_t __vireg_set_val(struct virtual_reg_dev* dev, const char* buf, size_t count){
    int val = 0;

    val = simple_strtol(buf, NULL, 10);

    if(down_interruptible(&(dev->sem))){
        return -ERESTARTSYS;
    }

    dev->val = val;
    up(&(dev->sem));

    return count;
}

/*
static ssize_t vireg_proc_read(char* page, char** start, off_t off, int count, int* eof, void* data){
    if(off >0 ){
        *eof = 1;
        return 0;
    }

    return __vireg_get_val(vireg_dev, page);
}*/

static ssize_t vireg_proc_write(struct file *filp, const char *ubuf, size_t cnt, loff_t *data){
    int err = 0;
    char* page = NULL;

    if(cnt > PAGE_SIZE){
        printk(KERN_ALERT"The buff is too large: %lu.\n", cnt);
        return -EFAULT;
    }

    page = (char*) __get_free_page(GFP_KERNEL);
    if(!page){
        printk(KERN_ALERT"Failed to alloc page.\n");
        return -ENOMEM;
    }

    if(copy_from_user(page, ubuf, cnt)){
        printk(KERN_ALERT"Failed to copy buff from user.\n");
        err = -EFAULT;
        goto out;
    }

    err = __vireg_set_val(vireg_dev, page, cnt);

out:
    free_page((unsigned long)page);
    return err;
}

static const struct file_operations vireg_proc_fops = {
    .open = vireg_proc_open,
    .write = vireg_proc_write,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};


static ssize_t vireg_val_show(struct device* dev, struct device_attribute* attr, char* buf);
static ssize_t vireg_val_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t count);

static DEVICE_ATTR(val, S_IRUGO | S_IWUSR, vireg_val_show, vireg_val_store);

static int vireg_open(struct inode * inode, struct file * filp){
    struct virtual_reg_dev * dev;

    dev = container_of(inode->i_cdev, struct virtual_reg_dev, dev);
    filp->private_data = dev;

    return 0;
}

static int vireg_release(struct inode* inode, struct file* filp){
    return 0;
}

static ssize_t vireg_read(struct file* filp, char __user *buf, size_t count, loff_t* f_pos){
    ssize_t err = 0;
    struct virtual_reg_dev* dev = filp->private_data;

    if(down_interruptible(&(dev->sem))){
        return -ERESTARTSYS;
    }

    if(count < sizeof(dev->val)){
        goto out;
    }

    if(copy_to_user(buf, &(dev->val), sizeof(dev->val))){
        err = -EFAULT;
        goto out;
    }

    err = sizeof(dev->val);

out:
    up(&(dev->sem));
    return err;
}

static ssize_t vireg_write(struct file * filp, const char __user * buf, size_t count, loff_t * f_pos){
    struct virtual_reg_dev* dev = filp->private_data;
    ssize_t err = 0;

    if(down_interruptible(&(dev->sem))){
        return -ERESTARTSYS;
    }

    if(count != sizeof(dev->val)){
        goto out;
    }

    if(copy_from_user(&(dev->val), buf, count)){
        err = -EFAULT;
        goto out;
    }

    err = sizeof(dev->val);

out:
    up(&(dev->sem));
    return err;
}

static ssize_t __vireg_get_val(struct virtual_reg_dev* dev, char* buf){
    int val = 0;

    if(down_interruptible(&dev->sem)){
        return -ERESTARTSYS;
    }

    val = dev->val;
    up(&(dev->sem));

    return snprintf(buf, PAGE_SIZE, "%d\n", val);
}


static ssize_t vireg_val_show(struct device* dev, struct device_attribute* attr, char* buf){
    struct virtual_reg_dev* hdev = (struct virtual_reg_dev*)dev_get_drvdata(dev);

    return __vireg_get_val(hdev, buf);
}

static ssize_t vireg_val_store(struct device*dev, struct device_attribute* attr, const char* buf, size_t count){
    struct virtual_reg_dev* hdev = (struct virtual_reg_dev*)dev_get_drvdata(dev);

    return __vireg_set_val(hdev, buf, count);
}


static void vireg_create_proc(void){
    proc_create(VIREG_DEVICE_PROC_NAME, 0644, 0,  &vireg_proc_fops);
}

static void vireg_remove_proc(void){
    remove_proc_entry(VIREG_DEVICE_PROC_NAME, NULL);
}

static int __vireg_setup_dev(struct virtual_reg_dev* dev){
    int err;
    dev_t devno = MKDEV(vireg_major, vireg_minor);

    memset(dev, 0, sizeof(struct virtual_reg_dev));

    cdev_init(&(dev->dev), &vireg_fops);
    dev->dev.owner = THIS_MODULE;
    dev->dev.ops = &vireg_fops;

    err = cdev_add(&(dev->dev), devno, 1);
    if(err){
        return err;
    }

    //init_MUTEX(&(dev->sem));
    sema_init(&(dev->sem), 1);
    dev->val = 0;

    return 0;
}

static int __init vireg_init(void){
    int err = -1;
    dev_t dev = 0;
    struct device* temp = NULL;

    printk(KERN_ALERT"Initializing vireg device.\n");

    err = alloc_chrdev_region(&dev, 0, 1, VIREG_DEVICE_NODE_NAME);
    if(err < 0){
        printk(KERN_ALERT"Failed to alloc char dev region.\n");
        goto fail;
    }

    vireg_major = MAJOR(dev);
    vireg_minor = MINOR(dev);

    vireg_dev = kmalloc(sizeof(struct virtual_reg_dev), GFP_KERNEL);
    if(!vireg_dev){
        err = -ENOMEM;
        printk(KERN_ALERT"Failed to alloc vireg device.\n");
        goto unregister;
    }

    err = __vireg_setup_dev(vireg_dev);
    if(err){
        printk(KERN_ALERT"Failed to setup vireg device: %d.\n", err);
        goto cleanup;
    }

    vireg_class = class_create(THIS_MODULE, VIREG_DEVICE_CLASS_NAME);
    if(IS_ERR(vireg_class)){
        err = PTR_ERR(vireg_class);
        printk(KERN_ALERT"Failed to create vireg device class.\n");
        goto destroy_cdev;
    }

    temp = device_create(vireg_class, NULL, dev, NULL, "%s", VIREG_DEVICE_FILE_NAME);
    if(IS_ERR(temp)){
        err = PTR_ERR(temp);
        printk(KERN_ALERT"Failed to create vireg device.\n");
        goto destroy_class;
    }

    err = device_create_file(temp, &dev_attr_val);
    if(err < 0){
        printk(KERN_ALERT"Failed to create attribute val of vireg device.\n");
        goto destroy_device;
    }

    dev_set_drvdata(temp, vireg_dev);

    vireg_create_proc();

    printk(KERN_ALERT"Succedded to initialize vireg device.\n");

    return 0;

destroy_device:
    device_destroy(vireg_class, dev);
destroy_class:
    class_destroy(vireg_class);
destroy_cdev:
    cdev_del(&(vireg_dev->dev));
cleanup:
    kfree(vireg_dev);
unregister:
    unregister_chrdev_region(MKDEV(vireg_major, vireg_minor), 1);
fail:
    return err;
}

static void __exit vireg_exit(void){
    dev_t devno = MKDEV(vireg_major, vireg_minor);

    printk(KERN_ALERT"Destory vireg device.\n");

    vireg_remove_proc();

    if(vireg_class){
        device_destroy(vireg_class, MKDEV(vireg_major, vireg_minor));
        class_destroy(vireg_class);
    }

    if(vireg_dev){
        cdev_del(&(vireg_dev->dev));
        kfree(vireg_dev);
    }

    unregister_chrdev_region(devno, 1);
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Virtual Register Driver");

module_init(vireg_init);
module_exit(vireg_exit);