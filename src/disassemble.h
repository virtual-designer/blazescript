/*
 * Created by rakinar2 on 9/1/23.
 */

#ifndef BLAZESCRIPT_DISASSEMBLE_H
#define BLAZESCRIPT_DISASSEMBLE_H

#include <stdio.h>
#include "bytecode.h"

void disassemble(FILE *__restrict__ fp, struct bytecode *bytecode);

#endif /* BLAZESCRIPT_DISASSEMBLE_H */
