# Makefile for converter
# Author: Fabjan Sukalia <fsukalia@gmail.com>
# Date: 2016-09-25

CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -g -fsanitize=address

.PHONY: all clean

all: converter

clean:
	rm -f converter

converter: converter.c ../common/instr.c ../common/print_instr.c ../common/v2_instr.c
	$(CC) $(CFLAGS) -o $@ $^

