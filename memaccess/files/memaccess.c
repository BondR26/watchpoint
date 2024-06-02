#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/io.h>

#define DEVICE_NAME "memaccess"
#define MEMORY_SIZE 4

static void __iomem *mem_ptr;
static unsigned long mem_address = 0;
static int test_addr = 111;

static struct kobject *memaccess_kobj;

// Function to read the memory address
static ssize_t read_mem_address(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    pr_info("read_mem_address function\n");
    
    pr_info("Random test addr = %lx", (unsigned long)&test_addr);
   
    pr_info("Starting to read the value from memory adress is %lx", mem_address);
    test_addr++;
    
    return sprintf(buf, "The value is : %d; Adress is 0x%X\n", test_addr, mem_address);
}

// Function to write the memory address
static ssize_t write_mem_address(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) 
{
    pr_info("write_mem_address function called\n");
   
    unsigned long new_mem_addr;
    int ret;

    ret = sscanf(buf, "%lx", &new_mem_addr);
    // get the new value from the input and store it into our varibale
    if( ret != 1)
    {
        pr_err("Invalid adress format\n");
        return -EINVAL;
    }

    mem_address = new_mem_addr;
    ret = pr_info("The new memory adress : %lx", mem_address);

    if (ret < 0)
    {
        pr_info("Couldnt read the mem_adress\n");
        return ret;
    }
    return count;
}

static struct kobj_attribute mem_address_attr = __ATTR(mem_address, 0660, read_mem_address, write_mem_address);

static int __init memaccess_init(void) {
    int retval;

    printk(KERN_INFO "MemAccess: Initializing module\n");

    memaccess_kobj = kobject_create_and_add("memaccess", kernel_kobj);
    if (!memaccess_kobj)
        return -ENOMEM;

    retval = sysfs_create_file(memaccess_kobj, &mem_address_attr.attr);
    if (retval) {
        kobject_put(memaccess_kobj);
    }

    return retval;
}

static void __exit memaccess_exit(void) 
{
    if (mem_ptr)
    {
        iounmap(mem_ptr);
    }
    kobject_put(memaccess_kobj);
    printk(KERN_INFO "MemAccess: Exiting module\n");
}

module_init(memaccess_init);
module_exit(memaccess_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A kernel module to read a specific memory address with sysfs interface");
