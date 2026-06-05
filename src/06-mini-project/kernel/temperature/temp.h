#ifndef TEMP_H
#define TEMP_H

#include <linux/types.h>

/**
 * temp_init() - Request physical memory and map it into kernel virtual memory.
 *
 * Return: 0 on success, or a negative error code on failure.
 */
int temp_init(void);

/**
 * read_temp() - Read the current CPU temperature from the mapped registers.
 * To convert it in degrees Celsius, divide by 1000.0
 *
 * Return: The calculated temperature value.
 */
uint32_t read_temp(void);


/**
 * temp_exit() - Unmap the virtual memory and release the requested physical memory region.
 */
void temp_exit(void);

#endif /* TEMP_H */
