#ifndef __RUNTIMEVALUES_H__
#define __RUNTIMEVALUES_H__

#include <stdbool.h>

typedef enum 
{
    VAL_NUMBER,
    VAL_NULL
} runtime_valtype_t;

typedef struct 
{
    runtime_valtype_t type;
    union {
        /* if (type == VAL_NUMBER) */
        struct {
            long long int intval;
            long double floatval;
            bool is_float;
        };
        /* endif */
    };
} runtime_val_t;

#endif
