#include <linux/delay.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/i2c.h>

#define I2C_BUS 1
#define I2C_ADDR 0x20   // addr

static struct kobject *my_kobj;
static struct i2c_client *client;
static struct i2c_board_info info = {
    I2C_BOARD_INFO("my_i2c_module", I2C_ADDR),
};

static ssize_t data_show(struct kobject *kobj,
                         struct kobj_attribute *attr,
                         char *buf)
{
    int ret;

    if (!client) {
        pr_err("I2C client not initialized!\n");
        return -ENODEV;
    }

    ret = i2c_smbus_read_byte(client);
    if (ret < 0) {
        pr_err("I2C read failed\n");
        return ret;
    }

    return sprintf(buf, "0x%02x\n", ret);
}


/* SYSFS WRITE */
static ssize_t data_store(struct kobject *kobj,
                          struct kobj_attribute *attr,
                          const char *buf, size_t count)
{
    unsigned long value;
    int ret;

    ret = kstrtoul(buf, 0, &value);
    if (ret)
        return ret;

    printk("my_i2c_module: writing %lu to I2C\n", value);

    if (client) {
        i2c_smbus_write_byte(client, (u8)value);
    } else {
        printk("I2C client not initialized!\n");
    }

    return count;
}

static struct kobj_attribute data_attribute =
    __ATTR(data, 0664, data_show, data_store);

/* INIT */
static int __init my_init(void)
{
    struct i2c_adapter *adapter;

    printk("my_i2c_module v2 loaded\n");

    /* sysfs */
    my_kobj = kobject_create_and_add("my_i2c_module", kernel_kobj);
    if (!my_kobj)
        return -ENOMEM;

    int ret;

    ret = sysfs_create_file(my_kobj, &data_attribute.attr);
    if (ret) {
        printk("Failed to create sysfs file\n");
        return ret;
    }

    /* I2C setup */
    int retries = 5;

    do {
        adapter = i2c_get_adapter(I2C_BUS);
        if (adapter)
            break;
        msleep(200);
    } while (--retries);

    //adapter = i2c_get_adapter(I2C_BUS);
    if (!adapter) {
        printk("Failed to get I2C adapter after retries\n");
        return -ENODEV;
    }
    client = i2c_new_client_device(adapter, &info);
    //client = i2c_new_dummy_device(adapter, I2C_ADDR);
    i2c_put_adapter(adapter);

    if (IS_ERR(client)) {
        printk("Failed to create I2C client\n");
        return PTR_ERR(client);
    }

    printk("I2C client created at address 0x%x\n", I2C_ADDR);

    return 0;
}

/* EXIT */
static void __exit my_exit(void)
{
    printk("my_i2c_module unloaded\n");

    if (client)
        i2c_unregister_device(client);

    if (my_kobj)
        kobject_put(my_kobj);
}

late_initcall(my_init);
//module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("NK");
MODULE_DESCRIPTION("Sysfs + I2C test1");
