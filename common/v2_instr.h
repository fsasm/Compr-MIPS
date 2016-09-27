/**
 * @file v2_instr.h
 * @author Fabjan Sukalia <fsukalia@gmail.com>
 * @date 2016-09-26
 */

#include "../common/instr.h"

#ifndef V2_INSTR_H
#define V2_INSTR_H

int parse_instr_v2(uint32_t instr, struct instr *out);
int write_instr_v2(struct instr *instr, uint32_t *out); /* return value is the size (2 or 4) */

#endif

