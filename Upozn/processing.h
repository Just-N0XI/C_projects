#ifndef PROCESSING_H
#define PROCESSING_H

#include "data.h"

/* --- Глобальные индексные массивы --- */

extern IndexArray idxEmpFIO;
extern IndexArray idxEmpPosition;
extern IndexArray idxTaskProject;
extern IndexArray idxTaskDeadline;

/* --- Инициализация / очистка --- */

void initIndexArrays(const Employee* eArr, int eCount, const Task* tArr, int tCount);
void freeIndexArrays(void);

/* --- Вставка в индекс при добавлении записи --- */

void insertIntoIndex(IndexArray* idx, int mainIdx, const char* key);

/* --- Пометка удалённой записи в индексах --- */

void markDeletedInEmpIndexes(int mainIdx);
void markDeletedInTaskIndexes(int mainIdx);

/* --- Бинарный поиск (возвращает массив совпадений) --- 
Результат — динамический массив mainIdx; освобождается через free() */

int* binarySearchIndex(const IndexArray* idx, const char* key, int* outCount);

/* --- Поиск индекса в основном массиве по ID --- 
Основной массив отсортирован по ID — используем бинарный поиск */

int getEmployeeIndexByID(const Employee* arr, int count, int emp_id);
int getTaskIndexByID(const Task* arr, int count, int task_id);

/* --- Вывод результатов поиска --- */

void findEmployeeByFIO(const Employee* eArr, const char* key);
void findEmployeeByPosition(const Employee* eArr, const char* key);
void findTaskByProject(const Task* tArr, const char* key);
void findTaskByDeadline(const Task* tArr, const char* key);

#endif
