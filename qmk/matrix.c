#include <stdbool.h>
#include "matrix.h"
#include "mcp23018.h"
#include "config.h"

#define IOEXPANDER_COLS 8
#define IOEXPANDER_ADDR 0


void matrix_init_custom() {
    /* Initialise IO Expander */
    mcp23018_init(IOEXPANDER_ADDR);

    /* Set port A to all output high */
    mcp23018_set_config(IOEXPANDER_ADDR, mcp23018_PORTA, ALL_OUTPUT);
    mcp23018_set_output(IOEXPANDER_ADDR, mcp23018_PORTA, ALL_HIGH);

    /* Set port B to all input with pull-up enabled */
    mcp23018_set_config(IOEXPANDER_ADDR, mcp23018_PORTB, ALL_INPUT);
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    bool matrix_has_changed = false;

    // TODO: add matrix scanning routine here
    // TODO: Implement MCP scanning
    matrix_print();

    return matrix_has_changed;
}
