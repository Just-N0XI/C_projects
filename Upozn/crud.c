#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "crud.h"
#include "ui.h"
#include "memory.h"
#include "processing.h"

void addEmployee(Employee** arr, int* count, int* capacity)
{
    Employee tmp;
    memset(&tmp, 0, sizeof(Employee));
    tmp.is_deleted = false;
    tmp.emp_id = ++Global_Max_Emp_ID;

    printf("Добавление сотрудника (ID будет %d)\n", tmp.emp_id);
    printf("0 в любом поле — отмена.\n\n");

    printf("ФИО: ");
    getValidString(tmp.fio, MAX_FIO);
    if (tmp.fio[0] == '0' && tmp.fio[1] == '\0') {
        printf("Отмена.\n"); 
        Global_Max_Emp_ID--; 
        return;
    }
    if (tmp.fio[0] == '\0') {
        printf("ФИО не может быть пустым.\n"); 
        Global_Max_Emp_ID--; 
        return;
    }

    printf("Должность: ");
    getValidString(tmp.position, MAX_POSITION);
    if (tmp.position[0] == '0' && tmp.position[1] == '\0') {
        printf("Отмена.\n"); Global_Max_Emp_ID--; return;
    }

    printf("Рабочих часов в сутки: ");
    tmp.work_hours = getValidInt();
    if (tmp.work_hours == 0) {
        printf("Отмена.\n"); 
        Global_Max_Emp_ID--; 
        return;
    }

    printf("ID руководителя (0 — нет): ");
    int mid = getValidInt();
    if (mid != 0 && getEmployeeIndexByID(*arr, *count, mid) == -1) {
        printf("Сотрудник с таким ID не найден. Поле оставлено 0.\n");
        mid = 0;
    }
    tmp.manager_id = mid;

    if (*count == *capacity)
        expandArray((void**)arr, capacity, sizeof(Employee));
    if (*count == *capacity) {
        fprintf(stderr, "Не удалось выделить память. Запись не добавлена.\n");
        Global_Max_Emp_ID--; 
        return;
    }

    int newIdx = *count;
    (*arr)[newIdx] = tmp;
    (*count)++;

    insertIntoIndex(&idxEmpFIO, newIdx, (*arr)[newIdx].fio);
    insertIntoIndex(&idxEmpPosition, newIdx, (*arr)[newIdx].position);

    printf("Сотрудник добавлен. ID = %d\n", (*arr)[newIdx].emp_id);
}

void addTask(Task** tArr, int* tCount, int* tCap, const Employee* eArr, int eCount)
{
    Task tmp;
    memset(&tmp, 0, sizeof(Task));
    tmp.is_deleted = false;
    tmp.task_id = ++Global_Max_Task_ID;

    printf("Добавление задачи (ID будет %d)\n", tmp.task_id);
    printf("0 в любом поле — отмена.\n\n");

    printf("Название проекта: ");
    getValidString(tmp.project_name, MAX_PROJECT);
    if (tmp.project_name[0] == '0' && tmp.project_name[1] == '\0') {
        printf("Отмена.\n"); 
        Global_Max_Task_ID--; 
        return;
    }
    if (tmp.project_name[0] == '\0') {
        printf("Название не может быть пустым.\n"); 
        Global_Max_Task_ID--; 
        return;
    }

    printf("Описание задачи: ");
    getValidString(tmp.task_desc, MAX_DESC);
    if (tmp.task_desc[0] == '0' && tmp.task_desc[1] == '\0') {
        printf("Отмена.\n"); 
        Global_Max_Task_ID--; 
        return;
    }

    printf("ID исполнителя: ");
    int eid = getValidInt();
    if (eid == 0) { 
        printf("Отмена.\n"); 
        Global_Max_Task_ID--; 
        return; 
    }
    if (getEmployeeIndexByID(eArr, eCount, eid) == -1) {
        printf("Ошибка: сотрудник не найден. Повторите ввод (0 — отмена): ");
        eid = getValidInt();
        if (eid == 0 || getEmployeeIndexByID(eArr, eCount, eid) == -1) {
            printf("Отмена.\n"); 
            Global_Max_Task_ID--; 
            return;
        }
    }
    tmp.executor_id = eid;

    printf("ID руководителя (0 — нет): ");
    tmp.manager_id = getValidInt();

    printf("Дата выдачи (ДД.ММ.ГГГГ, 0 — отмена): ");
    getValidString(tmp.gotTask_date, MAX_DATE);
    if (tmp.gotTask_date[0] == '0' && tmp.gotTask_date[1] == '\0') {
        printf("Отмена.\n"); 
        Global_Max_Task_ID--; 
        return;
    }

    printf("Дедлайн (ДД.ММ.ГГГГ, 0 — отмена): ");
    getValidString(tmp.deadline, MAX_DATE);
    if (tmp.deadline[0] == '0' && tmp.deadline[1] == '\0') {
        printf("Отмена.\n"); 
        Global_Max_Task_ID--; 
        return;
    }

    if (*tCount == *tCap)
        expandArray((void**)tArr, tCap, sizeof(Task));
    if (*tCount == *tCap) {
        fprintf(stderr, "Не удалось выделить память. Запись не добавлена.\n");
        Global_Max_Task_ID--; 
        return;
    }

    int newIdx = *tCount;
    (*tArr)[newIdx] = tmp;
    (*tCount)++;

    char sortKey[MAX_KEY];
    {
        int d = 0, m = 0, y = 0;
        if (sscanf(tmp.deadline, "%d.%d.%d", &d, &m, &y) == 3) {
            snprintf(sortKey, sizeof(sortKey), "%04d%02d%02d", y, m, d);
        }
        else {
            snprintf(sortKey, sizeof(sortKey), "99991231");
        }
    }
    insertIntoIndex(&idxTaskProject, newIdx, (*tArr)[newIdx].project_name);
    insertIntoIndex(&idxTaskDeadline, newIdx, sortKey);

    printf("Задача добавлена. ID = %d\n", (*tArr)[newIdx].task_id);
}

