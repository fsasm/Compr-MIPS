/**
 * @file mandelbrot.c
 * @author Fabjan Sukalia <fsukalia@gmail.com>
 * @date 2016-07-14
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "mul.h"
#include "fix16.h"

/* Mandelbrot fractal */
#define WIDTH (128)
#define HEIGHT (64)
#define MAX_ITER (512)

static const char output_map[16] = {
	'.', '_', ':', '|', '0', '%',  '&', '$', 
	'@', '+', '"', '^', '/', '\\', '~', '?'
};

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	const fix16_t left = -2 * FIX_CONST_1; // -2.0
	const fix16_t right = FIX_CONST_1;
	const fix16_t top = FIX_CONST_1;
	const fix16_t bot = -FIX_CONST_1;

	const fix16_t dx = fix_sub(right, left) / WIDTH;
	const fix16_t dy = fix_sub(top, bot) / HEIGHT;

	fix16_t cy = bot;
	for (uint_fast16_t y = 0; y < HEIGHT; y++) {
		fix16_t cx = left;
		for (uint_fast16_t x = 0; x < WIDTH; x++) {
			fix16_t zx = cx;
			fix16_t zy = cy;
			
			uint_fast16_t iter = 0;
			for (; iter < MAX_ITER; iter++) {
				fix16_t zx2 = fix_mul(zx, zx);
				fix16_t zy2 = fix_mul(zy, zy);

				if (fix_add(zx2, zy2) >= FIX_CONST_4)
					break;
				
				zy = fix_mul(zx, zy);
				zy = fix_add(zy, zy);

				zx = fix_sub(zx2, zy2);

				zx = fix_add(zx, cx);
				zy = fix_add(zy, cy);
			}

			if (iter == 0 || iter >= MAX_ITER)
				putchar(' ');
			else
				putchar(output_map[iter % 16]);

			cx = fix_add(cx, dx);
		}
		putchar('\n');
		cy = fix_add(cy, dy);
	}
	return 0;
}

