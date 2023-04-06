#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define CHAR_BUFFER_SIZE (1)

static uint8_t buffer_size = 0;
static char buffer[CHAR_BUFFER_SIZE];

#define ESCAPE_BYTE (0x00)

#define UART_WRITE_READY (0x01)
#define UART_READ_READY (0x02)

static volatile uint8_t *uart_status = (volatile uint8_t *)(-8);
static volatile uint8_t *uart_data = (volatile uint8_t *)(-4);

static uint8_t read_uart()
{
	while(((*uart_status) & UART_READ_READY) == 0) {
		/* wait until the uart receives data */
	}
	return *uart_data;
}

static void write_uart(uint8_t data)
{
	while(((*uart_status) & UART_WRITE_READY) == 0) {
		/* wait until the uart send buffer isn't full */
	}
	*uart_data = data;
}

/* To simulate EOF a special sequence must be send to the hardware. The first
 * byte in the sequence is the escape byte (0x00). For a EOF the following byte
 * must any number that is not 0x00. If the following byte is 0x00 then the 
 * sequence is recognized as the byte 0x00. 
 * Data in text form don't need escaping when send to the hardware. Binary data
 * must be escaped prior sending. */
int getchar()
{
	if (buffer_size > 0) {
		buffer_size--;
		return buffer[buffer_size];
	}
	
	uint8_t byte = read_uart();
	if (byte != ESCAPE_BYTE)
		return byte;

	byte = read_uart();
	if (byte == 0x00)
		return 0x00;

	return EOF;
}

int putchar(int c) {
	write_uart(c);

	return c;
}

int putstring(const char *s) {
	while (*s != '\0') {
		putchar(*s++);
	}
	return 0;
}

int puts(const char *s) {
	putstring(s);
	putchar('\n');
	return 0;
}

int ungetchar(int c)
{
	if (buffer_size >= CHAR_BUFFER_SIZE) {
		return EOF; /* EOF */
	}

	buffer[buffer_size] = (char)c;
	buffer_size++;

	return c;
}
