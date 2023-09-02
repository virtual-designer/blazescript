/*
 * Created by rakinar2 on 8/30/23.
 */

#include "register.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

uint64_t registers[REG_COUNT];

static const char *register_id_to_str_lut[] = {
    [R0] = "r0",
    [R1] = "r1",
    [R2] = "r2",
    [R3] = "r3",
    [R4] = "r4",
    [R5] = "r5",
    [R6] = "r6",
    [R7] = "r7",
    [R8] = "r8",
    [R9] = "r9",
    [IP] = "ip",
    [IS] = "is",
};

bool is_valid_register_id(register_type_t id)
{
    size_t size = sizeof (register_id_to_str_lut) / sizeof (register_id_to_str_lut[0]);
    return size > ((size_t) id);
}

const char *register_id_to_str(register_type_t id)
{
    assert(is_valid_register_id(id) && "Invalid register ID");
    return register_id_to_str_lut[id];
}