void editEmployee(Employee* arr, int count)
{
    printf("\n  Поиск сотрудника для редактирования:\n");
    printf("  1. По ФИО\n");
    printf("  2. По должности\n");
    printf("  3. По ID\n");
    printf("  0. Отмена\n");
    printf("  Выбор: ");
    int sub = getValidInt();
    if (sub == 0) { 
        printf("Отмена.\n"); 
        return; 
    }

    if (sub == 1) {
        char buf[MAX_FIO];
        printf("Введите ФИО: ");
        getValidString(buf, MAX_FIO);
        findEmployeeByFIO(arr, buf);
    }
    else if (sub == 2) {
        char buf[MAX_POSITION];
        printf("Введите должность: ");
        getValidString(buf, MAX_POSITION);
        findEmployeeByPosition(arr, buf);
    }
    else if (sub != 3) {
        printf("Неверный выбор.\n"); return;
    }

    printf("Введите ID сотрудника для редактирования (0 — отмена): ");
    int id = getValidInt();
    if (id == 0) { 
        printf("Отмена.\n"); 
        return; 
    }

    int idx = getEmployeeIndexByID(arr, count, id);
    if (idx == -1) { 
        printf("Сотрудник не найден.\n"); 
        return; 
    }

    Employee* e = &arr[idx];
    printf("Редактирование\n");

    char nb[MAX_FIO];
    printf("ФИО [%s]: ", e->fio);
    getValidString(nb, MAX_FIO);
    if (nb[0] != '\0') {
        markDeletedInEmpIndexes(idx);          /* убираем старый ключ */
        strncpy(e->fio, nb, MAX_FIO - 1);
        e->fio[MAX_FIO - 1] = '\0';
        insertIntoIndex(&idxEmpFIO, idx, e->fio); /* вставляем новый */
    }

    char nb2[MAX_POSITION];
    printf("Должность [%s]: ", e->position);
    getValidString(nb2, MAX_POSITION);
    if (nb2[0] != '\0') {
        for (int i = 0; i < idxEmpPosition.count; i++)
            if (idxEmpPosition.entries[i].mainIdx == idx)
                strncpy(idxEmpPosition.entries[i].key, "\x01", MAX_KEY - 1);
        strncpy(e->position, nb2, MAX_POSITION - 1);
        e->position[MAX_POSITION - 1] = '\0';
        insertIntoIndex(&idxEmpPosition, idx, e->position);
    }

    printf("Рабочих часов [%d] (0 — не менять): ", e->work_hours);
    int h = getValidInt();
    if (h > 0) 
        e->work_hours = h;

    printf("ID руководителя [%d] (-1 — убрать, 0 — не менять): ", e->manager_id);
    int mid = getValidInt();
    if (mid == -1) 
        e->manager_id = 0;
    else if (mid > 0) {
        if (getEmployeeIndexByID(arr, count, mid) == -1)
            printf("Руководитель не найден. Поле не изменено.\n");
        else
            e->manager_id = mid;
    }

    printf("Данные обновлены.\n");
}

