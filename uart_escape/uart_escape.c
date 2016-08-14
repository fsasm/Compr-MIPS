/**
 * @file uart_escape.c
 * @author Fabjan Sukalia <fsukalia@gmail.com>
 * @date 2016-08-14
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define ESCAPE_BYTE (0x00)

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	do {
		int c = getchar();
		if (c == EOF) {
			putchar(ESCAPE_BYTE);
			putchar(0x01);
			break;
		}

		if (c == ESCAPE_BYTE) {
			putchar(ESCAPE_BYTE);
			putchar(ESCAPE_BYTE);
			continue;
		}

		putchar(c);

	} while(true);
	return 0;
}
