#ifndef UI_H
#define UI_H

#include "data.h"

void printMainMenu(void);
int  submenuTable(void);
int  getValidInt(void);
void getValidString(char* buffer, int max_length);

void showTasksSorted(const Task* arr, const IndexArray* idx);
void showEmployeesSorted(const Employee* arr, const IndexArray* idx);
void showEmployees(const Employee* arr, int count);
void showTasks(const Task* arr, int count);

#endif
