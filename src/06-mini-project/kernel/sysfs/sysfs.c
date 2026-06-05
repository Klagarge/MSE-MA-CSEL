// workspace/src/06-mini-project/kernel/sysfs/sysfs.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/fs.h>
#include <linux/err.h>

#include "sysfs.h"

static struct class *sysfs_class = NULL;
static struct device *sysfs_device = NULL;

/* Global static structure to hold the registered callbacks */
static struct sysfs_callbacks device_cbs = {0};

/* Callback triggered on sysfs temperature file read */
static ssize_t temperature_show(struct device *dev, struct device_attribute *attr, char *buf) {
    uint32_t temp = 0;

    if (device_cbs.get_temperature) {
        temp = device_cbs.get_temperature();
    }

    /* Format the temperature and write it to the buffer */
    return snprintf(buf, PAGE_SIZE, "%u.%03u\n", temp / 1000, temp % 1000);
}

/* Callback triggered on sysfs mode file read */
static ssize_t mode_show(struct device *dev, struct device_attribute *attr, char *buf) {
    int mode = 0;

    if (device_cbs.get_mode) {
        mode = device_cbs.get_mode();
    }

    return snprintf(buf, PAGE_SIZE, "%d\n", mode);
}

/* Callback triggered on sysfs mode file write */
static ssize_t mode_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
    int mode;

    /* Safely convert string from user space to integer */
    if (kstrtoint(buf, 10, &mode) == 0) {
        if (device_cbs.set_mode) {
            device_cbs.set_mode(mode);
        }
    }

    return count;
}

/* Callback triggered on sysfs period_status file read */
static ssize_t period_status_show(struct device *dev, struct device_attribute *attr, char *buf) {
    uint32_t period = 0;

    if (device_cbs.get_period) {
        period = device_cbs.get_period();
    }

    return snprintf(buf, PAGE_SIZE, "%u\n", period);
}

/* Callback triggered on sysfs period_set file write */
static ssize_t period_set_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
    uint32_t period;

    if (kstrtouint(buf, 10, &period) == 0) {
        if (device_cbs.set_period) {
            device_cbs.set_period(period);
        }
    }

    return count;
}

/* Create the sysfs attribute structures with proper permissions */
static DEVICE_ATTR_RO(temperature);
static DEVICE_ATTR_RW(mode);
static DEVICE_ATTR_RO(period_status);
static DEVICE_ATTR_WO(period_set);

/* Group all attributes together to register them efficiently */
static struct attribute *regulator_attrs[] = {
    &dev_attr_temperature.attr,
    &dev_attr_mode.attr,
    &dev_attr_period_status.attr,
    &dev_attr_period_set.attr,
    NULL,
};

static const struct attribute_group regulator_group = {
    .attrs = regulator_attrs,
};

int temp_regulator_sysfs_init(struct sysfs_callbacks *cbs) {
    int ret;

    /* Save the callbacks provided by the main module */
    if (cbs) {
        device_cbs = *cbs;
    }

    /* Create sysfs class */
    sysfs_class = class_create(THIS_MODULE, "temp_regulator");
    if (IS_ERR(sysfs_class)) {
        return PTR_ERR(sysfs_class);
    }

    /* Create sysfs device */
    sysfs_device = device_create(sysfs_class, NULL, MKDEV(0, 0), NULL, "regulator");
    if (IS_ERR(sysfs_device)) {
        class_destroy(sysfs_class);
        return PTR_ERR(sysfs_device);
    }

    /* Register all files in the sysfs directory at once */
    ret = sysfs_create_group(&sysfs_device->kobj, &regulator_group);
    if (ret) {
        device_destroy(sysfs_class, MKDEV(0, 0));
        class_destroy(sysfs_class);
        return ret;
    }

    return 0;
}

void temp_regulator_sysfs_exit(void) {
    /* Clean up sysfs files and destroy resources */
    if (sysfs_device != NULL) {
        sysfs_remove_group(&sysfs_device->kobj, &regulator_group);
        device_destroy(sysfs_class, MKDEV(0, 0));
        sysfs_device = NULL;
    }

    if (sysfs_class != NULL) {
        class_destroy(sysfs_class);
        sysfs_class = NULL;
    }
}
