#include "oled.h"

#include "ssd1306.h"

#include "oled.h"
#include "ssd1306.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

/* Internal reference to the registered callbacks */
static struct oled_callbacks_t* oled_cbs = NULL;

/* Thread handle for the background update loop */
static pthread_t oled_thread;

/**
 * display() - Formats and sends dynamic data to the OLED screen.
 *
 * This function uses snprintf to convert numerical values into strings
 * before sending them to the SSD1306 controller.
 */
static void display(int mode, float temp, uint32_t period_ms) {
    char buffer[20];

    /* Static header rows */
    ssd1306_set_position(0, 0);
    ssd1306_puts("CSEL1a - SP.07");
    // ssd1306_set_position(0, 1);
    // ssd1306_puts("  WATCHDOG CPU ");
    // ssd1306_set_position(0, 2);
    // ssd1306_puts("--------------");

    // /* Mode display: switches between AUTO and MANU strings */
    // ssd1306_set_position(0, 3);
    // snprintf(buffer, sizeof(buffer), "Mode: %s", mode ? "AUTO" : "MANU");
    // ssd1306_puts(buffer);

    // /* Temperature display: formatted with one decimal precision */
    // ssd1306_set_position(0, 4);
    // snprintf(buffer, sizeof(buffer), "Temp: %.1f'C", temp);
    // ssd1306_puts(buffer);

    // /* Period/Frequency display: shows the value in milliseconds */
    // ssd1306_set_position(0, 5);
    // float freq = 1.0f / (period_ms / 1000.0f);
    // snprintf(buffer, sizeof(buffer), "Freq: %.1fHz", freq);
    // ssd1306_puts(buffer);
}

/**
 * update_oled_thread() - Continuous loop for screen refreshing.
 *
 * Runs at a 250ms interval to provide a responsive display without
 * overloading the I2C bus.
 */
static void *update_oled_thread(void* arg) {
    while (1) {
        if (oled_cbs != NULL) {
            /* Fetch fresh data using callbacks and update screen */
            display(
                oled_cbs->get_mode(),
                oled_cbs->get_temperature(),
                oled_cbs->get_period()
            );
        }
        /* Sleep to control refresh rate and save CPU cycles */
        usleep(1000000);
    }
    return NULL;
}

void init_oled(struct oled_callbacks_t* cbs) {
    if (cbs != NULL) {
        oled_cbs = cbs;
    }

    /* Initialize the hardware driver */
    ssd1306_init();

    /* Start the background thread for automatic updates */
    pthread_create(&oled_thread, NULL, update_oled_thread, NULL);
    pthread_detach(oled_thread);
}
