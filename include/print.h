/*
 * Created by rakinar2 on 9/22/23.
 */

#ifndef BLAZESCRIPT_PRINT_H
#define BLAZESCRIPT_PRINT_H

#include <stdbool.h>
#include "datatype.h"

void print_val(val_t *val);
void print_val_internal(val_t *val, bool quote_strings);

#endif /* BLAZESCRIPT_PRINT_H */