void editTask(Task* tArr, int tCount, const Employee* eArr, int eCount)
{
    printf("\n  Поиск задачи для редактирования:\n");
    printf("  1. По названию проекта\n");
    printf("  2. По дедлайну\n");
    printf("  3. По ID\n");
    printf("  0. Отмена\n");
    printf("  Выбор: ");
    int sub = getValidInt();
    if (sub == 0) { 
        printf("Отмена.\n"); 
        return; 
    }

    if (sub == 1) {
        char buf[MAX_PROJECT];
        printf("Введите название: ");
        getValidString(buf, MAX_PROJECT);
        findTaskByProject(tArr, buf);
    }
    else if (sub == 2) {
        char buf[MAX_DATE];
        printf("Введите дедлайн (ДД.ММ.ГГГГ): ");
        getValidString(buf, MAX_DATE);
        findTaskByDeadline(tArr, buf);
    }
    else if (sub != 3) {
        printf("Неверный выбор.\n"); 
        return;
    }

    printf("Введите ID задачи для редактирования (0 — отмена): ");
    int id = getValidInt();
    if (id == 0) { 
        printf("Отмена.\n"); 
        return; 
    }

    int idx = getTaskIndexByID(tArr, tCount, id);
    if (idx == -1) { 
        printf("Задача не найдена.\n"); 
        return; 
    }

    Task* t = &tArr[idx];
    printf("Редактирование задачи ID %d\n", t->task_id);

    /* Название проекта */
    char buf[MAX_PROJECT];
    printf("Название проекта [%s]: ", t->project_name);
    getValidString(buf, MAX_PROJECT);
    if (buf[0] != '\0') {
        for (int i = 0; i < idxTaskProject.count; i++)
            if (idxTaskProject.entries[i].mainIdx == idx)
                strncpy(idxTaskProject.entries[i].key, "\x01", MAX_KEY - 1);
        strncpy(t->project_name, buf, MAX_PROJECT - 1);
        t->project_name[MAX_PROJECT - 1] = '\0';
        insertIntoIndex(&idxTaskProject, idx, t->project_name);
    }

    /* Описание (не индексируется) */
    char buf2[MAX_DESC];
    printf("Описание [%s]: ", t->task_desc);
    getValidString(buf2, MAX_DESC);
    if (buf2[0] != '\0') {
        strncpy(t->task_desc, buf2, MAX_DESC - 1);
        t->task_desc[MAX_DESC - 1] = '\0';
    }

    /* Исполнитель (не индексируется) */
    printf("ID исполнителя [%d] (0 — не менять): ", t->executor_id);
    int eid = getValidInt();
    if (eid > 0) {
        if (getEmployeeIndexByID(eArr, eCount, eid) == -1)
            printf("Сотрудник не найден. Поле не изменено.\n");
        else
            t->executor_id = eid;
    }

    printf("ID руководителя [%d] (0 — не менять): ", t->manager_id);
    int mid = getValidInt();
    if (mid > 0) 
        t->manager_id = mid;

    char buf3[MAX_DATE];
    printf("Дедлайн [%s]: ", t->deadline);
    getValidString(buf3, MAX_DATE);
    if (buf3[0] != '\0') {
        /* помечаем старый ключ в idxTaskDeadline */
        for (int i = 0; i < idxTaskDeadline.count; i++)
            if (idxTaskDeadline.entries[i].mainIdx == idx)
                strncpy(idxTaskDeadline.entries[i].key, "\x01", MAX_KEY - 1);
        strncpy(t->deadline, buf3, MAX_DATE - 1);
        t->deadline[MAX_DATE - 1] = '\0';
        /* вставляем новый ключ в формате YYYYMMDD */
        char sortKey[MAX_KEY];
        int d = 0, m = 0, y = 0;

        if (sscanf(t->deadline, "%d.%d.%d", &d, &m, &y) == 3) {
            snprintf(sortKey, sizeof(sortKey), "%04d%02d%02d", y, m, d);
        }
        else {
            // Безопасное значение по умолчанию, если дата повреждена
            snprintf(sortKey, sizeof(sortKey), "99991231");
        }
    }

    printf("Данные обновлены.\n");
}

