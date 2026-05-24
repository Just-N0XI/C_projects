#define _CRT_SECURE_NO_WARNINGS
#include"processing.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

IndexArray idxEmpFIO;
IndexArray idxEmpPosition;
IndexArray idxTaskProject;
IndexArray idxTaskDeadline;

#define IDX_INIT_CAP 16
#define DELETED_KEY  "\x01"

static void initIdx(IndexArray* idx) {
	idx->capacity = IDX_INIT_CAP;
	idx->count = 0;
	idx->entries = (IndexEntry*)calloc((size_t)idx->capacity, sizeof(IndexEntry));
	if (!idx->entries)  
		fprintf(stderr, "Ошибка выделения памяти (индекс).\n");	
}

static void freeIdx(IndexArray* idx)
{
	free(idx->entries);
	idx->entries = NULL;
	idx->count = 0;
	idx->capacity = 0;
}

void insertIntoIndex(IndexArray* idx, int mainIdx, const char* key)
{
	if (idx->count == idx->capacity) {
		int newCap = idx->capacity * 2;
		IndexEntry* nextEntries = (IndexEntry*)realloc(idx->entries, (size_t)newCap * sizeof(IndexEntry));
		if (!nextEntries) {
			fprintf(stderr, "Ошибка realloc (индекс).\n");
			return;
		}

		idx->entries = nextEntries;
		
		memset(&(idx->entries[idx->capacity]), 0, (size_t)(newCap - idx->capacity) * sizeof(IndexEntry));
		idx->capacity = newCap;
	}

	int l = 0, r = idx->count;
	while (l < r) {
		int mid = (l + r) / 2;
		if (strcmp(idx->entries[mid].key, key) <= 0)
			l = mid + 1;
		else
			r = mid;
	}

	/* сдвиг вправо */
	for (int i = idx->count; i > l; i--)
		idx->entries[i] = idx->entries[i - 1];

	/* вставка */
	idx->entries[l].mainIdx = mainIdx;
	strncpy(idx->entries[l].key, key, MAX_KEY - 1);
	idx->entries[l].key[MAX_KEY - 1] = '\0';
	idx->count++;
}

static void deadlineToSortKey(const char* dl, char* out, int outSize)
{
	int d = 0, m = 0, y = 0;
	if (sscanf(dl, "%d.%d.%d", &d, &m, &y) != 3) 
		d = 0; m = 0; y = 0;
	
	snprintf(out, (size_t)outSize, "%04d%02d%02d", y, m, d);
}

void initIndexArrays(const Employee* eArr, int eCount, const Task* tArr, int tCount) {
	initIdx(&idxEmpFIO);
	initIdx(&idxEmpPosition);
	initIdx(&idxTaskProject);
	initIdx(&idxTaskDeadline);

	for (int i = 0; i < eCount; i++) {
		if (eArr[i].is_deleted) continue;
		insertIntoIndex(&idxEmpFIO, i, eArr[i].fio);
		insertIntoIndex(&idxEmpPosition, i, eArr[i].position);
	}

	for (int i = 0; i < tCount; i++) {
		if (tArr[i].is_deleted) continue;
		char sortKey[MAX_KEY];
		deadlineToSortKey(tArr[i].deadline, sortKey, MAX_KEY);
		insertIntoIndex(&idxTaskProject, i, tArr[i].project_name);
		insertIntoIndex(&idxTaskDeadline, i, sortKey);
	}
}

void freeIndexArrays(void) {
	freeIdx(&idxEmpFIO);
	freeIdx(&idxEmpPosition);
	freeIdx(&idxTaskProject);
	freeIdx(&idxTaskDeadline);
}

/*
markDeleted — заменяем key на \x01
\x01 < любого печатного символа, поэтому такие
записи уходят в самое начало индекса и никогда
не попадают в результаты поиска по нормальным ключам
*/
void markDeletedInEmpIndexes(int mainIdx)
{
	for (int i = 0; i < idxEmpFIO.count; i++)
		if (idxEmpFIO.entries[i].mainIdx == mainIdx)
			strncpy(idxEmpFIO.entries[i].key, DELETED_KEY, MAX_KEY - 1);

	for (int i = 0; i < idxEmpPosition.count; i++)
		if (idxEmpPosition.entries[i].mainIdx == mainIdx)
			strncpy(idxEmpPosition.entries[i].key, DELETED_KEY, MAX_KEY - 1);
}

void markDeletedInTaskIndexes(int mainIdx)
{
	for (int i = 0; i < idxTaskProject.count; i++)
		if (idxTaskProject.entries[i].mainIdx == mainIdx)
			strncpy(idxTaskProject.entries[i].key, DELETED_KEY, MAX_KEY - 1);

	for (int i = 0; i < idxTaskDeadline.count; i++)
		if (idxTaskDeadline.entries[i].mainIdx == mainIdx)
			strncpy(idxTaskDeadline.entries[i].key, DELETED_KEY, MAX_KEY - 1);
}

