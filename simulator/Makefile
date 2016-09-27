# Makefile for simulator
# Author: Fabjan Sukalia <fsukalia@gmail.com>
# Date: 2016-07-12

CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -O2 -D_XOPEN_SOURCE=500 -D_DEFAULT_SOURCE

.PHONY: all clean

all: simulator

clean:
	rm -f simulator

simulator: simulator.c ../common/instr.c ../common/v2_instr.c ../common/print_instr.c
	$(CC) $(CFLAGS) -o $@ $^

