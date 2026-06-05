#include "blink.h"

static struct task_struct* blink_thread;
atomic_t blink_period_ms;

int thread_blinkThread(void* data);

void blink_init(void) {
    int ret = 0;
    pr_info("Initialize blink thread\n");

    atomic_set(&blink_period_ms, DEFAULT_PERIOD_MS);

    ret = gpio_request(GPIO_PIN, "blink-led");
    if(ret) {
        pr_err("Failed to request GPIO pin %d, error %d\n", GPIO_PIN, ret);
        return;
    }

    ret = gpio_direction_output(GPIO_PIN, 0);
    if(ret) {
        pr_err("Failed to set GPIO pin %d direction, error %d\n", GPIO_PIN, ret);
        gpio_free(GPIO_PIN);
        return;
    }

    blink_thread = kthread_run(thread_blinkThread, NULL, "blink_thread");
    if (IS_ERR(blink_thread)) {
        pr_err("Failed to create blink thread\n");
        gpio_free(GPIO_PIN);
        return;
    }

    pr_info("Blink thread initialized\n");
}

void blink_exit(void) {
    pr_info("Exiting blink thread\n");
    kthread_stop(blink_thread);
    gpio_free(GPIO_PIN);
    pr_info("Blink thread exited\n");
}

int thread_blinkThread(void* data) {

    bool state = false;
    int period = 0;
    
    pr_info("Blink started\n");

    while (!kthread_should_stop()) {
        gpio_set_value(GPIO_PIN, state);
        state = !state;
        period = atomic_read(&blink_period_ms);
        msleep(period >> 1);
    }

    return 0;
}

void adjust_frequency(int new_period_ms) {
    pr_info("Adjusting blink period to %d ms\n", new_period_ms);
    atomic_set(&blink_period_ms, new_period_ms);
}