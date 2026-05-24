#ifndef MEMORY_H
#define MEMORY_H

#include "data.h"

void expandArray(void** arr, int* capacity, size_t elementSize);

void loadFiles(Employee** eArr, int* eCount, int* eCap, Task** tArr, int* tCount, int* tCap, bool* isLoaded);

void freeMemory(Employee** eArr, Task** tArr);

#endif