bool hasDependentTasks(int emp_id, const Task* tArr, int tCount)
{
    for (int i = 0; i < tCount; i++) {
        if (tArr[i].is_deleted) 
            continue;

        if (tArr[i].executor_id == emp_id || tArr[i].manager_id == emp_id)
            return true;
    }
    return false;
}

/* 
   Вспомогательная: выбрать одну запись из найденных
   Возвращает mainIdx в основном массиве или -1.
 */
static int chooseFromResults(int* results, int cnt,
    const Employee* eArr, int eCount,
    const Task* tArr, int tCount,
    int mode) /* 0 = employees, 1 = tasks */
{
    if (cnt == 1) return results[0];

    printf("Найдено несколько записей. Выберите:\n");
    printf("  0. Отмена\n");
    for (int i = 0; i < cnt; i++) {
        int mi = results[i];
        if (mode == 0 && mi < eCount)
            printf("  %d. ID=%-4d %s (%s)\n", i + 1, eArr[mi].emp_id, eArr[mi].fio, eArr[mi].position);
        else if (mode == 1 && mi < tCount)
            printf("  %d. ID=%-4d %s | %s\n", i + 1, tArr[mi].task_id, tArr[mi].project_name, tArr[mi].deadline);
    }
    printf("  Выбор: ");
    int ch = getValidInt();
    if (ch == 0 || ch > cnt) { 
        printf("Отмена.\n"); 
        return -1; 
    }

    return results[ch - 1];
}


void deleteEmployee(Employee* eArr, int eCount,
    const Task* tArr, int tCount)
{
    printf("\n  Удаление сотрудника — поиск по:\n");
    printf("  1. ФИО\n");
    printf("  2. Должности\n");
    printf("  3. ID\n");
    printf("  0. Отмена\n");
    printf("  Выбор: ");
    int sub = getValidInt();
    if (sub == 0) { 
        printf("Отмена.\n"); 
        return; 
    }

    int mainIdx = -1;

    if (sub == 1) {
        char buf[MAX_FIO];
        printf("Введите ФИО: ");
        getValidString(buf, MAX_FIO);
        int cnt = 0;
        int* res = binarySearchIndex(&idxEmpFIO, buf, &cnt);
        if (!res || cnt == 0) { 
            printf("Ничего не найдено.\n"); 
            free(res); 
            return; 
        }
        /* вывести найденных */
        printf("%-6s %-30s %-20s\n", "ID", "ФИО", "Должность");
        for (int i = 0; i < cnt; i++) {
            int mi = res[i];
            if (mi >= eCount) 
                continue;
            printf("%-6d %-30s %-20s\n",
                eArr[mi].emp_id, eArr[mi].fio, eArr[mi].position);
        }
        mainIdx = chooseFromResults(res, cnt, eArr, eCount, NULL, 0, 0);
        free(res);

    }
    else if (sub == 2) {
        char buf[MAX_POSITION];
        printf("Введите должность: ");
        getValidString(buf, MAX_POSITION);
        int cnt = 0;
        int* res = binarySearchIndex(&idxEmpPosition, buf, &cnt);
        if (!res || cnt == 0) { 
            printf("Ничего не найдено.\n"); 
            free(res); 
            return; 
        }
        printf("%-6s %-30s %-20s\n", "ID", "ФИО", "Должность");
        for (int i = 0; i < cnt; i++) {
            int mi = res[i];
            if (mi >= eCount) 
                continue;
            printf("%-6d %-30s %-20s\n", eArr[mi].emp_id, eArr[mi].fio, eArr[mi].position);
        }
        mainIdx = chooseFromResults(res, cnt, eArr, eCount, NULL, 0, 0);
        free(res);

    }
    else if (sub == 3) {
        printf("Введите ID (0 — отмена): ");
        int id = getValidInt();
        if (id == 0) { 
            printf("Отмена.\n"); 
            return; 
        }
        mainIdx = getEmployeeIndexByID(eArr, eCount, id);
        if (mainIdx == -1) { 
            printf("Сотрудник не найден.\n"); 
            return; 
        }

    }
    else {
        printf("Неверный выбор.\n"); 
        return;
    }

    if (mainIdx == -1) 
        return;

    if (hasDependentTasks(eArr[mainIdx].emp_id, tArr, tCount)) {
        printf("Ошибка: сотрудник назначен в существующих задачах.\n");
        return;
    }

    printf("Удалить сотрудника ID %d — %s? (1 — да, 0 — отмена): ",
        eArr[mainIdx].emp_id, eArr[mainIdx].fio);
    int confirm = getValidInt();
    if (confirm != 1) { 
        printf("Отмена.\n"); 
        return; 
    }

    eArr[mainIdx].is_deleted = true;
    markDeletedInEmpIndexes(mainIdx);
    printf("Сотрудник удалён.\n");
}


