#include "include/utils.h"
#include "include/print.h"
#include "datatype.h"
#include "lib.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void print_val_internal(val_t *val, bool quote_strings)
{
    if (val == NULL)
    {
        puts("[NULL]");
        return;
    }

    switch (val->type)
    {
        case VAL_INTEGER:
            printf("\033[1;33m%lld\033[0m", val->intval);
            break;

        case VAL_FLOAT:
            printf("\033[1;33m%Lf\033[0m", val->floatval);
            break;

        case VAL_STRING:
            printf("\033[32m%s%s%s\033[0m", quote_strings ? "\"" : "", val->strval, quote_strings ? "\"" : "");
            break;

        case VAL_BOOLEAN:
            printf("\033[36m%s\033[0m", val->boolval == true ? "true" : "false");
            break;

        case VAL_NULL:
            printf("\033[2mnull\033[0m");
            break;

        case VAL_ARRAY:
            printf("\033[34mArray (%zu)\033[0m [", val->arrval->length);

            for (size_t i = 0; i < val->arrval->length; i++)
            {
                print_val_internal((val_t *) val->arrval->data[i], true);

                if (i != val->arrval->length - 1)
                    printf(", ");
            }

            printf("]");
            break;

        case VAL_FUNCTION:
            printf("\033[2m[Function%s]\033[0m", val->fnval->type == FN_USER_CUSTOM ? "" : " Built-in");
            break;

        default:
            libblaze_fatal_error("unrecognized value type: %d", val->type);
    }
}

void print_val(val_t *val)
{
    print_val_internal(val, true);
    printf("\n");
}