int* binarySearchIndex(const IndexArray* idx, const char* key, int* outCount)
{
	*outCount = 0;
	if (!idx->entries || idx->count == 0) return NULL;

	int cap = 8;
	int* result = (int*)calloc((size_t)cap, sizeof(int));
	if (!result) 
		return NULL;

	/* бинарный поиск левой границы */
	int l = 0, r = idx->count;
	while (l < r) {
		int mid = (l + r) / 2;
		if (strcmp(idx->entries[mid].key, key) < 0)
			l = mid + 1;
		else
			r = mid;
	}

	/* влево от границы */
	for (int i = l - 1; i >= 0; i--) {
		if (strcmp(idx->entries[i].key, DELETED_KEY) == 0) 
			continue;

		if (!strstr(idx->entries[i].key, key)) 
			break;

		if (*outCount == cap) {
			cap *= 2;
			int* temp = (int*)realloc(result, (size_t)cap * sizeof(int));

			if (!temp) { 
				free(result); 
				return NULL; 
			}

			memset(temp + *outCount, 0, (size_t)(cap / 2) * sizeof(int));
			result = temp;
		}

		result[(*outCount)++] = idx->entries[i].mainIdx;
	}

	/* вправо от границы */
	for (int i = l; i < idx->count; i++) {
		if (strcmp(idx->entries[i].key, DELETED_KEY) == 0) 
			continue;

		if (!strstr(idx->entries[i].key, key)) 
			continue;

		if (*outCount == cap) {
			cap *= 2;
			int* temp = (int*)realloc(result, (size_t)cap * sizeof(int));
			if (!temp) {
				free(result); 
				return NULL; 
			}

			memset(temp + *outCount, 0, (size_t)(cap / 2) * sizeof(int));
			result = temp;
		}

		result[(*outCount)++] = idx->entries[i].mainIdx;
	}

	if (*outCount == 0) { 
		free(result); 
		return NULL; 
	}

	return result;
}

int getEmployeeIndexByID(const Employee* arr, int count, int emp_id)
{
	int l = 0, r = count - 1;
	while (l <= r) {
		int mid = (l + r) / 2;
		if (arr[mid].emp_id == emp_id) {
			if (arr[mid].is_deleted) return -1;
			return mid;
		}

		if (arr[mid].emp_id < emp_id) 
			l = mid + 1;
		else                          
			r = mid - 1;
	}

	return -1;
}

int getTaskIndexByID(const Task* arr, int count, int task_id)
{
	int l = 0, r = count - 1;
	while (l <= r) {
		int mid = (l + r) / 2;
		if (arr[mid].task_id == task_id) {
			if (arr[mid].is_deleted) return -1;
			return mid;
		}
		
		if (arr[mid].task_id < task_id) 
			l = mid + 1;
		else                            
			r = mid - 1;
	}

	return -1;
}

void findEmployeeByFIO(const Employee* eArr, const char* key)
{
	int cnt = 0;
	int* res = binarySearchIndex(&idxEmpFIO, key, &cnt);
	if (!res) { 
		printf("Ничего не найдено.\n"); 
		return; 
	}

	printf("%-6s %-30s %-20s %-6s %-6s\n", "ID", "ФИО", "Должность", "Часы", "Рук.");
	
	for (int i = 0; i < cnt; i++) {
		const Employee* e = &eArr[res[i]];
		printf("%-6d %-30s %-20s %-6d %-6d\n",
			e->emp_id, 
			e->fio, 
			e->position, 
			e->work_hours, 
			e->manager_id);
	}

	free(res);
}

void findEmployeeByPosition(const Employee* eArr, const char* key)
{
	int cnt = 0;
	int* res = binarySearchIndex(&idxEmpPosition, key, &cnt);
	if (!res) { 
		printf("Ничего не найдено.\n"); 
		return; 
	}

	printf("%-6s %-30s %-20s %-6s %-6s\n", "ID", "ФИО", "Должность", "Часы", "Рук.");
	
	for (int i = 0; i < cnt; i++) {
		const Employee* e = &eArr[res[i]];
		printf("%-6d %-30s %-20s %-6d %-6d\n",
			e->emp_id, 
			e->fio, 
			e->position, 
			e->work_hours, 
			e->manager_id);
	}

	free(res);
}

void findTaskByProject(const Task* tArr, const char* key)
{
	int cnt = 0;
	int* res = binarySearchIndex(&idxTaskProject, key, &cnt);
	if (!res) { 
		printf("Ничего не найдено.\n"); 
		return; 
	}

	printf("%-6s %-20s %-28s %-6s %-6s %-12s %-12s\n",
		"ID", "Проект", "Описание", "Исп.", "Рук.", "Выдана", "Дедлайн");
	
	for (int i = 0; i < cnt; i++) {
		const Task* t = &tArr[res[i]];
		printf("%-6d %-20s %-28s %-6d %-6d %-12s %-12s\n",
			t->task_id,
			t->project_name, 
			t->task_desc,
			t->executor_id,
			t->manager_id, 
			t->gotTask_date, 
			t->deadline);
	}

	free(res);
}

void findTaskByDeadline(const Task* tArr, const char* key)
{
	char sortKey[MAX_KEY];
	deadlineToSortKey(key, sortKey, MAX_KEY);
	int cnt = 0;
	int* res = binarySearchIndex(&idxTaskDeadline, sortKey, &cnt);
	if (!res) { 
		printf("Ничего не найдено.\n"); 
		return; 
	}

	printf("%-6s %-20s %-28s %-12s\n", "ID", "Проект", "Описание", "Дедлайн");
	
	for (int i = 0; i < cnt; i++) {
		const Task* t = &tArr[res[i]];
		printf("%-6d %-20s %-28s %-12s\n",
			t->task_id, 
			t->project_name, 
			t->task_desc, 
			t->deadline);
	}

	free(res);
}