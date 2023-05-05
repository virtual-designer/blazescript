#include <inttypes.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>

#include "utils.h"
#include "opcode.h"
#include "scope.h"
#include "vector.h"
#include "string.h"
#include "compile.h"
#include "functions.h"

#define OPCODE_HANDLER(name) static uint8_t *opcode_handler_##name(uint8_t *ip, bytecode_t *bytecode)
#define OPCODE_HANDLER_REF(name) opcode_handler_##name

static uint8_t stack[4096];
static size_t si = 0, count = 0;

static opcode_handler_t handlers[OPCODE_COUNT];

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
    for (ssize_t i = sizeof (stack); i >= 0; i--)
        printf("[%lu]: %u%s\n", i, stack[i], i == (ssize_t) si ? "  <=" : "");

    return ++ip;
}

OPCODE_HANDLER(hlt)
{
#ifdef _DEBUG
#ifndef _NODEBUG
    (OPCODE_HANDLER_REF(dump))(ip, bytecode);
#endif
#endif
    exit(errno != 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}

OPCODE_HANDLER(test)
{
    puts("test instruction");
    return ++ip;
}

OPCODE_HANDLER(push)
{
    stack[si++] = *(++ip);
    count++;
    return ++ip;
}

OPCODE_HANDLER(pop)
{
    si--;
    count--;
    return ++ip;
}

static uint8_t *stack_pop()
{
    if (si == 0)
        return NULL;

    si--;
    count--;
    return (stack + si);
}

static void stack_push(uint8_t el)
{
    count++;
    stack[si++] = el;
}

OPCODE_HANDLER(add)
{
    uint8_t *num2 = stack_pop();
    uint8_t *num1 = stack_pop();
    assert(num1 != NULL && num2 != NULL);
    stack_push(*num1 + *num2);
    return ++ip;
}

OPCODE_HANDLER(sub)
{
    uint8_t *num2 = stack_pop();
    uint8_t *num1 = stack_pop();
    assert(num1 != NULL && num2 != NULL);
    stack_push(*num1 - *num2);
    return ++ip;
}

OPCODE_HANDLER(mul)
{
    uint8_t *num2 = stack_pop();
    uint8_t *num1 = stack_pop();
    assert(num1 != NULL && num2 != NULL);
    stack_push(*num1 * *num2);
    return ++ip;
}

OPCODE_HANDLER(div)
{
    uint8_t *num2 = stack_pop();
    uint8_t *num1 = stack_pop();
    assert(num1 != NULL && num2 != NULL);

    if (num2 == 0)
    {
        bytecode_set_error(bytecode, "cannot divide by zero");
        return ++ip;
    }

    stack_push((uint8_t) (*num1 / *num2));
    return ++ip;
}

OPCODE_HANDLER(mod)
{
    uint8_t *num2 = stack_pop();
    uint8_t *num1 = stack_pop();
    assert(num1 != NULL && num2 != NULL);

    if (num2 == 0)
    {
        bytecode_set_error(bytecode, "cannot divide by zero");
        return ++ip;
    }

    stack_push(*num1 % *num2);
    return ++ip;
}

struct builtin_fn_info {
    const char *name;
    NATIVE_FN_TYPE(callback);
    bool is_variadic;
    size_t argument_count;
    runtime_valtype_t argument_types[ARGS_MAX];
};

static struct builtin_fn_info const builtin_fn_list[] = {
    { "println", NATIVE_FN_REF(println), true, 0, { VAL_ANY } }
};

OPCODE_HANDLER(builtin_fn_call)
{
    ip++;

    char *identifier_name = xmalloc(IDENTIFIER_MAX + 1);
    size_t i;

    for (i = 0; *ip != '\0'; i++, ip++)
        identifier_name[i] = (char) *ip;

    ip++;
    identifier_name[i] = '\0';

    for (size_t i2 = 0; i2 < sizeof (builtin_fn_list) / sizeof (builtin_fn_list[0]); i2++)
    {
        if (STREQ(builtin_fn_list[i2].name, identifier_name))
        {
            vector_t args = VEC_INIT, args_rev = VEC_INIT;
            free(identifier_name);
            NATIVE_FN_TYPE(callback) = builtin_fn_list[i2].callback;

            size_t iterations = builtin_fn_list[i2].is_variadic ? *ip : builtin_fn_list[i2].argument_count;

            for (size_t i3 = 0; i3 < iterations; i3++)
            {
                runtime_val_t value = {
                    .type = dt_to_rtval_type(*(ip - i3 + iterations))
                };

                if (value.type == VAL_STRING)
                {
                    string_t str = _str("");
                    size_t len = 0;
                    uint8_t *c = stack_pop();

                    while (c != NULL && *c != 0)
                    {
                        concat_c_safe(str, &len, (char) *c);
                        c = stack_pop();
                    }

                    concat_c_safe(str, &len, 0);
                    value.strval = str;
                }
                else if (value.type == VAL_NUMBER)
                {
                    value.intval = *stack_pop();
                    value.is_float = false;
                }
                else
                {
                    assert(false && "Unsupported type");
                }

                VEC_PUSH(args, value, runtime_val_t);
            }

            for (ssize_t j = (ssize_t) args.length - 1; j >= 0; j--)
            {
                VEC_PUSH(args_rev, VEC_GET(args, j, runtime_val_t), runtime_val_t);
                ip++;
            }

            callback(args_rev, NULL);
            return ip;
        }
    }

    bytecode_set_error(bytecode, "Undefined builtin function '%s'", identifier_name);
    free(identifier_name);

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
    string_t str = _str("");
    size_t len = 0;

    while (*ip != 0)
    {
        concat_c_safe(str, &len, (char) *(ip++));
    }

    stack_push(0);

    for (ssize_t i = len - 1; i >= 0; i--)
        stack_push(str[i]);

    free(str);
    return ++ip;
}

OPCODE_HANDLER(pop_str)
{
    while (stack_pop() != 0);
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
}

void opcode_init()
{
    for (size_t i = 0; i < OPCODE_COUNT; i++)
        handlers[i] = &opcode_handler_nop;

    opcode_set_handlers();
}
