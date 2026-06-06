// workspace/src/06-mini-project/kernel/sysfs/sysfs.h
#ifndef SYSFS_H
#define SYSFS_H

#include <linux/types.h>

/* Structure holding all the necessary callbacks to interact with the hardware */
struct sysfs_callbacks {
    uint32_t (*get_temperature)(void);

    int (*get_mode)(void);
    void (*set_mode)(int mode);

    uint32_t (*get_period)(void);
    void (*set_period)(uint32_t period_ms);
};

/* Register the callbacks and create the sysfs interface */
int temp_regulator_sysfs_init(struct sysfs_callbacks *cbs);

/* Remove sysfs files and destroy device and class */
void temp_regulator_sysfs_exit(void);

#endif /* SYSFS_H */
