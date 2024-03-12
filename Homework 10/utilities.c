#include "utilities.h"

volatile enum mode_t mode = IDLE;

enum mode_t get_mode() {
    return mode;
}

void set_mode(enum mode_t new_mode) {
    mode = new_mode;
}


