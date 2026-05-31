#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include <locale.h>

#include "data.h"
#include "memory.h"
#include "files.h"
#include "ui.h"
#include "crud.h"
#include "processing.h"

int main(void)
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    Employee* eArr = NULL;
    Task* tArr = NULL;
    int eCount = 0, eCap = 0;
    int tCount = 0, tCap = 0;
    bool isLoaded = false;

    while (1) {
        printMainMenu();
        int choice = getValidInt();

        if (choice != 1 && choice != 10 && !isLoaded) {
            printf("Сначала загрузите данные (пункт 1).\n");
            continue;
        }

        switch (choice) {

        case 1:
            loadFiles(&eArr, &eCount, &eCap, &tArr, &tCount, &tCap, &isLoaded);
            break;

        case 2: {
            int sub = submenuTable();
            if (sub == 1) 
                showEmployees(eArr, eCount);
            
            else if (sub == 2) 
                showTasks(tArr, tCount);
            
            else 
                printf("Отмена.\n");
            
            break;
        }

        case 3: {
            int sub = submenuTable();
            if (sub == 0) { 
                printf("Отмена.\n"); 
                break; 
            }
            
            if (sub == 1) {
                printf("\n  Сортировать сотрудников по:\n");
                printf("  1. ID (порядок по умолчанию)\n");
                printf("  2. ФИО\n");
                printf("  3. Должности\n");
                printf("  0. Отмена\n");
                printf("  Выбор: ");
                int field = getValidInt();
                if (field == 0) 
                    printf("Отмена.\n");

                else if (field == 1) 
                    showEmployees(eArr, eCount);
                
                else if (field == 2) 
                    showEmployeesSorted(eArr, &idxEmpFIO);
                
                else if (field == 3) 
                    showEmployeesSorted(eArr, &idxEmpPosition);
                
                else                 
                    printf("Неверный выбор.\n");
            }
            else if (sub == 2) {
                printf("\n  Сортировать задачи по:\n");
                printf("  1. ID (порядок по умолчанию)\n");
                printf("  2. Названию проекта\n");
                printf("  3. Дедлайну\n");
                printf("  0. Отмена\n");
                printf("  Выбор: ");
                int field = getValidInt();
                if (field == 0) printf("Отмена.\n");
                else if (field == 1) showTasks(tArr, tCount);
                else if (field == 2) showTasksSorted(tArr, &idxTaskProject);
                else if (field == 3) showTasksSorted(tArr, &idxTaskDeadline);
                else                 printf("Неверный выбор.\n");
            }
            break;
        }

        case 4: {
            printf("\n  Поиск:\n");
            printf("  1. Сотрудник по ФИО\n");
            printf("  2. Сотрудник по должности\n");
            printf("  3. Задача по проекту\n");
            printf("  4. Задача по дедлайну\n");
            printf("  0. Отмена\n");
            printf("  Выбор: ");
            int sub = getValidInt();
            if (sub == 0) { 
                printf("Отмена.\n"); 
                break; 
            }

            if (sub == 1) {
                char buf[MAX_FIO];
                printf("ФИО: ");
                getValidString(buf, MAX_FIO);
                findEmployeeByFIO(eArr, buf);
            }
            else if (sub == 2) {
                char buf[MAX_POSITION];
                printf("Должность: ");
                getValidString(buf, MAX_POSITION);
                findEmployeeByPosition(eArr, buf);
            }
            else if (sub == 3) {
                char buf[MAX_PROJECT];
                printf("Проект: ");
                getValidString(buf, MAX_PROJECT);
                findTaskByProject(tArr, buf);
            }
            else if (sub == 4) {
                char buf[MAX_DATE];
                printf("Дедлайн (ДД.ММ.ГГГГ): ");
                getValidString(buf, MAX_DATE);
                findTaskByDeadline(tArr, buf);
            }
            else {
                printf("Неверный выбор.\n");
            }
            break;
        }

        case 5: {
            int sub = submenuTable();
            
            if (sub == 1) 
                addEmployee(&eArr, &eCount, &eCap);
            
            else if (sub == 2) 
                addTask(&tArr, &tCount, &tCap, eArr, eCount);
            
            else  
                printf("Отмена.\n");
            
            break;
        }

        case 6: {
            int sub = submenuTable();
            if (sub == 1) 
                deleteEmployee(eArr, eCount, tArr, tCount);
            
            else if (sub == 2) 
                deleteTask(tArr, tCount);
            
            else 
                printf("Отмена.\n");
            
            break;
        }

        case 7: {
            int sub = submenuTable();
            if (sub == 1) 
                editEmployee(eArr, eCount);
            
            else if (sub == 2) 
                editTask(tArr, tCount, eArr, eCount);
            
            else 
                printf("Отмена.\n");
            
            break;
        }

        case 8:
            sf_ExportProjectTasks(tArr, tCount);
            break;

        case 9:
            sf_ExportMonthTasks(tArr, tCount);
            break;

        case 10:
            freeMemory(&eArr, &tArr);
            return 0;

        case 11:
            saveFiles(eArr, eCount, tArr, tCount);
            freeMemory(&eArr, &tArr);
            return 0;

        default:
            printf("Неверный пункт. Введите число из меню.\n");
            break;
        }
    }
}
