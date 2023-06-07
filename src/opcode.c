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

runtime_val_t registers[REG_COUNT];

static bool is_valid_regid(bytecode_t *bytecode, int regid)
{
    if (regid < 0 || regid >= REG_COUNT)
    {
        bytecode_set_error(bytecode, "regid operand is invalid: %u", regid);
        return false;
    }

    return true;
}

#define REG2_OR_NUM_VAL(reg2_is_reg, reg2id, number) (reg2_is_reg == 0x01 ? registers[reg2id].intval : number)

static uint8_t *binary_operation_reg(bytecode_t *bytecode, uint8_t *ip, char operator)
{
    uint8_t reg2_is_reg = *++ip,
            reg1id = *++ip, 
            reg2id = *(ip + 1);

    int number;

    if (!is_valid_regid(bytecode, reg1id))
        return ++ip;

    if (reg2_is_reg && !is_valid_regid(bytecode, reg2id))
        return ++ip;

    if (reg2_is_reg)
        ip++;
    else 
    {
        uint8_t byte1 = *++ip,   
                byte2 = *++ip,
                byte3 = *++ip,
                byte4 = *++ip;
        
        number = byte1 | byte2 << 8 | byte3 << 16 | byte4 << 32;
    }

    if (registers[reg1id].type != VAL_NUMBER || (reg2_is_reg && registers[reg2id].type != VAL_NUMBER)) 
    {
        bytecode_set_error(bytecode, "operands must be number");
        return ++ip;
    }

    if ((operator == '%' || operator == '/') && ((reg2_is_reg && registers[reg2id].intval == 0) || (!reg2_is_reg && number == 0)))
    {
        bytecode_set_error(bytecode, "operand #2 must be non-zero number");
        return ++ip;
    }

    int value = operator == '+' ? registers[reg1id].intval + REG2_OR_NUM_VAL(reg2_is_reg, reg2id, number) : (
        operator == '-' ? registers[reg1id].intval - REG2_OR_NUM_VAL(reg2_is_reg, reg2id, number) : (
            operator == '*' ? registers[reg1id].intval * REG2_OR_NUM_VAL(reg2_is_reg, reg2id, number) : (
                operator == '/' ? registers[reg1id].intval / REG2_OR_NUM_VAL(reg2_is_reg, reg2id, number) : (
                    operator == '%' ? registers[reg1id].intval % REG2_OR_NUM_VAL(reg2_is_reg, reg2id, number) : (
                        operator == '|' ? registers[reg1id].intval | REG2_OR_NUM_VAL(reg2_is_reg, reg2id, number) : (
                            operator == '&' ? registers[reg1id].intval & REG2_OR_NUM_VAL(reg2_is_reg, reg2id, number) : (
                                operator == '^' ? registers[reg1id].intval ^ REG2_OR_NUM_VAL(reg2_is_reg, reg2id, number) : (
                                    -1
                                )
                            )
                        )
                    )
                )
            )
        )
    );

    registers[reg1id] = (runtime_val_t) {
        .type = VAL_NUMBER,
        .is_float = false,
        .intval = value
    };
    
    return ++ip;
}

OPCODE_HANDLER(regadd)
{
    return binary_operation_reg(bytecode, ip, '+');
}

OPCODE_HANDLER(regsub)
{
    return binary_operation_reg(bytecode, ip, '-');
}

OPCODE_HANDLER(regmul)
{
    return binary_operation_reg(bytecode, ip, '*');
}

OPCODE_HANDLER(regdiv)
{
    return binary_operation_reg(bytecode, ip, '/');
}

OPCODE_HANDLER(regmod)
{
    return binary_operation_reg(bytecode, ip, '%');
}

OPCODE_HANDLER(regor)
{
    return binary_operation_reg(bytecode, ip, '|');
}

OPCODE_HANDLER(regand)
{
    return binary_operation_reg(bytecode, ip, '&');
}

OPCODE_HANDLER(regxor)
{
    return binary_operation_reg(bytecode, ip, '^');
}

OPCODE_HANDLER(mov)
{
    uint8_t regid = *++ip;
    uint8_t byte1 = *++ip,   
            byte2 = *++ip,
            byte3 = *++ip,
            byte4 = *++ip;

    int number = byte1 | byte2 << 8 | byte3 << 16 | byte4 << 32;
    
    if (!is_valid_regid(bytecode, regid))
        return ++ip;
    
    registers[regid] = (runtime_val_t) {
        .type = VAL_NUMBER,
        .is_float = false,
        .intval = number
    };
    
    return ++ip;
}

OPCODE_HANDLER(regdump)
{
    puts("* REGDUMP");

    for (int i = 0; i < REG_COUNT; i++)
    {
        printf("%%r%d: ", i);
        print_rtval(&registers[i], true, 0, true);
    }

    return ++ip;
}

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
    handlers[OP_MOV] = OPCODE_HANDLER_REF(mov);
    handlers[OP_REGDUMP] = OPCODE_HANDLER_REF(regdump);
    handlers[OP_REGADD] = OPCODE_HANDLER_REF(regadd);
    handlers[OP_REGSUB] = OPCODE_HANDLER_REF(regsub);
    handlers[OP_REGMUL] = OPCODE_HANDLER_REF(regmul);
    handlers[OP_REGDIV] = OPCODE_HANDLER_REF(regdiv);
    handlers[OP_REGMOD] = OPCODE_HANDLER_REF(regmod);
    handlers[OP_REGOR] = OPCODE_HANDLER_REF(regor);
    handlers[OP_REGAND] = OPCODE_HANDLER_REF(regand);
    handlers[OP_REGXOR] = OPCODE_HANDLER_REF(regxor);
}

void opcode_init()
{
    global = stack_create(20);
    global_scope = scope_init(NULL);

    for (size_t i = 0; i < OPCODE_COUNT; i++)
        handlers[i] = &opcode_handler_nop;
    
    opcode_set_handlers();
    
    for (size_t i = 0; i < REG_COUNT; i++)
        registers[i] = BLAZE_NULL;
}
