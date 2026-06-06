#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/errno.h>

#include "temp.h"

#define TEMPERATURE_SENSOR_BASE_ADDR 0x01C25000
#define TEMPERATURE_SENSOR_REG_SIZE  0x1000
#define TEMPERATURE_REG_OFFSET       0x80

static struct resource *temp_res = NULL;
static void __iomem *temp_reg = NULL;

int temp_init(void) {
    /* Request physical memory region */
    temp_res = request_mem_region(TEMPERATURE_SENSOR_BASE_ADDR,
                                  TEMPERATURE_SENSOR_REG_SIZE,
                                  "nanopi - temperature sensor");
    if (temp_res == NULL) {
        pr_warn("temp_regulator: Failed to reserve physical memory region\n");
    }

    /* Map the physical memory to virtual kernel memory */
    temp_reg = ioremap(TEMPERATURE_SENSOR_BASE_ADDR, TEMPERATURE_SENSOR_REG_SIZE);
    if (temp_reg == NULL) {
        pr_err("temp_regulator: Failed to ioremap registers\n");
        /* Clean up previously requested region */
        release_mem_region(TEMPERATURE_SENSOR_BASE_ADDR, TEMPERATURE_SENSOR_REG_SIZE);
        temp_res = NULL;
        return -ENOMEM;
    }

    pr_info("temp_regulator: Temperature sensor memory successfully mapped\n");
    return 0;
}

uint32_t read_temp(void) {
    uint32_t temperature = 0;
    uint32_t raw_val = 0;

    if (temp_reg == NULL) {
        pr_warn("temp_regulator: Cannot read temperature, sensor not initialized\n");
        return 0;
    }

    /* Read the raw register value from THS0_DATA (offset 0x80) */
    raw_val = ioread32(temp_reg + TEMPERATURE_REG_OFFSET);

    /* Choose the correct formula according to the datasheet */
    if (raw_val > 0x500) {
        /* Temperature is lower than 70°C */
        temperature = -1191 * (int32_t)raw_val / 10 + 223000;
    } else {
        /* Temperature is higher than or equal to 70°C */
        temperature = -1452 * (int32_t)raw_val / 10 + 259000;
    }

    return temperature;
}

void temp_exit(void) {
    /* Unmap virtual address space */
    if (temp_reg != NULL) {
        iounmap(temp_reg);
        temp_reg = NULL;
    }

    /* Release physical memory region */
    if (temp_res != NULL) {
        release_mem_region(TEMPERATURE_SENSOR_BASE_ADDR, TEMPERATURE_SENSOR_REG_SIZE);
        temp_res = NULL;
    }

    pr_info("temp_regulator: Temperature sensor resources released\n");
}
