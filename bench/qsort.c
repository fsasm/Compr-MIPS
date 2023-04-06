/**
 * @file qsort.c
 * @author Fabjan Sukalia <fsukalia@gmail.com>
 * @date 2016-12-20
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "mul.h"

#define SIZE (100u)

void qsorti(int *base, size_t nmemb)
{
	if (nmemb == 0 || nmemb == 1)
		return;
	
	int pivot = base[nmemb/2];
	int t = base[0];
	base[0] = base[nmemb/2];
	base[nmemb / 2] = t;

	size_t li = 1;
	size_t ri = nmemb;
	
	while (li < ri) {
		if (base[li] <= pivot) {
			li++;
		} else {
			bool cmp;
			do {
				ri--;
				cmp = base[ri] >= pivot;
			} while (li < ri && cmp);
			t = base[li];
			base[li] = base[ri];
			base[ri] = t;
		}
	}
	li--;
	
	t = base[0];
	base[0] = base[li];
	base[li] = t;
	
	qsorti(base, li);
	qsorti(&base[ri], (nmemb - ri));
}
static uint64_t rand_seed;

static int rand(void)
{
	/* similar to the proposal in the C99 standard */
	rand_seed = mulu32(rand_seed, 1103515245u) + 12345u;
	return (rand_seed >> 16) & 0x7FFF;
}

static bool is_sorted_seq(size_t num, int arr[num])
{
	if (num == 0 || num == 1)
		return true;
	
	bool sorted = true;
	for (size_t i = 0; i < (num - 1); i++) {
		sorted &= (arr[i] <= arr[i + 1]);
	}
	
	return sorted;
}

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	int arr[SIZE];
	rand_seed = 0xBADC0FFE;
	puts("Quicksort test");
	
	for (size_t i = 0; i < SIZE; i++) {
		arr[i] = rand();
	}
	
	qsorti(arr, SIZE);
	
	if (is_sorted_seq(SIZE, arr)) {
		puts("success");
	} else {
		puts("fail");
	}
	return 0;
}
