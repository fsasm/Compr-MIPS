/**
 * @file imm_list.h
 * @author Fabjan Sukalia <fsukalia@gmail.com>
 * @date 2016-08-12
 * A list of immeadiate values with their occurences.
 */

#include <stdint.h>
#include <stdlib.h>

struct imm_entry {
	uint32_t num;
	union {
		int16_t simm;
		uint16_t imm;
	};
};

struct imm_list {
	size_t num_entries;
	struct imm_entry *entries;
};

void imm_list_init(struct imm_list *list);
void imm_list_destroy(struct imm_list *list);

void imm_list_add_signed(struct imm_list *list, int16_t simm);
void imm_list_add_unsigned(struct imm_list *list, uint16_t imm);

void imm_list_sort_signed(struct imm_list *list);
void imm_list_sort_unsigned(struct imm_list *list);

