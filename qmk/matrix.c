#include <stdbool.h>
#include <string.h>
#include "mcp23018.h"
#include "atomic_util.h"
#include "matrix.h"
#include "gpio.h"
#include "config.h"

#define IOEXPANDER_COLS 8
#define IOEXPANDER_ADDR 0
#define IOEXPANDER_COLS_PORT mcp23018_PORTA
#define IOEXPANDER_ROWS_PORT mcp23018_PORTB
#define MATRIX_INPUT_PRESSED_STATE 0

static pin_t row_pins[MATRIX_ROWS_PER_HAND] = MATRIX_ROW_PINS;
static pin_t col_pins[MATRIX_COLS] = MATRIX_COL_PINS;

/* All other initialisations happen in qmk_firmware/quantum/matrix_common.c */
void matrix_init_custom(void) {
    /* Initialise IO Expander */
    mcp23018_init(IOEXPANDER_ADDR);

    /* Set port A (columns) to all output high */
    mcp23018_set_config(IOEXPANDER_ADDR, IOEXPANDER_COLS_PORT, ALL_OUTPUT);
    mcp23018_set_output(IOEXPANDER_ADDR, IOEXPANDER_COLS_PORT, ALL_HIGH);

    /* Set port B (rows) to all input with pull-up enabled */
    mcp23018_set_config(IOEXPANDER_ADDR, IOEXPANDER_ROWS_PORT, ALL_INPUT);
}

/* From qmk_firmware/quantum/matrix.c */
static inline void gpio_atomic_set_pin_output_low(pin_t pin) {
    ATOMIC_BLOCK_FORCEON {
        gpio_set_pin_output(pin);
        gpio_write_pin_low(pin);
    }
}

/* From qmk_firmware/quantum/matrix.c */
static inline void gpio_atomic_set_pin_input_high(pin_t pin) {
    ATOMIC_BLOCK_FORCEON {
        gpio_set_pin_input_high(pin);
    }
}

/* Set a single pin to output and to logic low */
static bool mcp23018_set_output_pin_low(uint8_t slave_addr, mcp23018_port_t port, uint8_t pin) {
    uint8_t conf = 0;
    uint8_t ret = 0;

    if (!mcp23018_read_pins(IOEXPANDER_ADDR, port, &ret)) {
        return false;
    }
    conf = ret | ~(0x00000001 << pin);

    if (!mcp23018_set_config(IOEXPANDER_ADDR, port, conf)) {
        return false;
    }

    return mcp23018_set_output(slave_addr, port, conf);
}

/* From qmk_firmware/quantum/matrix.c */
static bool select_row(uint8_t row) {
    pin_t pin = row_pins[row];
    if (row < IOEXPANDER_COLS) {
        if (pin != NO_PIN) {
            gpio_atomic_set_pin_output_low(pin);

            return true;
        }
    } else {
        mcp23018_set_output_pin_low(IOEXPANDER_ADDR, IOEXPANDER_ROWS_PORT, pin);

        return true;
    }

    return false;
}

/* Set a single pin to input and with pull-up enabled */
static bool mcp23018_set_input_pin_high(uint8_t slave_addr, mcp23018_port_t port, uint8_t pin) {
    uint8_t conf = 0;
    uint8_t ret = 0;

    if (!mcp23018_read_pins(IOEXPANDER_ADDR, port, &ret)) {
        return false;
    }
    conf = ret | (0x000000001 << pin);

    return mcp23018_set_config(IOEXPANDER_ADDR, IOEXPANDER_COLS_PORT, conf);
}

/* From qmk_firmware/quantum/matrix.c */
static void unselect_row(uint8_t row) {
    pin_t pin = row_pins[row];
    if (row < IOEXPANDER_COLS) {
        if (pin != NO_PIN) {
            gpio_atomic_set_pin_input_high(pin);
        }
    } else {
        mcp23018_set_input_pin_high(IOEXPANDER_ADDR, IOEXPANDER_ROWS_PORT, row);
    }
}

/* From qmk_firmware/quantum/matrix.c */
static inline uint8_t readMatrixPin(pin_t pin) {
    if (pin != NO_PIN) {
        return (gpio_read_pin(pin) == MATRIX_INPUT_PRESSED_STATE) ? 0 : 1;
    } else {
        return 1;
    }
}

/* Read an individual pin from the IO Expander */
static inline uint8_t mcp23018_read_pin(uint8_t slave_addr, mcp23018_port_t port, uint8_t pin) {
    uint8_t ret = 0;
    mcp23018_read_pins(slave_addr, port, &ret);

    return ret & pin;
}

/* From qmk_firmware/quantum/matrix.c */
void matrix_read_cols_on_row(matrix_row_t current_matrix[], uint8_t current_row) {
    // Start with a clear matrix row
    matrix_row_t current_row_value = 0;

    if (!select_row(current_row)) { // Select row
        return;                     // skip NO_PIN row
    }
    matrix_output_select_delay();

    // For each col...
    matrix_row_t row_shifter = MATRIX_ROW_SHIFTER;
    for (uint8_t col_index = 0; col_index < MATRIX_COLS; col_index++, row_shifter <<= 1) {
        uint8_t pin_state = 0;
        if (col_index < IOEXPANDER_COLS) {
            pin_state = readMatrixPin(col_pins[col_index]);
        } else {
            pin_state = mcp23018_read_pin(IOEXPANDER_ADDR, IOEXPANDER_COLS, col_index); 
        }

        // Populate the matrix row with the state of the col pin
        current_row_value |= pin_state ? 0 : row_shifter;
    }

    // Unselect row
    unselect_row(current_row);
    matrix_output_unselect_delay(current_row, current_row_value != 0); // wait for all Col signals to go HIGH

    // Update the matrix
    current_matrix[current_row] = current_row_value;
}

/* Called in qmk_firmware/quantum/matrix_common.c
 * Adapted from qmk_firmware/quantum/matrix.c
 * */
bool matrix_scan_custom(matrix_row_t raw_matrix[]) {
    matrix_print();
    bool matrix_has_changed = false;
    matrix_row_t curr_matrix[MATRIX_ROWS] = {0};

    for (uint8_t current_row = 0; current_row < MATRIX_ROWS; current_row++) {
        matrix_read_cols_on_row(curr_matrix, current_row);
    }

    bool changed = memcmp(raw_matrix, curr_matrix, sizeof(curr_matrix)) != 0;
    if (changed) memcpy(raw_matrix, curr_matrix, sizeof(curr_matrix));

    return matrix_has_changed;
}
