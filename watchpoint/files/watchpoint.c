#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/uaccess.h>
#include <linux/hw_breakpoint.h>
#include <linux/sched.h>
#include <linux/stacktrace.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("RB");
MODULE_DESCRIPTION("A kernel module that sets a watchpoint on a memory address.");
MODULE_VERSION("1.0");

// this is a varibale , which stores the memory address
// which the user would then be able to modify through sysfs
static unsigned long gWatchMemAddress = 0;

// this macro allows the variable be set memory adress when loading kenrel module
// or chagne it via sysfs
module_param(gWatchMemAddress, ulong, 0644); //644 means you can read and write the file or directory and other users can only read it.
MODULE_PARM_DESC(gWatchMemAddress, "The memory address to monitor.");



typedef void (*access_callback_t)(void);

static void read_callback(void)
{
    pr_info("Read access detected\n");
}

static void write_callback(void)
{
    pr_info("Write access detected\n");
}

// this function is essentially the main logic of this module
// it is a callback function with a signature of harware breakpiint event type
// These kind of functions are used as a so called notification chains
// in our case we use it for notfying twhen our memory adress is accesed
static void watch_handler(struct perf_event *bp_event_, struct perf_sample_data * data_, struct pt_regs* regs_) 
{    
    unsigned long address = bp_event_->attr.bp_addr;
    // essentially this will happen every time
    if(address == gWatchMemAddress)
    {
        unsigned int type = bp_event_->attr.bp_type;

        pr_info("Watchpoint triggered at address: %lx\n", gWatchMemAddress);

        access_callback_t a_cb;
        if(type & HW_BREAKPOINT_W)
        {
            a_cb = read_callback;
        }   
        else if( type & HW_BREAKPOINT_R)
        {
            a_cb = write_callback;
        }

        a_cb();
        
        // this function is responsible for printing call stack(backtrace)
        dump_stack();
    }
}

// this here part is responsible for creating the event itself;
// now, there are couple of ways in which it is possible to setup 
// hardware breakpoint (some of them offers to use debug registers dr0)
// however there is already api for this purpose - perf_event
// this is a performance subsystem which offers a lot of features
static struct perf_event * __percpu *hw_breakpoint_event;
static int setup_watchpoint(void)
{
    // if the event was previously registered (in case user changes the memory adress to look after)
    // and the event is not erroneous 
    if(hw_breakpoint_event && !IS_ERR(hw_breakpoint_event))
    {
        unregister_wide_hw_breakpoint(hw_breakpoint_event);
        pr_info("Previous hardware breakpoint unregistered\n");
    }
    else
    {
        pr_info("No valid hardware_bp to unregister\n");
    }

    struct perf_event_attr attr;
    
    hw_breakpoint_init(&attr);
    attr.bp_addr = gWatchMemAddress;
    attr.bp_len = HW_BREAKPOINT_LEN_4;  // Assuming 4 bytes for simplicity
    attr.bp_type = HW_BREAKPOINT_W | HW_BREAKPOINT_R;  // Watch for read and write

    hw_breakpoint_event = register_wide_hw_breakpoint(&attr, watch_handler, NULL);

    if (IS_ERR(hw_breakpoint_event))
    {
        pr_err("Failed to register hardware breakpoint\n");
        return PTR_ERR(hw_breakpoint_event);
    }

    pr_info("Hardware breakpoint registered at address: %lx\n", gWatchMemAddress);
    return 0;
}

static void remove_watchpoint(void)
{
    if (hw_breakpoint_event && !IS_ERR(hw_breakpoint_event))
    {
        unregister_wide_hw_breakpoint(hw_breakpoint_event);
        pr_info("Hardware breakpoint unregistered\n");
    }
    else
    {
        pr_info("No valid hardware_bp to unregister\n");
    }
}

// These two functions next _show and _stroe are essentially the user defined
// functions, signature of which is predifned and is called within the kobj_attr_show and _store function by the system; 
// by defining our own function we get to define what to do with our 
// varibale and thus perfroming operations requested in task 

// this function would be called when the user would access sysfs to read the file asscoiated with this module
// so for instance if i would to cat /sys/kernel/watch/gWatchMemAddress
static ssize_t watch_address_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "The memory adress watched : %lx\n", gWatchMemAddress);
}

// this function is called when the user wants to write into the sysfs attribute
static ssize_t watch_address_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) 
{
    pr_info ("Request to change hw breakpoint memory adress\n");
    unsigned long adress ;
    // get the new value from the input and store it into our varibale
    if(sscanf(buf, "%lx", &adress) != 1)
    {
        pr_err("Invalid adress format\n");
        return -EINVAL;
    }

    gWatchMemAddress = adress;
    pr_info("The new memory adress : %lx", gWatchMemAddress);
    // Set the hardware breakpoint
    int ret = setup_watchpoint();
    if(ret)
    {
        pr_err("Failed to set up watchpoint\n");
    }
    return count;
}

// this is the struct, which the kobjects system would use to call our functions;
static struct kobj_attribute watch_address_attribute = __ATTR(gWatchMemAddress, 0660, watch_address_show, watch_address_store);

// 
static struct kobject *watch_kobj;
static int __init watch_module_init(void) 
{
    int error = 0;

    pr_info("Module loading with watch_adress = %lx\n", gWatchMemAddress);

    watch_kobj = kobject_create_and_add("watch_module", kernel_kobj);
    if (!watch_kobj)
        return -ENOMEM;

    error = sysfs_create_file(watch_kobj, &watch_address_attribute.attr);
    if (error) {
        kobject_put(watch_kobj);
        return error;
    }

    error = setup_watchpoint();
    if(error)
    {
        sysfs_remove_file(watch_kobj, &watch_address_attribute.attr);
        kobject_put(watch_kobj);
        return error;
    }

    pr_info("Watch module initialized.\n");
    return 0;
}

static void __exit watch_module_exit(void) 
{   
    remove_watchpoint();
    sysfs_remove_file(watch_kobj, &watch_address_attribute.attr);
    kobject_put(watch_kobj);
    pr_info("Watch module exited.\n");
}

// dirver entry point 
module_init(watch_module_init);
// called when dirver is unloaded 
module_exit(watch_module_exit);
