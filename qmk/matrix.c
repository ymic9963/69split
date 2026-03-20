#include <stdbool.h>
#include <string.h>
#include "mcp23018.h"
#include "atomic_util.h"
#include "matrix.h"
#include "gpio.h"
#include "config.h"

#define IOEXPANDER_COLS 8
#define IOEXPANDER_ADDR 0

/* All other initialisations happen in qmk_firmware/quantum/matrix_common.c */
void matrix_init_custom() {
    /* Initialise IO Expander */
    mcp23018_init(IOEXPANDER_ADDR);

    /* Set port A (columns) to all output high */
    mcp23018_set_config(IOEXPANDER_ADDR, mcp23018_PORTA, ALL_OUTPUT);
    mcp23018_set_output(IOEXPANDER_ADDR, mcp23018_PORTA, ALL_HIGH);

    /* Set port B (rows) to all input with pull-up enabled */
    mcp23018_set_config(IOEXPANDER_ADDR, mcp23018_PORTB, ALL_INPUT);
}

/* From qmk_firmware/quantum/matrix.c */
static inline void gpio_atomic_set_pin_output_low(pin_t pin) {
    ATOMIC_BLOCK_FORCEON {
        gpio_set_pin_output(pin);
        gpio_write_pin_low(pin);
    }
}

/* From qmk_firmware/quantum/matrix.c */
static inline void gpio_atomic_set_pin_output_high(pin_t pin) {
    ATOMIC_BLOCK_FORCEON {
        gpio_set_pin_output(pin);
        gpio_write_pin_high(pin);
    }
}

/* From qmk_firmware/quantum/matrix.c */
static inline void gpio_atomic_set_pin_input_high(pin_t pin) {
    ATOMIC_BLOCK_FORCEON {
        gpio_set_pin_input_high(pin);
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

/* From qmk_firmware/quantum/matrix.c */
static bool select_row(uint8_t row) {
    pin_t pin = row_pins[row];
    if (pin != NO_PIN) {
        gpio_atomic_set_pin_output_low(pin);
        return true;
    }
    return false;
}

/* From qmk_firmware/quantum/matrix.c */
static void unselect_row(uint8_t row) {
    pin_t pin = row_pins[row];
    if (pin != NO_PIN) {
        gpio_atomic_set_pin_input_high(pin);
    }
}

/* From qmk_firmware/quantum/matrix.c */
static void unselect_rows(void) {
    for (uint8_t x = 0; x < MATRIX_ROWS_PER_HAND; x++) {
        unselect_row(x);
    }
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
        uint8_t pin_state = readMatrixPin(col_pins[col_index]);

        // Populate the matrix row with the state of the col pin
        current_row_value |= pin_state ? 0 : row_shifter;
    }

    // Unselect row
    unselect_row(current_row);
    matrix_output_unselect_delay(current_row, current_row_value != 0); // wait for all Col signals to go HIGH

    // Update the matrix
    current_matrix[current_row] = current_row_value;
}

/* Called in qmk_firmware/quantum/matrix_common.c */
bool matrix_scan_custom(matrix_row_t raw_matrix[]) {
    matrix_print();
    bool matrix_has_changed = false;
    matrix_row_t curr_matrix[MATRIX_ROWS] = {0};

    // TODO: Implement MCP scanning
    // scan the columns of the MCP

    // TODO: add matrix scanning routine here
    for (uint8_t current_row = 0; current_row < MATRIX_ROWS_PER_HAND; current_row++) {
        matrix_read_cols_on_row(curr_matrix, current_row);
    }
    bool changed = memcmp(raw_matrix, curr_matrix, sizeof(curr_matrix)) != 0;
    if (changed) memcpy(raw_matrix, curr_matrix, sizeof(curr_matrix));

    return matrix_has_changed;
}
