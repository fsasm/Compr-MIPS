# Makefile for analyzer
# Author: Fabjan Sukalia <fsukalia@gmail.com>
# Date: 2016-07-12

CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -g -Og -D_XOPEN_SOURCE=500 -D_DEFAULT_SOURCE

.PHONY: all clean

all: analyzer

clean:
	rm -f analyzer

analyzer: analyzer.c ../common/instr.c ../common/v2_instr.c imm_list.c
	$(CC) $(CFLAGS) -o $@ $^

