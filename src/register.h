/*
 * Created by rakinar2 on 8/30/23.
 */

#ifndef BLAZESCRIPT_REGISTER_H
#define BLAZESCRIPT_REGISTER_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    R0,
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    R7,
    R8,
    R9,
    IP,
    IS,
    REG_COUNT
} register_type_t;

const char *register_id_to_str(register_type_t id);
bool is_valid_register_id(register_type_t id);

extern uint64_t registers[REG_COUNT];

#endif /* BLAZESCRIPT_REGISTER_H */
