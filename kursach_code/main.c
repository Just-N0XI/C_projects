#define _CRT_SECURE_NO_WARNINGS
#include "vin_search.h"
#include <locale.h>

 /* --- поиск --- */

static SearchResult do_search(Database* db, const char* query) {
    int qlen = (int)strlen(query);
    if (qlen == VIN_LEN) {
        printf("Выполняется точный поиск\n");
        int idx = search_exact(db, query);
        SearchResult sr = { NULL, 0 };
        if (idx >= 0) {
            sr.indices = (int*)malloc(sizeof(int));
            if (sr.indices) { sr.indices[0] = idx; sr.count = 1; }
        }
        return sr;
    }
    else {
        printf("Выполняется частичный поиск\n");
        return search_partial(db, query);
    }
}

/* --- подсчёт всех записей в БД --- */

static int db_total_count(const Database* db) {
    int total = 0;
    for (int s = 0; s < db->num_sections; s++)
        total += db->sections[s].count;
    return total;
}

/* --- ввод поля с подстановкой '-' при пустом вводе --- */

static void read_field(const char* prompt, char* buf, int size) {
    read_line(prompt, buf, size);
    if (buf[0] == '\0') {
        buf[0] = '-';
        buf[1] = '\0';
    }
}

/* --- ввод новой записи --- */

static void menu_add(Database* db) {
    Record r;
    memset(&r, 0, sizeof(r));
    char buf[MAX_FIELD * 2];

    while (1) {
        read_line("VIN (17 символов): ", buf, sizeof(buf));
        to_upper_str(buf);
        if (validate_vin(buf) == OK) break;
    }
    safe_strncpy(r.vin, buf, sizeof(r.vin));

    read_field("Гос. номер        : ", buf, sizeof(buf));
    safe_strncpy(r.plate, buf, sizeof(r.plate));

    read_field("Марка             : ", buf, sizeof(buf));
    safe_strncpy(r.brand, buf, sizeof(r.brand));

    read_field("Модель            : ", buf, sizeof(buf));
    safe_strncpy(r.model, buf, sizeof(r.model));

    read_field("Фамилия           : ", buf, sizeof(buf));
    safe_strncpy(r.surname, buf, sizeof(r.surname));

    read_field("Имя               : ", buf, sizeof(buf));
    safe_strncpy(r.name, buf, sizeof(r.name));

    read_field("Отчество          : ", buf, sizeof(buf));
    safe_strncpy(r.patronymic, buf, sizeof(r.patronymic));

    read_field("Дата рождения     : ", buf, sizeof(buf));
    safe_strncpy(r.birthdate, buf, sizeof(r.birthdate));

    int rc = db_insert_record(db, &r);
    if (rc == OK)
        printf("Запись добавлена.\n\n");
    else
        printf("Ошибка при добавлении записи (код %d).\n\n", rc);
}

/* --- редактирование записи --- */

static void menu_edit(Database* db, int global_idx) {
    Record* r = &db->records[global_idx];
    char buf[MAX_FIELD * 2];

    while(1) {
        printf("\nЧто редактировать?\n"
            "  1. Фамилия   (сейчас: %s)\n"
            "  2. Имя       (сейчас: %s)\n"
            "  3. Отчество  (сейчас: %s)\n"
            "  4. Гос. номер(сейчас: %s)\n"
            "  0. Завершить редактирование\n",
            r->surname, r->name, r->patronymic, r->plate);

        int choice = ask_int("Ваш выбор: ", 0, 4);

        if (choice == 0) break;

        switch (choice) {
        case 1:
            read_field("Новая фамилия    : ", buf, sizeof(buf));
            safe_strncpy(r->surname, buf, sizeof(r->surname));
            break;
        case 2:
            read_field("Новое имя        : ", buf, sizeof(buf));
            safe_strncpy(r->name, buf, sizeof(r->name));
            break;
        case 3:
            read_field("Новое отчество   : ", buf, sizeof(buf));
            safe_strncpy(r->patronymic, buf, sizeof(r->patronymic));
            break;
        case 4:
            read_field("Новый гос. номер : ", buf, sizeof(buf));
            safe_strncpy(r->plate, buf, sizeof(r->plate));
            break;
        }
        db->modified = 1;
        printf("Изменено.\n");
    }
}

static void show_db(const Database* db) {
    if (db_total_count(db) == 0) {
        printf("Реестр пуст. Сначала добавьте записи.\n\n");
        return;
    }

    int count = 1;

    for (int s = 0; s < db->num_sections; s++) {
        const Section* sec = &db->sections[s];

        for (int i = 0; i < sec->count; i++) {
            const Record* r = &db->records[sec->start + i];

            printf("  [%d]\n"
                "  VIN           : %s\n"
                "  Гос. номер    : %s\n"
                "  Марка         : %s\n"
                "  Модель        : %s\n"
                "  Фамилия       : %s\n"
                "  Имя           : %s\n"
                "  Отчество      : %s\n"
                "  Дата рождения : %s\n\n",
                count++,
                r->vin,
                r->plate,
                r->brand,
                r->model,
                r->surname,
                r->name,
                r->patronymic,
                r->birthdate);
        }
    }
}

