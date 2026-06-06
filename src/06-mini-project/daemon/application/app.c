#include "app.h"
#include "sysfs.h"

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

static LED* led;

/* Threads objects */
static pthread_t anim_thread_id;
static pthread_mutex_t anim_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t anim_condition = PTHREAD_COND_INITIALIZER; // SHARED RESOURCES

/* The counter of pending animations */
static int pending_animations = 0; // SHARED RESOURCES
static int keep_running = 1;

void btn_set_led(LED* l) {
    if (l != NULL) {
        led = l;
    }
}

void set_period(uint32_t period_ms){
    sysfs_set_period(period_ms);
}

uint32_t get_period() {
    return sysfs_get_period();
}

void increase_period() {
    uint32_t current_period = sysfs_get_period();
    set_period(current_period + GAP_PERIOD_MS);
}

void decrease_period() {
    uint32_t current_period = sysfs_get_period();
    set_period(current_period - GAP_PERIOD_MS);
}

int get_mode() {
    return sysfs_get_mode();
}

void set_mode(int mode) {
    sysfs_set_mode(mode);
}

void mode_toggle() {
    int current_mode = sysfs_get_mode();
    set_mode(current_mode ^ 1);
}

float get_temperature() {
    return sysfs_get_temperature();
}


/**
 * animation_worker() - The single thread that processes the queue
 */
static void* animation_worker(void* arg) {
    while (keep_running) {
        // ENTER CRITICAL SECTION
        pthread_mutex_lock(&anim_lock);

        /*
         * As long as there are no animations to do, the thread sleeps here.
         * It consumes 0% CPU while waiting.
         */
        while (pending_animations == 0 && keep_running) {
            /*
             * Wait signal to make animation.
             * The function unlocks the mutex to allow another process to add an animation.
             * The function forces the thread to sleep until a signal is received.
             */
            pthread_cond_wait(&anim_condition, &anim_lock);
        }

        if (!keep_running) {
            pthread_mutex_unlock(&anim_lock);
            break;
        }

        /* We take one animation from the queue */
        pending_animations--;

        pthread_mutex_unlock(&anim_lock);
        // LEAVE CRITICAL SECTION

        /* Perform the visual task */
        if (led != NULL) {
            LED_on(led);
            usleep(150000);
            LED_off(led);
            usleep(100000); /* Small delay between consecutive pulses */
        }
    }
    return NULL;
}


/**
 * init_animations() - To be called once during daemon startup
 */
void init_animations(void) {
    pthread_create(&anim_thread_id, NULL, animation_worker, NULL);

}

/**
 * trigger_pulse() - Increments the counter and wakes up the thread
 */
static void trigger_pulse(void) {

    // ENTER CRITICAL SECTION
    pthread_mutex_lock(&anim_lock);

    pending_animations++;

    /* Signal the thread that there is work to do */
    pthread_cond_signal(&anim_condition);

    pthread_mutex_unlock(&anim_lock);
    // LEAVE CRITICAL SECTION
}

void btn_increase_period() {
    trigger_pulse();
    increase_period();
}

void btn_decrease_period() {
    trigger_pulse();
    decrease_period();
}
