# Makefile for analyzer
# Author: Fabjan Sukalia <fsukalia@gmail.com>
# Date: 2016-07-12

CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -g -Og 

.PHONY: all clean

all: analyzer

clean:
	rm -f analyzer

analyzer: analyzer.c ../common/instr.c 
	$(CC) $(CFLAGS) -o $@ $^

