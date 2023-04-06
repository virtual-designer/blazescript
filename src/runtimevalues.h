#ifndef __RUNTIMEVALUES_H__
#define __RUNTIMEVALUES_H__

#include <stdbool.h>
#include "map.h"

typedef enum 
{
    VAL_NUMBER,
    VAL_NULL,
    VAL_BOOLEAN,
    VAL_OBJECT
} runtime_valtype_t;

typedef struct runtime_val_t
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
        
        /* if (type == VAL_BOOLEAN) */
        bool boolval;
        /* endif */
        
        /* if (type == VAL_OBJECT) */
        map_t properties;
        /* endif */
    };
} runtime_val_t;

#endif
