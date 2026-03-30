#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/i2c.h>

static struct kobject *my_kobj;
static struct i2c_client *client;

/* ================= SYSFS ================= */

static ssize_t data_show(struct kobject *kobj,
                         struct kobj_attribute *attr,
                         char *buf)
{
    int ret;

    if (!client)
        return -ENODEV;

    ret = i2c_smbus_read_byte(client);
    if (ret < 0)
        return ret;

    return sprintf(buf, "0x%02x\n", ret);
}

static ssize_t data_store(struct kobject *kobj,
                          struct kobj_attribute *attr,
                          const char *buf, size_t count)
{
    unsigned long value;

    if (kstrtoul(buf, 0, &value))
        return -EINVAL;

    if (client)
        i2c_smbus_write_byte(client, (u8)value);

    return count;
}

static struct kobj_attribute data_attribute =
    __ATTR(data, 0664, data_show, data_store);

/* ================= I2C DRIVER ================= */

static int my_probe(struct i2c_client *cl)
{
    int ret;

    pr_info("my_i2c_module: device found at 0x%02x\n", cl->addr);

    client = cl;

    /* sysfs create */
    my_kobj = kobject_create_and_add("my_i2c_module", kernel_kobj);
    if (!my_kobj)
        return -ENOMEM;

    ret = sysfs_create_file(my_kobj, &data_attribute.attr);
    if (ret) {
        kobject_put(my_kobj);
        return ret;
    }

    return 0;
}

static void my_remove(struct i2c_client *cl)
{
    pr_info("my_i2c_module removed\n");

    if (my_kobj)
        kobject_put(my_kobj);

    client = NULL;
}

/* ================= ID TABLE ================= */

static const struct i2c_device_id my_id[] = {
    { "my_i2c_module", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, my_id);

/* ================= DRIVER ================= */

static struct i2c_driver my_driver = {
    .driver = {
        .name = "my_i2c_module",
    },
    .probe = my_probe,
    .remove = my_remove,
    .id_table = my_id,
};

module_i2c_driver(my_driver);

/* ================= INFO ================= */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("NK");
MODULE_DESCRIPTION("Sysfs + I2C driver v3");
