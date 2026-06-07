#include "sysfs.h"
#include <stdint.h>
#include <stdio.h>

#include <pthread.h>

static pthread_mutex_t sysfs_lock = PTHREAD_MUTEX_INITIALIZER;

float sysfs_get_temperature() {
    pthread_mutex_lock(&sysfs_lock);
    FILE *f = fopen(PATH_TEMPERATURE, "r");
    if (f == NULL) {
        pthread_mutex_unlock(&sysfs_lock);
        return 0.0f; /* Return 0.0 if the kernel module is not loaded */
    }

    float temp = 0.0f;

    /*
    * The kernel formats the temperature as "XX.YYY" (e.g., "41.730").
    * Since we are in user space, we can simply read it as a float.
    */
    if (fscanf(f, "%f", &temp) == 1) {
        fclose(f);
        pthread_mutex_unlock(&sysfs_lock);
        return temp;
    }

    fclose(f);
    pthread_mutex_unlock(&sysfs_lock);
    return 0.0f;
}

uint32_t sysfs_get_mode() {
    pthread_mutex_lock(&sysfs_lock);
    FILE *f = fopen(PATH_MODE, "r");
    if (f == NULL) {
        pthread_mutex_unlock(&sysfs_lock);
        return 0;
    }

    uint32_t mode = 0;
    fscanf(f, "%u", &mode);
    fclose(f);
    pthread_mutex_unlock(&sysfs_lock);

    return mode;
}

void sysfs_set_mode(uint32_t mode) {
    pthread_mutex_lock(&sysfs_lock);
    FILE *f = fopen(PATH_MODE, "w");
    if (f == NULL) {
        pthread_mutex_unlock(&sysfs_lock);
        return;
    }

    /* Write the integer as an ASCII string */
    fprintf(f, "%u\n", mode);
    fclose(f);
    pthread_mutex_unlock(&sysfs_lock);
}

uint32_t sysfs_get_period() {
    pthread_mutex_lock(&sysfs_lock);
    FILE *f = fopen(PATH_PERIOD_ST, "r");
    if (f == NULL) {
        pthread_mutex_unlock(&sysfs_lock);
        return 0;
    }

    uint32_t period = 0;
    fscanf(f, "%u", &period);
    fclose(f);
    pthread_mutex_unlock(&sysfs_lock);

    return period;
}

void sysfs_set_period(uint32_t period) {
    pthread_mutex_lock(&sysfs_lock);
    FILE *f = fopen(PATH_PERIOD_SET, "w");
    if (f == NULL) {
        pthread_mutex_unlock(&sysfs_lock);
        return;
    }

    fprintf(f, "%u\n", period);
    fclose(f);
    pthread_mutex_unlock(&sysfs_lock);
}
