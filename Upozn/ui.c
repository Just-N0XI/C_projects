#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui.h"

void printMainMenu(void)
{
    printf("\n|------------------------------------------|\n");
    printf("|      СИСТЕМА УПРАВЛЕНИЯ ПРОЕКТАМИ        |\n");
    printf("|------------------------------------------|\n");
    printf("|  1.  Чтение данных из файлов             |\n");
    printf("|  2.  Просмотр списков                    |\n");
    printf("|  3.  Сортировка                          |\n");
    printf("|  4.  Поиск                               |\n");
    printf("|  5.  Добавление записи                   |\n");
    printf("|  6.  Удаление записи                     |\n");
    printf("|  7.  Редактирование записи               |\n");
    printf("|  8.  Задачи по проекту                   |\n");
    printf("|  9.  Задачи на ближайший месяц           |\n");
    printf("| 10.  Выход без сохранения                |\n");
    printf("| 11.  Выход с сохранением                 |\n");
    printf("|__________________________________________|\n");
    printf("Выберите пункт меню: ");
}

int submenuTable(void)
{
    printf("1. Сотрудники\n");
    printf("2. Задачи\n");
    printf("0. Отмена\n");
    printf("Выбор: ");
    return getValidInt();
}

int getValidInt(void) {
    int val;
    while (1) {
        if (scanf("%d", &val) == 1)
            return val;
        int c; while ((c = getchar()) != '\n' && c != EOF);
        printf("Ошибка: ожидается число. Повторите ввод: ");
    }
}

void getValidString(char* buffer, int max_length)
{
    while (1) {
        if (!fgets(buffer, max_length, stdin)) {
            buffer[0] = '\0'; 
            return; 
        }

        if (buffer[0] == '\n') {
            continue;
        }

        int len = (int)strlen(buffer);
        
        if (len > 0 && buffer[len - 1] == '\n') { 
            buffer[len - 1] = '\0'; 
            return; 
        }

        int c = getchar();
        
        if (c == '\n' || c == EOF) 
            return;
        
        while ((c = getchar()) != '\n' && c != EOF);
        printf("Строка слишком длинная. Повторите ввод: ");
    }
}

void showEmployeesSorted(const Employee* arr, const IndexArray* idx)
{
    if (idx->count == 0) { 
        printf("Список пуст.\n"); 
        return; 
    }

    printf("\n%-6s %-30s %-20s %-6s %-6s\n",
        "ID", "ФИО", "Должность", "Часы", "Рук.");
    
    printf("%-6s %-30s %-20s %-6s %-6s\n",
        "------", "------------------------------",
        "--------------------", "------", "------");
    
    for (int i = 0; i < idx->count; i++) {
        if (strcmp(idx->entries[i].key, "\x01") == 0) 
            continue;

        const Employee* e = &arr[idx->entries[i].mainIdx];
        printf("%-6d %-30s %-20s %-6d %-6d\n",
            e->emp_id, 
            e->fio, 
            e->position,
            e->work_hours, 
            e->manager_id);
    }
}

void showEmployees(const Employee* arr, int count) {
    int visible = 0;
    for (int i = 0; i < count; i++)
        if (!arr[i].is_deleted) {
            visible++;
            break;
        }      

    if (visible == 0) {
        printf("Список пуст.\n"); 
        return;
    }

    printf("\n%-6s %-30s %-20s %-6s %-6s\n",
        "ID", "ФИО", "Должность", "Часы", "Рук.");

    printf("%-6s %-30s %-20s %-6s %-6s\n",
        "------", "------------------------------",
        "--------------------", "------", "------");

    for (int i = 0; i < count; i++) {
        if (arr[i].is_deleted) 
            continue;
        
        printf("%-6d %-30s %-20s %-6d %-6d\n",
            arr[i].emp_id,
            arr[i].fio,
            arr[i].position,
            arr[i].work_hours,
            arr[i].manager_id);
    }
}

void showTasks(const Task* arr, int count) {
    int visible = 0;
    for (int i = 0; i < count; i++)
        if (!arr[i].is_deleted) {
            visible++;
            break;
        }

    if (visible == 0) {
        printf("Список пуст.\n");
        return;
    }

    printf("\n%-6s %-20s %-28s %-6s %-6s %-12s %-12s\n",
        "ID", "Проект", "Описание", "Исп.", "Рук.", "Выдана", "Дедлайн");

    printf("%-6s %-20s %-28s %-6s %-6s %-12s %-12s\n",
        "------", "--------------------", "----------------------------",
        "------", "------", "------------", "------------");

    for (int i = 0; i < count; i++) {
        if (arr[i].is_deleted) {
            continue;
        }

        printf("%-6d %-20s %-28s %-6d %-6d %-12s %-12s\n",
            arr[i].task_id,
            arr[i].project_name,
            arr[i].task_desc,
            arr[i].executor_id,
            arr[i].manager_id,
            arr[i].gotTask_date,
            arr[i].deadline);
    }
}

void showTasksSorted(const Task* arr, const IndexArray* idx)
{
    if (idx->count == 0) { 
        printf("Список пуст.\n"); 
        return; 
    }

    printf("\n%-6s %-20s %-28s %-6s %-6s %-12s %-12s\n",
        "ID", "Проект", "Описание", "Исп.", "Рук.", "Выдана", "Дедлайн");
    
    printf("%-6s %-20s %-28s %-6s %-6s %-12s %-12s\n",
        "------", "--------------------", "----------------------------",
        "------", "------", "------------", "------------");
    
    for (int i = 0; i < idx->count; i++) {
        if (strcmp(idx->entries[i].key, "\x01") == 0) 
            continue;

        const Task* t = &arr[idx->entries[i].mainIdx];
        printf("%-6d %-20s %-28s %-6d %-6d %-12s %-12s\n",
            t->task_id, 
            t->project_name, 
            t->task_desc,
            t->executor_id, 
            t->manager_id,
            t->gotTask_date, 
            t->deadline);
    }
}