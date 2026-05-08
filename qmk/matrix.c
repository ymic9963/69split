#include <stdbool.h>
#include <string.h>
#include "i2c_master.h"
#include "wait.h"
#include "mcp23018.h"
#include "atomic_util.h"
#include "matrix.h"
#include "gpio.h"
#include "config.h"

#define IOEXPANDER_COLS_START_INDEX 7
#define IOEXPANDER_ADDR 0x20
#define IOEXPANDER_COLS_PORT mcp23018_PORTA
#define IOEXPANDER_ROWS_PORT mcp23018_PORTB

static pin_t row_pins[MATRIX_ROWS_PER_HAND] = MATRIX_ROW_PINS;
static pin_t col_pins[MATRIX_COLS] = MATRIX_COL_PINS;

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

// TODO: create set output high and input low
static inline uint8_t mcp23018_read_pin(uint8_t port, uint8_t pin) {
    uint8_t pins = 0;
    mcp23018_read_pins(IOEXPANDER_ADDR, port, &pins);

    uint8_t pin_mask = 1 << pin;
    uint8_t pin_value = pins & pin_mask;

    return (pin_value == 0) ? 0 : 1;
}

static inline void mcp23018_set_pin_input_high(uint8_t port, uint8_t pin) {
    uint8_t cur_port_value = 0;
    mcp23018_read_pins(IOEXPANDER_ADDR, port, &cur_port_value);

    uint8_t row_port = 1 << pin;
    uint8_t conf = cur_port_value & ~(row_port); // clear bit first
    conf |= row_port; // set bit

    mcp23018_set_config(IOEXPANDER_ADDR, port, conf);
}

static inline void mcp23018_set_pin_output_low(uint8_t port, uint8_t pin) {
    uint8_t cur_port_value = 0;
    mcp23018_read_pins(IOEXPANDER_ADDR, port, &cur_port_value);

    uint8_t row_port = ~(1 << pin);
    uint8_t conf = cur_port_value & row_port;

    mcp23018_set_config(IOEXPANDER_ADDR, port, conf);
    mcp23018_set_output(IOEXPANDER_ADDR, port, conf);
}

/* Adapted from qmk_firmware/quantum/matrix.c */
static bool select_row(uint8_t row) {
    pin_t pin = row_pins[row];
    if (pin != NO_PIN) {
        // Selecting the rows must happen on both MCU and IO expander
        gpio_atomic_set_pin_output_low(pin);
        mcp23018_set_pin_output_low(IOEXPANDER_ROWS_PORT, row); // comment out for default
        return true;
    }
    return false;
}

/* Adapted from qmk_firmware/quantum/matrix.c */
static void unselect_row(uint8_t row) {
    pin_t pin = row_pins[row];
    if (pin != NO_PIN) {
        // Unselecting the rows must happen on both MCU and IO expander
        gpio_atomic_set_pin_input_high(pin);
        mcp23018_set_pin_input_high(IOEXPANDER_ROWS_PORT, row);
    }
}

/* From qmk_firmware/quantum/matrix.c */
static inline uint8_t readMatrixPin(pin_t pin) {
    if (pin != NO_PIN) {
        return (gpio_read_pin(pin) == 0) ? 0 : 1;
    } else {
        return 1;
    }
}

 /* Adapted from qmk_firmware/quantum/matrix.c */
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
        if (col_index < IOEXPANDER_COLS_START_INDEX) {
            pin_state = readMatrixPin(col_pins[col_index]);
        } else {
            pin_state = mcp23018_read_pin(IOEXPANDER_COLS_PORT, col_index - IOEXPANDER_COLS_START_INDEX );
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

 /* Adapted from qmk_firmware/drivers/gpio/mcp23018.c to reduce delay */
void mcp23018_init_fast(uint8_t addr) {
    static uint8_t s_init = 0;
    if (!s_init) {
        i2c_init();
        wait_ms(10);

        s_init = 1;
    }
}

 /* Adapted from qmk_firmware/quantum/matrix.c */
void matrix_init_custom(void) {
    /* Initialise matrix pins */
    for (uint8_t x = 0; x < MATRIX_ROWS; x++) {
        if (row_pins[x] != NO_PIN) {
            gpio_atomic_set_pin_input_high(row_pins[x]);
        }
    }

    for (uint8_t x = 0; x < MATRIX_COLS; x++) {
        if (col_pins[x] != NO_PIN) {
            gpio_atomic_set_pin_input_high(col_pins[x]);
        }
    }

    /* Initialise IO Expander */
    mcp23018_init_fast(IOEXPANDER_ADDR);

    /* Set port A (rows) to all input with pull-up enabled */
    if (mcp23018_set_config(IOEXPANDER_ADDR, IOEXPANDER_COLS_PORT, ALL_INPUT) == false) {
        gpio_write_pin_high(C6);
        return;
    }

    /* Set port B (rows) to all input with pull-up enabled */
    if (mcp23018_set_config(IOEXPANDER_ADDR, IOEXPANDER_ROWS_PORT, ALL_INPUT) == false) {
        gpio_write_pin_high(C7);
        return;
    }
}

/* Called in qmk_firmware/quantum/matrix_common.c
 * Adapted from qmk_firmware/quantum/matrix.c
 * */
bool matrix_scan_custom(matrix_row_t raw_matrix[]) {
    matrix_row_t curr_matrix[MATRIX_ROWS] = {0};

    for (uint8_t current_row = 0; current_row < MATRIX_ROWS; current_row++) {
        matrix_read_cols_on_row(curr_matrix, current_row);
    }

    bool changed = memcmp(raw_matrix, curr_matrix, sizeof(curr_matrix)) != 0;
    if (changed) memcpy(raw_matrix, curr_matrix, sizeof(curr_matrix));

    return changed;
}
