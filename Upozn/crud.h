#ifndef CRUD_H
#define CRUD_H

#include "data.h"

void addEmployee(Employee** arr, int* count, int* capacity);
void addTask(Task** tArr, int* tCount, int* tCap, const Employee* eArr, int eCount);

void editEmployee(Employee* arr, int count);
void editTask(Task* tArr, int tCount, const Employee* eArr, int eCount);

bool hasDependentTasks(int emp_id, const Task* tArr, int tCount);

void deleteEmployee(Employee* eArr, int eCount, const Task* tArr, int tCount);
void deleteTask(Task* arr, int count);

#endif
