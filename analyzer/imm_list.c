/**
 * @file imm_list.h
 * @author Fabjan Sukalia <fsukalia@gmail.com>
 * @date 2016-08-12
 */

#include "imm_list.h"

void imm_list_init(struct imm_list *list)
{
	if (list == NULL)
		return;

	list->num_entries = 0;
	list->entries = NULL;
}

void imm_list_destroy(struct imm_list *list)
{
	if (list == NULL)
		return;

	free(list->entries);
	list->entries = NULL;
}

void imm_list_add_signed(struct imm_list *list, int16_t simm)
{
	if (list == NULL)
		return;

	if (list->num_entries == 0) {
		list->num_entries = 1;
		struct imm_entry *entry = malloc(sizeof(struct imm_entry));
		entry->num = 1;
		entry->simm = simm;
		list->entries = entry;
		return;
	}

	struct imm_entry *entries = list->entries;
	for (size_t i = 0; i < list->num_entries; i++) {
		if (entries[i].simm == simm) {
			entries[i].num++;
			return;
		}
	}

	list->num_entries++;
	size_t index = list->num_entries;
	list->entries = realloc(list->entries, sizeof(struct imm_entry) * index);
	list->entries[index - 1] = (struct imm_entry) {.num = 1, .simm = simm};
}

void imm_list_add_unsigned(struct imm_list *list, uint16_t imm)
{
	if (list == NULL)
		return;

	if (list->num_entries == 0) {
		list->num_entries = 1;
		struct imm_entry *entry = malloc(sizeof(struct imm_entry));
		entry->num = 1;
		entry->imm = imm;
		list->entries = entry;
		return;
	}

	struct imm_entry *entries = list->entries;
	for (size_t i = 0; i < list->num_entries; i++) {
		if (entries[i].imm == imm) {
			entries[i].num++;
			return;
		}
	}

	list->num_entries++;
	size_t index = list->num_entries;
	list->entries = realloc(list->entries, sizeof(struct imm_entry) * index);
	list->entries[index - 1] = (struct imm_entry) {.num = 1, .imm = imm};
}

static int cmp_signed(const void *left, const void *right)
{
	if ((((struct imm_entry *)left)->simm) < (((struct imm_entry *)right)->simm))
		return -1;
	return 1;
}

static int cmp_unsigned(const void *left, const void *right)
{
	if ((((struct imm_entry *)left)->imm) < (((struct imm_entry *)right)->imm))
		return -1;
	return 1;
}

void imm_list_sort_signed(struct imm_list *list)
{
	if (list == NULL)
		return;

	if (list->num_entries == 0)
		return;

	qsort(list->entries, list->num_entries, sizeof(struct imm_entry), cmp_signed);
}

void imm_list_sort_unsigned(struct imm_list *list)
{
	if (list == NULL)
		return;

	if (list->num_entries == 0)
		return;

	qsort(list->entries, list->num_entries, sizeof(struct imm_entry), cmp_unsigned);
}

