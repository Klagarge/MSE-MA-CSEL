#include "sysfs.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>



float get_temperature() {
    FILE *f = fopen(PATH_TEMPERATURE, "r");
    if (f == NULL) {
        return 0.0f; /* Return 0.0 if the kernel module is not loaded */
    }

    float temp = 0.0f;

    /*
    * The kernel formats the temperature as "XX.YYY" (e.g., "41.730").
    * Since we are in user space, we can simply read it as a float.
    */
    if (fscanf(f, "%f", &temp) == 1) {
        fclose(f);
        return temp;
    }

    fclose(f);
    return 0.0f;
}

uint32_t get_mode() {
    FILE *f = fopen(PATH_MODE, "r");
    if (f == NULL) return 0;

    uint32_t mode = 0;
    fscanf(f, "%u", &mode);
    fclose(f);

    return mode;
}

void set_mode(uint32_t mode) {
    FILE *f = fopen(PATH_MODE, "w");
    if (f == NULL) return;

    /* Write the integer as an ASCII string */
    fprintf(f, "%u\n", mode);
    fclose(f);
}

uint32_t get_period() {
    FILE *f = fopen(PATH_PERIOD_ST, "r");
    if (f == NULL) return 0;

    uint32_t period = 0;
    fscanf(f, "%u", &period);
    fclose(f);

    return period;
}

void set_period(uint32_t period) {
    FILE *f = fopen(PATH_PERIOD_SET, "w");
    if (f == NULL) return;

    fprintf(f, "%u\n", period);
    fclose(f);
}
