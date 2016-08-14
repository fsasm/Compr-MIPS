# Makefile for uart_escape
# Author: Fabjan Sukalia <fsukalia@gmail.com>
# Date: 2016-07-12

CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -O2 -D_XOPEN_SOURCE=500 -D_DEFAULT_SOURCE

.PHONY: all clean

all: uart_escape

clean:
	rm -f uart_escape

uart_escape: uart_escape.c
	$(CC) $(CFLAGS) -o $@ $^

