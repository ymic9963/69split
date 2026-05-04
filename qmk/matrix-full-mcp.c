#include <string.h>
#include "matrix.h"
#include "debounce.h"
#include "wait.h"
#include "print.h"
#include "debug.h"
#include "atomic_util.h"
#include "mcp23018.h"

// maybe re-write mcp23018_init() to reduce delay
#define IOEXPANDER_COLS_START_INDEX 7
#define IOEXPANDER_ADDR 0x20
#define IOEXPANDER_COLS_PORT mcp23018_PORTA
#define IOEXPANDER_ROWS_PORT mcp23018_PORTB
#define MATRIX_INPUT_PRESSED_STATE 0
#define MATRIX_IO_DELAY 30

/* matrix state(1:on, 0:off) */
matrix_row_t raw_matrix[MATRIX_ROWS];
matrix_row_t matrix[MATRIX_ROWS];

static const pin_t row_pins[MATRIX_ROWS] = MATRIX_ROW_PINS;
static const pin_t col_pins[MATRIX_COLS] = MATRIX_COL_PINS;

// user-defined overridable functions

__attribute__((weak)) void matrix_init_kb(void) {
    matrix_init_user();
}

__attribute__((weak)) void matrix_scan_kb(void) {
    matrix_scan_user();
}

__attribute__((weak)) void matrix_init_user(void) {}

__attribute__((weak)) void matrix_scan_user(void) {}

// helper functions

inline matrix_row_t matrix_get_row(uint8_t row) {
    // Matrix mask lets you disable switches in the returned matrix data. For example, if you have a
    // switch blocker installed and the switch is always pressed.
    return matrix[row];
}

#define print_matrix_header() print("\nr/c 0123456789ABCDEF\n")
#define print_matrix_row(row) print_bin_reverse16(matrix_get_row(row))

void matrix_print(void) {
    print_matrix_header();

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        print_hex8(row);
        print(": ");
        print_matrix_row(row);
        print("\n");
    }
}

/* `matrix_io_delay ()` exists for backwards compatibility. From now on, use matrix_output_unselect_delay(). */
__attribute__((weak)) void matrix_io_delay(void) {
    wait_us(MATRIX_IO_DELAY);
}

__attribute__((weak)) void matrix_output_select_delay(void) {
    waitInputPinDelay();
}
__attribute__((weak)) void matrix_output_unselect_delay(uint8_t line, bool key_pressed) {
    matrix_io_delay();
}

static inline void gpio_atomic_set_pin_output_low(pin_t pin) {
    ATOMIC_BLOCK_FORCEON {
        gpio_set_pin_output(pin);
        gpio_write_pin_low(pin);
    }
}

static inline void gpio_atomic_set_pin_output_high(pin_t pin) {
    ATOMIC_BLOCK_FORCEON {
        gpio_set_pin_output(pin);
        gpio_write_pin_high(pin);
    }
}

static inline void gpio_atomic_set_pin_input_high(pin_t pin) {
    ATOMIC_BLOCK_FORCEON {
        gpio_set_pin_input_high(pin);
    }
}

static inline uint8_t readMatrixPin(pin_t pin) {
    if (pin != NO_PIN) {
        return (gpio_read_pin(pin) == MATRIX_INPUT_PRESSED_STATE) ? 0 : 1;
    } else {
        return 1;
    }
}

// bit 0 is port 0
// you set everything in one go, there is no way of not setting everything together
static bool io_expander_set_pin_output_low(uint8_t pin) {
    const uint8_t base_conf  = 0xE0;

    // row 1 => 0b00000010 => invert 0b11111101 => XOR with 0b11100000 => 0b11111101
    uint8_t row_port = ~(0x1 << pin);
    uint8_t conf = base_conf ^ row_port;

    mcp23018_set_config(IOEXPANDER_ADDR, IOEXPANDER_ROWS_PORT, conf);
    mcp23018_set_output(IOEXPANDER_ADDR, IOEXPANDER_ROWS_PORT, conf);
}