void deleteTask(Task* tArr, int tCount)
{
    printf("\n  Удаление задачи — поиск по:\n");
    printf("  1. Названию проекта\n");
    printf("  2. Дедлайну\n");
    printf("  3. ID\n");
    printf("  0. Отмена\n");
    printf("  Выбор: ");
    int sub = getValidInt();
    if (sub == 0) { 
        printf("Отмена.\n"); 
        return; 
    }

    int mainIdx = -1;

    if (sub == 1) {
        char buf[MAX_PROJECT];
        printf("Введите название: ");
        getValidString(buf, MAX_PROJECT);
        int cnt = 0;
        int* res = binarySearchIndex(&idxTaskProject, buf, &cnt);
        if (!res || cnt == 0) { 
            printf("Ничего не найдено.\n"); 
            free(res); 
            return; 
        }
        printf("%-6s %-20s %-28s %-12s\n", "ID", "Проект", "Описание", "Дедлайн");
        for (int i = 0; i < cnt; i++) {
            int mi = res[i];
            if (mi >= tCount) 
                continue;
            printf("%-6d %-20s %-28s %-12s\n",
                tArr[mi].task_id, tArr[mi].project_name, tArr[mi].task_desc, tArr[mi].deadline);
        }
        mainIdx = chooseFromResults(res, cnt, NULL, 0, tArr, tCount, 1);
        free(res);

    }
    else if (sub == 2) {
        char buf[MAX_DATE];
        printf("Введите дедлайн (ДД.ММ.ГГГГ): ");
        getValidString(buf, MAX_DATE);
        int cnt = 0;
        char sortKey[MAX_KEY];
        {
            int d = 0, m = 0, y = 0;

            if (sscanf(buf, "%d.%d.%d", &d, &m, &y) == 3) {
                snprintf(sortKey, sizeof(sortKey), "%04d%02d%02d", y, m, d);
            }
            else {
                snprintf(sortKey, sizeof(sortKey), "99991231");
            }
        }
        int* res = binarySearchIndex(&idxTaskDeadline, sortKey, &cnt);
        if (!res || cnt == 0) { 
            printf("Ничего не найдено.\n"); 
            free(res); 
            return; 
        }
        printf("%-6s %-20s %-12s\n", "ID", "Проект", "Дедлайн");
        for (int i = 0; i < cnt; i++) {
            int mi = res[i];
            if (mi >= tCount) 
                continue;
            printf("%-6d %-20s %-12s\n", tArr[mi].task_id, tArr[mi].project_name, tArr[mi].deadline);
        }
        mainIdx = chooseFromResults(res, cnt, NULL, 0, tArr, tCount, 1);
        free(res);

    }
    else if (sub == 3) {
        printf("Введите ID (0 — отмена): ");
        int id = getValidInt();
        if (id == 0) { 
            printf("Отмена.\n"); 
            return; 
        }
        mainIdx = getTaskIndexByID(tArr, tCount, id);
        if (mainIdx == -1) { 
            printf("Задача не найдена.\n"); 
            return; 
        }

    }
    else {
        printf("Неверный выбор.\n"); 
        return;
    }

    if (mainIdx == -1) 
        return;

    printf("Удалить задачу ID %d — %s? (1 — да, 0 — отмена): ",
        tArr[mainIdx].task_id, tArr[mainIdx].project_name);
    int confirm = getValidInt();
    if (confirm != 1) { 
        printf("Отмена.\n"); 
        return; 
    }

    tArr[mainIdx].is_deleted = true;
    markDeletedInTaskIndexes(mainIdx);
    printf("Задача удалена.\n");
}