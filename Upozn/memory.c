#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"
#include "processing.h"

#define INIT_CAPACITY 10

int Global_Max_Emp_ID = 0;
int Global_Max_Task_ID = 0;

void expandArray(void** arr, int* capacity, size_t elementSize) {
	int newCap = (*capacity == 0) ? 4 : (*capacity) * 2;
	int* temp = realloc(*arr, (size_t)newCap * elementSize);
	if (temp == NULL) {
		fprintf(stderr, "Ошибка: не удалось выделить память.\n");
		return;
	}

	*arr = temp;
	*capacity = newCap;
}

void loadFiles(Employee** eArr, int* eCount, int* eCap, Task** tArr, int* tCount, int* tCap, bool* isLoaded) {
	if (*isLoaded) {
		printf("Данные уже загружены в память.\n");
		return;
	}

	*eCap = INIT_CAPACITY;
	*eCount = 0;
	*eArr = (Employee*)malloc((size_t)*eCap * sizeof(Employee));

	if (!*eArr) {
		fprintf(stderr, "Ошибка выделения памяти.\n");
		return;
	}

	*tCap = INIT_CAPACITY;
	*tCount = 0;
	*tArr = (Task*)malloc((size_t)*tCap * sizeof(Task));

	if (!*tArr) {
		fprintf(stderr, "Ошибка выделения памяти.\n");
		free(*eArr);
		return;
	}

	bool anyMissing = false;

	FILE* fe = fopen("employees.bin", "rb");
	if (!fe)
		anyMissing = true;
	else {
		Employee temp;
		while (fread(&temp, sizeof(Employee), 1, fe) == 1) {
			if (*eCount == *eCap)
				expandArray((void**)eArr, eCap, sizeof(Employee));
			(*eArr)[*eCount] = temp;
			(*eCount)++;
			if (!temp.is_deleted && temp.emp_id > Global_Max_Emp_ID)
				Global_Max_Emp_ID = temp.emp_id;
		}
		fclose(fe);
	}

	FILE* (ft) = fopen("tasks.bin", "rb");
	if (!ft)
		anyMissing = true;
	else {
		Task temp;
		while (fread(&temp, sizeof(Task), 1, ft) == 1) {
			if (*tCount == *tCap)
				expandArray((void**)tArr, tCap, sizeof(Task));
			(*tArr)[*tCount] = temp;
			(*tCount)++;
			if (!temp.is_deleted && temp.task_id > Global_Max_Task_ID)
				Global_Max_Task_ID = temp.task_id;
		}
		fclose(ft);
	}

	if (anyMissing)
		printf("Файлы данных не найдены. Созданы пустые списки.\n");
	else
		printf("Данные успешно загружены.\n");

	initIndexArrays(*eArr, *eCount, *tArr, *tCount);

	*isLoaded = true;
}

void freeMemory(Employee** eArr, Task** tArr) {
	if (eArr && *eArr) {
		free(*eArr);
		*eArr = NULL;
	}
	if (tArr && *tArr) {
		free(*tArr);
		*tArr = NULL;
	}
	freeIndexArrays();
}