#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include "opcode.h"
#include "scope.h"
#include "vector.h"
#include "string.h"
#include "functions.h"
#include "stack.h"
#include "blaze.h"

#define OPCODE_HANDLER(name) static uint8_t *opcode_handler_##name(uint8_t *ip, bytecode_t *bytecode)
#define OPCODE_HANDLER_REF(name) opcode_handler_##name

static opcode_handler_t handlers[OPCODE_COUNT];
static stack_t global;
static scope_t global_scope;

OPCODE_HANDLER(nop)
{
    return ++ip;
}

opcode_handler_t opcode_get_handler(opcode_t opcode)
{
    if (opcode >= OPCODE_COUNT)
        return NULL;
    
    return handlers[opcode];
}

OPCODE_HANDLER(dump)
{
    stack_print(&global);
    return ++ip;
}

OPCODE_HANDLER(hlt)
{
#ifdef _DEBUG
#ifndef _NODEBUG
    (OPCODE_HANDLER_REF(dump))(ip, bytecode);
#endif
#endif
    scope_free(&global_scope);
    exit(errno != 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}

OPCODE_HANDLER(test)
{
    puts("test instruction");
    return ++ip;
}

OPCODE_HANDLER(push)
{
    stack_push(&global, (runtime_val_t) {
        .type = VAL_NUMBER,
        .is_float = false,
        .intval = *++ip
    });

    return ++ip;
}

OPCODE_HANDLER(pop)
{
    stack_pop(&global);
    return ++ip;
}

static void binary_operation(bytecode_t *bytecode, ast_operator_t operator)
{
    runtime_val_t num2 = stack_pop(&global);
    runtime_val_t num1 = stack_pop(&global);

    if (num1.type != VAL_NUMBER)
        bytecode_set_error(bytecode, "Value #1 in stack is not a number");
    else if (num2.type != VAL_NUMBER)
        bytecode_set_error(bytecode, "Value #0 in stack is not a number");

    if (operator == OP_DIVIDE || operator == OP_MOD)
    {
        if (num2.intval == 0)
            bytecode_set_error(bytecode, "Cannot divide by 0 (value #0)");
    }

    stack_push(&global, (runtime_val_t) {
        .type = VAL_NUMBER,
        .intval = operator == OP_PLUS ? num1.intval + num2.intval : (
            operator == OP_MINUS ? num1.intval - num2.intval : (
                operator == OP_TIMES ? num1.intval * num2.intval : (
                    operator == OP_DIVIDE ? num1.intval / num2.intval : (
                        num1.intval % num2.intval
                    )
                )
            )
        )
    });
}

OPCODE_HANDLER(add)
{
    binary_operation(bytecode, OP_PLUS);
    return ++ip;
}

OPCODE_HANDLER(sub)
{    
    binary_operation(bytecode, OP_MINUS);
    return ++ip;
}

OPCODE_HANDLER(mul)
{
    binary_operation(bytecode, OP_TIMES);
    return ++ip;
}

OPCODE_HANDLER(div)
{
    binary_operation(bytecode, OP_DIVIDE);
    return ++ip;
}

OPCODE_HANDLER(mod)
{
    binary_operation(bytecode, OP_MOD);
    return ++ip;
}

OPCODE_HANDLER(decl_var)
{
    ip++;
    char *identifier = bytecode_get_next_string(&ip);
    scope_declare_identifier(&global_scope, identifier, & BLAZE_NULL, false);
    return ++ip;
}

OPCODE_HANDLER(store_varval)
{
    ip++;
    char *identifier = bytecode_get_next_string(&ip);
    runtime_val_t value = stack_pop(&global);
    scope_assign_identifier(&global_scope, identifier, &value);
    return ++ip;
}

OPCODE_HANDLER(push_varval)
{
    ip++;
    char *identifier = bytecode_get_next_string(&ip);
    identifier_t *i = scope_resolve_identifier(&global_scope, identifier);

    if (i == NULL)
        bytecode_set_error(bytecode, "'%s' is not defined", identifier);
    else
        stack_push(&global, *i->value);

    return ++ip;
}

static function_t __native_functions[] = {
    { "println", NATIVE_FN_REF(println) },
    { "print", NATIVE_FN_REF(print) },
    { "sleep", NATIVE_FN_REF(sleep) },
    { "pause", NATIVE_FN_REF(pause) },
    { "typeof", NATIVE_FN_REF(typeof) },
    { "read", NATIVE_FN_REF(read) }
};

OPCODE_HANDLER(builtin_fn_call)
{
    uint8_t numargs = *++ip;

    ip++;

    char *symbol = bytecode_get_next_string(&ip);
    vector_t args = VEC_INIT;
    scope_t scope = scope_init(NULL);

    for (uint8_t i = 0; i < numargs; i++)  
    { 
        runtime_val_t element = stack_pop(&global);
        runtime_val_t value = {
            .type = element.type
        };

        if (element.type == VAL_NUMBER)
        {
            if (element.is_float)
                value.floatval = element.floatval;
            else
                value.intval = element.intval;
        }
        else if (element.type == VAL_STRING)
            value.strval = strdup(element.strval);

        VEC_PUSH(args, value, runtime_val_t);
    }

    for (size_t i = 0; i < (sizeof (__native_functions) / sizeof (__native_functions[0])); i++) 
    {
        if (STREQ(__native_functions[i].name, strdup(symbol)))
        {
            __native_functions[i].callback(args, &scope);
            scope_free(&scope);
            return ++ip;
        }
    }

    scope_free(&scope);
    bytecode_set_error(bytecode, "Unknown built in function '%s'", symbol);
    
    return ++ip;
}

static uint8_t *str_forward(uint8_t *ip)
{
    while (*ip != 0)
        ip++;

    return ++ip;
}

OPCODE_HANDLER(push_str)
{
    ip++;
                    
    stack_push(&global, (runtime_val_t) {
        .type = VAL_STRING,
        .strval = bytecode_get_next_string(&ip)
    });

    return ++ip;
}

OPCODE_HANDLER(pop_str)
{
    return opcode_handler_pop(ip, bytecode);
}

OPCODE_HANDLER(print)
{
    vector_t args = VEC_INIT; 
    runtime_val_t value = stack_pop(&global);
    VEC_PUSH(args, value, runtime_val_t);
    NATIVE_FN_REF(println)(args, &global_scope);    
    return ++ip;
}

static void opcode_set_handlers() 
{
    handlers[OP_HLT] = OPCODE_HANDLER_REF(hlt);
    handlers[OP_TEST] = OPCODE_HANDLER_REF(test);
    handlers[OP_PUSH] = OPCODE_HANDLER_REF(push);
    handlers[OP_ADD] = OPCODE_HANDLER_REF(add);
    handlers[OP_SUB] = OPCODE_HANDLER_REF(sub);
    handlers[OP_MUL] = OPCODE_HANDLER_REF(mul);
    handlers[OP_DIV] = OPCODE_HANDLER_REF(div);
    handlers[OP_MODULUS] = OPCODE_HANDLER_REF(mod);
    handlers[OP_DUMP] = OPCODE_HANDLER_REF(dump);
    handlers[OP_POP] = OPCODE_HANDLER_REF(pop);
    handlers[OP_PUSH_STR] = OPCODE_HANDLER_REF(push_str);
    handlers[OP_POP_STR] = OPCODE_HANDLER_REF(pop_str);
    handlers[OP_BUILTIN_FN_CALL] = OPCODE_HANDLER_REF(builtin_fn_call);
    handlers[OP_DECL_VAR] = OPCODE_HANDLER_REF(decl_var);
    handlers[OP_STORE_VARVAL] = OPCODE_HANDLER_REF(store_varval);
    handlers[OP_PUSH_VARVAL] = OPCODE_HANDLER_REF(push_varval);
    handlers[OP_PRINT] = OPCODE_HANDLER_REF(print);
}

void opcode_init()
{
    global = stack_create(20);
    global_scope = scope_init(NULL);

    for (size_t i = 0; i < OPCODE_COUNT; i++)
        handlers[i] = &opcode_handler_nop;

    opcode_set_handlers();
}