static bool select_row(uint8_t row) {
    pin_t pin = row_pins[row];
    if (pin != NO_PIN) {
        // Selecting the rows must happen on both MCU and IO expander
        gpio_atomic_set_pin_output_low(pin);
        // TODO: test it first????? have it in the scan routine
        io_expander_set_pin_output_low(row); // comment out for default
        return true;
    }
    return false;
}

static void unselect_row(uint8_t row) {
    pin_t pin = row_pins[row];
    if (pin != NO_PIN) {
        // Unselecting the rows must happen on both MCU and IO expander
        // TODO: create io_expander_set_pin_input_high()
        gpio_atomic_set_pin_input_high(pin);
    }
}

static void unselect_rows(void) {
    for (uint8_t x = 0; x < MATRIX_ROWS; x++) {
        unselect_row(x);
    }
}

__attribute__((weak)) void matrix_init_pins(void) {
    unselect_rows();
    for (uint8_t x = 0; x < MATRIX_COLS; x++) {
        if (col_pins[x] != NO_PIN) {
            gpio_atomic_set_pin_input_high(col_pins[x]);
        }
    }
}

/* Emulate matrix_init_pins() for IO expander */
void matrix_init_io_expander(void) {
    /* Initialise IO Expander */
    mcp23018_init(IOEXPANDER_ADDR);

    /* Set port B (rows) to all input with pull-up enabled */
    if (mcp23018_set_config(IOEXPANDER_ADDR, IOEXPANDER_ROWS_PORT, ALL_INPUT) == false) {
        gpio_write_pin_high(E6);
        return;
    }

    /* Set port A (rows) to all input with pull-up enabled */
    if (mcp23018_set_config(IOEXPANDER_ADDR, IOEXPANDER_COLS_PORT, ALL_INPUT) == false) {
        gpio_write_pin_high(E6);
        return;
    }
}

__attribute__((weak)) void matrix_init(void) {
    matrix_init_io_expander();

    // initialize key pins
    matrix_init_pins();

    // initialize matrix state: all keys off
    memset(matrix, 0, sizeof(matrix));
    memset(raw_matrix, 0, sizeof(raw_matrix));

    debounce_init();

    matrix_init_kb();
}

void matrix_read_cols_on_row(matrix_row_t current_matrix[], uint8_t current_row) {
    // Start with a clear matrix row
    matrix_row_t current_row_value = 0;

    if (!select_row(current_row)) { // Select row
        return;                     // skip NO_PIN row
    }

    // TODO: would this one require an equivalen for io exp?
    matrix_output_select_delay();

    // For each col...
    matrix_row_t row_shifter = MATRIX_ROW_SHIFTER;
    for (uint8_t col_index = 0; col_index < MATRIX_COLS; col_index++, row_shifter <<= 1) {
        uint8_t pin_state = 0;
        // TODO: io exp read pin
        pin_state = readMatrixPin(col_pins[col_index]);

        // Populate the matrix row with the state of the col pin
        current_row_value |= pin_state ? 0 : row_shifter;
    }

    // Unselect row
    unselect_row(current_row);

    // TODO: would this one require an equivalen for io exp?
    matrix_output_unselect_delay(current_row, current_row_value != 0); // wait for all Col signals to go HIGH

    // Update the matrix
    current_matrix[current_row] = current_row_value;
}

uint8_t matrix_scan(void) {
    matrix_row_t curr_matrix[MATRIX_ROWS] = {0};

    // Set row, read cols
    for (uint8_t current_row = 0; current_row < MATRIX_ROWS; current_row++) {
        matrix_read_cols_on_row(curr_matrix, current_row);
    }

    bool changed = memcmp(raw_matrix, curr_matrix, sizeof(curr_matrix)) != 0;
    if (changed) memcpy(raw_matrix, curr_matrix, sizeof(curr_matrix));

    changed = debounce(raw_matrix, matrix, changed);
    matrix_scan_kb();

    return (uint8_t)changed;
}
