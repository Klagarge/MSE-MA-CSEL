// temp_regulator.c
#include <linux/module.h>	// needed by all modules
#include <linux/init.h>		// needed for macros
#include <linux/kernel.h>	// needed for debugging

#include "linux/printk.h"

#include "temperature/temp.h"
#include "blink/blink.h"
#include "regulator/regulator.h"


static int __init temp_regulator_init(void) {
    pr_info("Linux module temp_regulator loading...\n");

    int ret;

    /* Initialize temperature sensor API */
    ret = temp_init();
    if (ret != 0) {
        pr_err("temp_regulator: Initialization failed with error %d\n", ret);
        return ret;
    }

    blink_init();


    struct regulator_callbacks cbs = {
        .adjust_period = adjust_period,
        .get_temperature = read_temp,
    };

    ret = regulator_init(&cbs);
    if (ret != 0) {
        pr_err("temp_regulator: Regulator initialization failed: %d\n", ret);
        blink_exit();
        temp_exit();
        return ret;
    }


    pr_info("Linux module temp_regulator loaded\n");
    return 0;
}

static void __exit temp_regulator_exit(void) {
    pr_info("Linux module temp_regulator unloading...\n");

    /* Stop the brain logic */
    regulator_exit();

    /* Release hardware resources */
    blink_exit();
    temp_exit();

    pr_info("Linux module temp_regulator unloaded\n");
}

module_init(temp_regulator_init);
module_exit(temp_regulator_exit);

MODULE_AUTHOR("Klagarge <remi@heredero.ch>");
MODULE_AUTHOR("Fastium <fastium.pro@proton.me>");
MODULE_DESCRIPTION("Temperature regulator");
MODULE_LICENSE("GPL");
