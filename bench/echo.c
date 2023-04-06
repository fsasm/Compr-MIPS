#include <stdio.h>

void main(void)
{
	puts("Echo demo\n");
	int c = EOF;
	do {
		c = getchar();
		if (c == '\n')
			putchar('\r');
		putchar(c);
	} while(c != EOF);
}

