#ifndef SYSFS_H
#define SYSFS_H

#include <stdint.h>

/* Define the absolute paths to the sysfs files created by the kernel */
#define SYSFS_BASE_PATH  "/sys/class/temp_regulator/regulator"
#define PATH_TEMPERATURE SYSFS_BASE_PATH "/temperature"
#define PATH_MODE        SYSFS_BASE_PATH "/mode"
#define PATH_PERIOD_ST   SYSFS_BASE_PATH "/period_status"
#define PATH_PERIOD_SET  SYSFS_BASE_PATH "/period_set"

float sysfs_get_temperature();
uint32_t sysfs_get_mode();
void sysfs_set_mode(uint32_t mode);
uint32_t sysfs_get_period();
void sysfs_set_period(uint32_t period);


#endif /* SYSFS_H */
