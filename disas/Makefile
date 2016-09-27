# Makefile for disas
# Author: Fabjan Sukalia <fsukalia@gmail.com>
# Date: 2016-09-25

CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -g -fsanitize=address -D_XOPEN_SOURCE=500 -D_DEFAULT_SOURCE

.PHONY: all clean

all: disas

clean:
	rm -f disas

disas: disas.c ../common/instr.c ../common/print_instr.c ../common/v2_instr.c
	$(CC) $(CFLAGS) -o $@ $^

