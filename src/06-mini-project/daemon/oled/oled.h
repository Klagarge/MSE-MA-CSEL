#ifndef MSE_MA_CSEL_OLED_H
#define MSE_MA_CSEL_OLED_H


#include <stdint.h>

/* Callback structure to decouple the OLED module from the application logic */
struct oled_callbacks_t {
    float (*get_temperature)(void);
    int (*get_mode)(void);
    uint32_t (*get_period)(void);
};

/*
 * Initialize the SSD1306 display and start the background refresh thread.
 * The cbs parameter provides the functions needed to fetch real-time data.
 */
void init_oled(struct oled_callbacks_t* cbs);

#endif //MSE_MA_CSEL_OLED_H
