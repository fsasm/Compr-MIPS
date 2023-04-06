/**
 * @file fix16.h
 * @author Fabjan Sukalia <fsukalia@gmail.com>
 * @date 2016-08-05
 * Fixed point type with signed 16 bit integer part and 16 bit fractional part.
 * Common operations like addition, multiplication and division are supported
 * and also parsing and printing of fixed point numbers. 
 */

#include <stdint.h>

typedef int32_t fix16_t;

#define FIX_CONST_1 (1 << 16)
#define FIX_CONST_4 (4 << 16)
#define FIX_INT_MAX INT16_MAX
#define FIX_INT_MIN INT16_MIN
#define FIX_MAX INT32_MAX
#define FIX_MIN INT32_MIN

fix16_t fix_add(fix16_t left, fix16_t right);
fix16_t fix_sub(fix16_t left, fix16_t right);
fix16_t fix_mul(fix16_t left, fix16_t right);
fix16_t fix_div(fix16_t left, fix16_t right);
fix16_t fix_parse(void);
void    fix_print(fix16_t f);

