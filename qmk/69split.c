#include <stdbool.h>
#include "gpio.h"

/* Pre-init state of keyboard LEDs */
void keyboard_pre_init_kb(void) {
    gpio_set_pin_output(C6);
    gpio_write_pin_high(C6);

    gpio_set_pin_output(C7);
    gpio_write_pin_high(C7);

    gpio_set_pin_output(E6);
    gpio_write_pin_high(E6);
}

void keyboard_post_init_kb(void) {
    gpio_write_pin_low(C6);
    gpio_write_pin_low(C7);
    gpio_write_pin_low(E6);
}
