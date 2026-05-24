#ifndef FILES_H
#define FILES_H

#include "data.h"

void saveFiles(const Employee* eArr, int eCount, const Task* tArr, int tCount);

void sf_ExportProjectTasks(const Task* tArr, int tCount);
void sf_ExportMonthTasks(const Task* tArr, int tCount);

#endif
