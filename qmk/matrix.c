#include <stdbool.h>
#include "matrix.h"
#include "debounce.h"
#include "print.h"
#include "config.h"

static pin_t row_pins[MATRIX_ROWS_PER_HAND] = MATRIX_ROW_PINS;
static pin_t col_pins[MATRIX_COLS] = MATRIX_COL_PINS;

matrix_row_t raw_matrix[MATRIX_ROWS]; /* Raw values */
matrix_row_t matrix[MATRIX_ROWS]; /* Debounced values */

__attribute__((weak)) void matrix_init_kb(void) { matrix_init_user(); }

__attribute__((weak)) void matrix_scan_kb(void) { matrix_scan_user(); }

__attribute__((weak)) void matrix_init_user(void) {}

__attribute__((weak)) void matrix_scan_user(void) {}

/* From qmk_firmware/quantum/matrix_common.c */
inline matrix_row_t matrix_get_row(uint8_t row) {
    return matrix[row];
}

/* From qmk_firmware/quantum/matrix_common.c */
void matrix_print(void) {
    print("\nr/c 0123456789ABCDEF\n");

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        print_hex8(row);
        print(": ");
        print_bin_reverse16(matrix_get_row(row));
        print("\n");
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
void matrix_init_pins(void) {
    unselect_rows();
    for (uint8_t x = 0; x < MATRIX_COLS; x++) {
        if (col_pins[x] != NO_PIN) {
            gpio_atomic_set_pin_input_high(col_pins[x]);
        }
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
// TODO: Implement MCP initialisation
// TODO: Might be able to use lite matrix so maybe check with mcp23017 matrix example
/* From qmk_firmware/quantum/matrix.c */
void matrix_init(void) {
    /* Initialise matrix pins */
    matrix_init_pins();

    /* All keys off */
    memset(matrix, 0, sizeof(matrix));
    memset(raw_matrix, 0, sizeof(raw_matrix));

    /* Init the configured debounce routine */
    debounce_init();

    /* This *must* be called for correct keyboard behavior */
    matrix_init_kb();
}

// TODO: Implement MCP scanning
// TODO: Might be able to use lite matrix so maybe check with mcp23017 matrix example
/* From qmk_firmware/quantum/matrix.c */
uint8_t matrix_scan(void) {
    bool changed = false;
    matrix_row_t curr_matrix[MATRIX_ROWS] = {0};

    /* Set row, read columns */
    for (uint8_t current_row = 0; current_row < MATRIX_ROWS_PER_HAND; current_row++) {
        matrix_read_cols_on_row(curr_matrix, current_row);
    }

    changed = memcmp(raw_matrix, curr_matrix, sizeof(curr_matrix)) != 0;
    if (changed) memcpy(raw_matrix, curr_matrix, sizeof(curr_matrix));

    /* Use the configured debounce routine */
    changed = debounce(raw_matrix, matrix, changed);

    /* This *must* be called for correct keyboard behavior */
    matrix_scan_kb();

    return changed;
}
