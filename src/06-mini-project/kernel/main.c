// temp_regulator.c
#include <linux/module.h>	// needed by all modules
#include <linux/init.h>		// needed for macros
#include <linux/kernel.h>	// needed for debugging
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/err.h>

#include "linux/printk.h"

#include "temperature/temp.h"

static struct task_struct *temp_thread = NULL;

/* Thread function that runs in the background */
static int temp_thread_fn(void *data) {
    pr_info("temp_regulator: Background thread started\n");

    while (!kthread_should_stop()) {
        read_temp();
        /* Wait 1 second (1000 milliseconds) */
        msleep(1000);
    }

    pr_info("temp_regulator: Background thread stopping\n");
    return 0;
}

static int __init temp_regulator_init(void) {
    pr_info("Linux module temp_regulator loading...\n");

    int ret;

    /* Initialize temperature sensor API */
       ret = temp_init();
       if (ret != 0) {
           pr_err("temp_regulator: Initialization failed with error %d\n", ret);
           return ret;
       }

    pr_info("Linux module temp_regulator loaded\n");
    return 0;
}

static void __exit temp_regulator_exit(void) {
    pr_info("Linux module temp_regulator unloading...\n");


    /* Release sensor resources */
    temp_exit();

    pr_info("Linux module temp_regulator unloaded\n");
}

module_init(temp_regulator_init);
module_exit(temp_regulator_exit);

MODULE_AUTHOR("Klagarge <remi@heredero.ch>");
MODULE_AUTHOR("Fastium <fastium.pro@proton.me>");
MODULE_DESCRIPTION("Temperature regulator");
MODULE_LICENSE("GPL");
