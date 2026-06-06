// workspace/src/06-mini-project/kernel/regulator/regulator.c
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/err.h>

#include "regulator.h"
#include "../sysfs/sysfs.h"

/* Internal state of the regulator */
static int current_mode = 1;        /* 1 = Auto, 0 = Manual */
static uint32_t current_period = 1000;   /* Current blinking period in ms */

static struct task_struct *regulator_thread = NULL;

static struct regulator_callbacks regulator_cbs = {0};

/* --- Sysfs Callbacks --- */

static uint32_t cb_get_temperature(void) {
    return regulator_cbs.get_temperature();
}

static int cb_get_mode(void) {
    return current_mode;
}

static void cb_set_mode(int mode) {
    /* Accept only 0 or 1 as valid modes */
    if (mode == 0 || mode == 1) {
        current_mode = mode;
        pr_info("regulator: Mode switched to %s\n", mode ? "Auto" : "Manual");
    }
}

static uint32_t cb_get_period(void) {
    return current_period;
}

static void cb_set_period(uint32_t period_ms) {
    /* Allow period changes only in Manual mode */
    if (current_mode == 0) {
        if (period_ms > 0) {
            current_period = period_ms;
            regulator_cbs.adjust_period(current_period);
            pr_info("regulator: Manual period set to %u ms\n", current_period);
        } else {
            pr_warn("regulator: Period must be greater than 0\n");
        }

    } else {
        pr_warn("regulator: Cannot set period manually in auto mode\n");
    }
}

/* Pack callbacks into the structure expected by sysfs */
static struct sysfs_callbacks sysfs_cbs = {
    .get_temperature = cb_get_temperature,
    .get_mode        = cb_get_mode,
    .set_mode        = cb_set_mode,
    .get_period   = cb_get_period,
    .set_period   = cb_set_period,
};

/* --- Auto Mode Logic --- */

static void process_auto_mode(void) {
    uint32_t temp_milli = regulator_cbs.get_temperature();
    uint32_t temp_c = temp_milli / 1000;
    uint32_t new_period = 2;

    /* Determine period based on temperature thresholds */
    if (temp_c < 35) {
        new_period = 500;
    } else if (temp_c < 40) {
        new_period = 200;
    } else if (temp_c < 45) {
        new_period = 100;
    } else {
        new_period = 50;
    }

    /* Apply only if the period has changed to avoid unnecessary hardware updates */
    if (new_period != current_period) {
        current_period = new_period;
        regulator_cbs.adjust_period(current_period);
        pr_info("regulator: Auto mode adjusted period to %u ms (Temp: %u C)\n",
                current_period, temp_c);
    }
}

/* Background thread checking the temperature periodically */
static int regulator_thread_fn(void *data) {
    while (!kthread_should_stop()) {
        if (current_mode == 1) {
            process_auto_mode();
        }
        msleep(1000);
    }
    return 0;
}

/* --- Public API --- */

int regulator_init(struct regulator_callbacks *cbs) {

    if (cbs == NULL) {
        pr_err("regulator: Callbacks are NULL\n");
        return -EINVAL;
    }

    // register callbacks
    regulator_cbs = *cbs;

    /* Initialize sysfs and inject our brain callbacks */
    int ret = temp_regulator_sysfs_init(&sysfs_cbs);
    if (ret != 0) {
        return ret;
    }

    /* Start the background decision loop */
    regulator_thread = kthread_run(regulator_thread_fn, NULL, "regulator_logic");
    if (IS_ERR(regulator_thread)) {
        temp_regulator_sysfs_exit();
        return PTR_ERR(regulator_thread);
    }

    return 0;
}

void regulator_exit(void) {

    if (regulator_thread != NULL) {
        kthread_stop(regulator_thread);
        regulator_thread = NULL;
    }

    temp_regulator_sysfs_exit();
}
