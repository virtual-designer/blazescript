/*
 * Created by rakinar2 on 8/24/23.
 */

#ifndef BLAZESCRIPT_EVAL_H
#define BLAZESCRIPT_EVAL_H

#include "ast.h"
#include "datatype.h"

void val_free(val_t *val);
val_t *eval(const ast_node_t *node);
void print_val(val_t *val);
const char *val_type_to_str(val_type_t type);

#endif /* BLAZESCRIPT_EVAL_H */
