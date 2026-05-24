#ifndef DATA_H
#define DATA_H

#include <stdbool.h>
#include <stddef.h>
//#include <locale.h>

#define MAX_FIO 100
#define MAX_POSITION 80
#define MAX_PROJECT 100
#define MAX_DESC 200
#define MAX_DATE 20

/* --- Структуры --- */

typedef struct {
    int  emp_id;
    char fio[MAX_FIO];
    char position[MAX_POSITION];
    int  work_hours;
    int  manager_id;
    bool is_deleted;
} Employee;

typedef struct {
    int  task_id;
    char project_name[MAX_PROJECT];
    char task_desc[MAX_DESC];
    int  executor_id;
    int  manager_id;
    char gotTask_date[MAX_DATE];
    char deadline[MAX_DATE];
    bool is_deleted;
} Task;

/* --- Индексный массив --- */
/* mainIdx — индекс записи в основном массиве
   key — строковый ключ сортировки (ФИО, должность, проект, дедлайн) */

#define MAX_KEY 100

typedef struct {
    int  mainIdx; // номер записи в основном массиве (eArr или tArr)
    char key[MAX_KEY]; // ключ сортировки (ФИО, должность, ...)
} IndexEntry;

typedef struct {
    IndexEntry* entries; 
    int count; // сколько записей сейчас
    int capacity; // сколько выделено памяти
} IndexArray;

/* --- Глобальные счётчики ID --- */

extern int Global_Max_Emp_ID;
extern int Global_Max_Task_ID;

#endif