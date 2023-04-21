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

#define OPCODE_HANDLER(name) static uint8_t *opcode_handler_##name(uint8_t *ip, bytecode_t *bytecode)
#define OPCODE_HANDLER_REF(name) opcode_handler_##name

static uint8_t stack[4096];
static size_t si = 0;

static opcode_handler_t handlers[OPCODE_COUNT];

OPCODE_HANDLER(nop)
{
    return ++ip;
}

opcode_handler_t opcode_get_handler(opcode_t opcode)
{
    if (opcode >= OPCODE_COUNT || opcode < 0x00)
        return NULL;
    
    return handlers[opcode];
}

OPCODE_HANDLER(dump)
{
    for (ssize_t i = sizeof (stack); i >= 0; i--)
        printf("[%lu]: %u%s\n", i, stack[i], i == si ? "  <=" : "");

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
    return ++ip;
}

OPCODE_HANDLER(pop)
{
    si--;
    return ++ip;
}

static uint8_t *stack_pop()
{
    if (si == 0)
        return NULL;

    si--;
    return (stack + si);
}

static void stack_push(uint8_t el)
{
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

OPCODE_HANDLER(builtin_fn_call)
{
    ip++;

    char *identifer_name = xmalloc(IDENTIFIER_MAX + 1);
    size_t i;

    for (size_t i = 0; *ip != '\0'; i++, ip++)
        identifer_name[i] = *ip;

    identifer_name[i] = '\0';

    uint8_t arglen = *++ip;
    vector_t args = VEC_INIT;

    for (uint8_t i = 0; i < arglen; i++)
    {
        VEC_PUSH(args, *++ip, uint8_t);
    }

    if (STREQ(identifer_name, "println") == 0)
    {
        for (uint8_t i = 0; i < arglen; i++)
        {
            printf(COLOR("1;33", "%u\n"), VEC_GET(args, i, uint8_t));
        }
    }

    free(identifer_name);
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
    handlers[OP_BUILTIN_FN_CALL] = OPCODE_HANDLER_REF(builtin_fn_call);
}

void opcode_init()
{
    for (size_t i = 0; i < OPCODE_COUNT; i++)
        handlers[i] = &opcode_handler_nop;

    opcode_set_handlers();
}
