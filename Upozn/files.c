#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "files.h"
#include "ui.h"

void saveFiles(const Employee* eArr, int eCount, const Task* tArr, int tCount) {
	FILE* fe = fopen("employees.bin", "wb");
	if (!fe) {
		fprintf(stderr, "Ошибка: невозможно сохранить данные (employees.bin).\n");
		return;
	}

	for (int i = 0; i < eCount; i++)
		if (!eArr[i].is_deleted)
			fwrite(&eArr[i], sizeof(Employee), 1, fe);
	fclose(fe);

	FILE* ft = fopen("tasks.bin", "wb");
	if (!ft) {
		fprintf(stderr, "Ошибка: невозможно сохранить данные (tasks.bin).\n");
		return;
	}

	for (int i = 0; i < tCount; i++)
		if (!tArr->is_deleted)
			fwrite(&tArr[i], sizeof(Task), 1, ft);
	fclose(ft);

	printf("Данные успешно сохранены.\n");
}

void sf_ExportProjectTasks(const Task* tArr, int tCount) {
	char project[MAX_PROJECT];
	printf("Введите название проекта: ");
	
	getValidString(project, sizeof(project));

	if (project[0] == '\0' || (project[0] == '0' && project[1] == '\0')) {
		return;
	}
	project[strcspn(project, "\n")] = '\0';

	FILE* fout = fopen("project_tasks.txt", "w");
	if (!fout)
		fprintf(stderr, "Не удалось создать файл отчёта.\n");
	
	int found = 0;

	printf("\n%-6s %-20s %-28s %-6s %-6s %-12s %-12s\n",
		"ID", "Проект", "Описание", "Исп.", "Рук.", "Выдана", "Дедлайн");

	printf("%-6s %-20s %-28s %-6s %-6s %-12s %-12s\n",
		"------", "--------------------", "----------------------------",
		"------", "------", "------------", "------------");

	for (int i = 0; i < tCount; i++) {
		if (tArr[i].is_deleted)
			continue;

		if (strcmp(tArr[i].project_name, project) != 0)
			continue;

		printf("%-6d %-20s %-30s %-6d %-6d %-12s %-12s\n",
			tArr[i].task_id, 
			tArr[i].project_name, 
			tArr[i].task_desc,
			tArr[i].executor_id, 
			tArr[i].manager_id,
			tArr[i].gotTask_date, 
			tArr[i].deadline);

		if (fout)
			fprintf(fout, "%d | %s | %s | %d | %d | %s | %s\n",
				tArr[i].task_id, 
				tArr[i].project_name, 
				tArr[i].task_desc,
				tArr[i].executor_id, 
				tArr[i].manager_id,
				tArr[i].gotTask_date, 
				tArr[i].deadline);
		found++;
	}

	if (found == 0) {
		printf("Задач по данному проекту не найдено.\n");
		if (fout)
			fprintf(fout, "Задач по данному проекту не найдено.\n");
	}
	if (fout)
		fclose(fout);
}

static int parseDate(const char* s, struct tm* out) {
	int day, month, year;
	if (sscanf(s, "%d.%d.%d", &day, &month, &year) != 3) 
		return 0;
	if (day < 1 || day > 31 || month < 1 || month > 12 || year < 1900) 
		return 0;
	out->tm_mday = day;
	out->tm_mon = month - 1;
	out->tm_year = year - 1900;
	out->tm_hour = out->tm_min = out->tm_sec = 0;
	out->tm_isdst = -1;
	return 1;
}

void sf_ExportMonthTasks(const Task* tArr, int tCount) {
	time_t now = time(NULL);

	FILE* fout = fopen("month_tasks.txt", "w");
	if (!fout)
		fprintf(stderr, "Не удалось создать файл отчёта.\n");
	
	int found = 0;

	printf("\n%-6s %-20s %-30s %-12s\n",
		"ID", "Проект", "Описание", "Дедлайн");

	printf("%-6s %-20s %-30s %-12s\n",
		"------", "--------------------",
		"------------------------------", "------------");

	for (int i = 0; i < tCount; i++) {
		if (tArr[i].is_deleted)
			continue;

		struct tm timeDL = { 0 };
		if (!parseDate(tArr[i].deadline, &timeDL)) {
			printf("Ошибка считывания даты для задачи ID %d. Запись пропущена в отчёте.\n", tArr[i].task_id);
			continue;
		}

		time_t DL = mktime(&timeDL);
		double diff = difftime(DL, now);

		if (diff < 0 || diff > 30.0 * 86400.0) 
			continue;

		printf("%-6d %-20s %-30s %-12s\n",
			tArr[i].task_id,
			tArr[i].project_name,
			tArr[i].task_desc, 
			tArr[i].deadline);

		if (fout)
			fprintf(fout, "%d | %s | %s | %s\n",
				tArr[i].task_id, 
				tArr[i].project_name,
				tArr[i].task_desc, 
				tArr[i].deadline);
		found++;
	}

	if (found == 0) {
		printf("Задач на ближайший месяц не найдено.\n");
		if (fout) fprintf(fout, "Задач на ближайший месяц не найдено.\n");
	}
	if (fout) fclose(fout);
}