/* --- меню после нахождения результатов --- */

static void menu_results(Database* db, SearchResult* sr, const char* query) {
    print_results(db, sr);

    if (sr->count == 0) return;

    printf("Действие:\n"
        "  1. Удалить\n"
        "  2. Редактировать\n"
        "  3. Сохранить результаты в файл\n"
        "  0. Отмена\n");
    int action = ask_int("Ваш выбор: ", 0, 3);

    if (action == 0) return;

    /* Если найдено несколько и нужно выбрать конкретную */
    int target_idx = -1; /* глобальный индекс выбранной записи */
    if (action == 1 || action == 2) {
        if (sr->count == 1) {
            target_idx = sr->indices[0];
        }
        else {
            printf("Найдено несколько записей. Введите номер (1–%d): ", sr->count);
            int pick = ask_int("", 1, sr->count);
            target_idx = sr->indices[pick - 1];
        }
    }

    switch (action) {
    case 1: /* удалить */
        printf("\nЗапись:\n");
        print_record(stdout, &db->records[target_idx], 1);
        if (ask_yn("Удалить эту запись? (y/n): ")) {
            db_delete_record(db, target_idx);
            printf("Запись удалена.\n\n");
        }
        else {
            printf("Удаление отменено.\n\n");
        }
        break;

    case 2: /* редактировать */
        menu_edit(db, target_idx);
        break;

    case 3: /* сохранить результаты */
    {
        char out_path[MAX_PATH];
        read_line("Путь к выходному файлу (.txt): ", out_path, MAX_PATH);
        if (out_path[0] == '\0') {
            printf("Путь не указан, сохранение отменено.\n\n");
        }
        else if (strcmp(out_path, db->filepath) == 0) {
            printf("Нельзя сохранять результаты поиска в файл реестра.\n\n");
        }
        else {
            save_results(db, sr, query, out_path);
        }
        break;
    }
    }
}

/* --- меню поиска --- */

static void menu_find(Database* db) {
    if (db_total_count(db) == 0) {
        printf("Реестр пуст. Сначала добавьте записи.\n\n");
        return;
    }

    char query[VIN_LEN + 2];
    while (1) {
        read_line("Введите VIN или его часть (1–17 символов): ",
            query, sizeof(query));
        to_upper_str(query);

        int len = (int)strlen(query);
        if (len == 0) { printf("Запрос не может быть пустым.\n"); continue; }
        if (len > VIN_LEN) { printf("Запрос длиннее 17 символов.\n"); continue; }

        /* посимвольная проверка */
        int valid = 1;
        for (int i = 0; i < len; i++) {
            char c = query[i];
            if (!((c >= 'A' && c <= 'Z' && c != 'I' && c != 'O' && c != 'Q') ||
                (c >= '0' && c <= '9'))) {
                printf("Символ '%c' недопустим.\n", c);
                valid = 0; break;
            }
        }
        if (valid) break;
    }

    SearchResult sr = do_search(db, query);
    menu_results(db, &sr, query);
    search_result_free(&sr);
}

int main(void) {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    printf("==============================================\n");
    printf("  Система учёта ТС ГАИ — поиск по VIN\n");
    printf("==============================================\n\n");

    /* инициализация БД */
    Database db;
    if (db_init(&db) != OK) {
        fprintf(stderr, "Ошибка инициализации базы данных.\n");
        return 1;
    }

    /* ввод пути к файлу */
    char path[MAX_PATH];
    read_line("Введите путь к файлу реестра (.txt): ", path, MAX_PATH);
    if (path[0] == '\0') {
        fprintf(stderr, "Путь не указан. Выход.\n");
        db_free(&db);
        return 1;
    }

    if (db_load(&db, path) != OK) {
        db_free(&db);
        return 1;
    }

    while (1) {
        printf("----------------------------------------------\n");
        printf("  Главное меню\n");
        printf("  1. Найти запись\n");
        printf("  2. Добавить запись\n");
        printf("  3. Просмотр всего реестра\n");
        printf("  0. Выход\n");
        printf("----------------------------------------------\n");

        int choice = ask_int("Ваш выбор: ", 0, 3);

        if (choice == 0) break;
        if (choice == 1) menu_find(&db);
        if (choice == 2) menu_add(&db);
        if (choice == 3) show_db(&db);
    }

    /* автосохранение */
    if (db.modified) {
        printf("Сохранение изменений в \"%s\"...\n", db.filepath);
        if (db_save(&db) == OK)
            printf("Сохранено.\n");
        else
            fprintf(stderr, "Ошибка при сохранении.\n");
    }

    db_free(&db);
    return 0;
}