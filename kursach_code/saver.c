#define _CRT_SECURE_NO_WARNINGS
#include "vin_search.h"

#define SEP "------------------------------------------------------------\n"

void print_record(FILE* out, const Record* r, int num) {
    if (!out || !r) {
        return;
    }

    fprintf(out,
        "  [%d]\n"
        "  VIN           : %s\n"
        "  Гос. номер    : %s\n"
        "  Марка         : %s\n"
        "  Модель        : %s\n"
        "  Фамилия       : %s\n"
        "  Имя           : %s\n"
        "  Отчество      : %s\n"
        "  Дата рождения : %s\n",
        num,
        r->vin, r->plate, r->brand, r->model,
        r->surname, r->name, r->patronymic, r->birthdate);
}

void print_results(const Database* db, const SearchResult* sr) {
    if (!db || !sr) {
        return;
    }

    printf("\n" SEP);
    printf("Найдено записей: %d\n", sr->count);
    printf(SEP);

    if (sr->count == 0) {
        printf("  Совпадений не найдено.\n");
    }
    else {
        for (int i = 0; i < sr->count; i++) {
            print_record(stdout, &db->records[sr->indices[i]], i + 1);

            if (i < sr->count - 1) {
                printf("\n");
            }
        }
    }

    printf(SEP "\n");
}

int save_results(const Database* db, const SearchResult* sr,
    const char* query, const char* out_path) {
    if (!db || !sr || !query || !out_path) {
        return ERR_FILE;
    }

    FILE* fp = fopen(out_path, "w");

    if (!fp) {
        fprintf(stderr, "Ошибка: не удалось открыть файл \"%s\".\n", out_path);
        return ERR_FILE;
    }

    time_t now = time(NULL);
    char tbuf[64] = "н/д";
    struct tm* tm_info = localtime(&now);

    if (tm_info) {
        strftime(tbuf, sizeof(tbuf), "%d.%m.%Y %H:%M:%S", tm_info);
    }

    fprintf(fp,
        "============================\n"
        " РЕЗУЛЬТАТЫ ПОИСКА ВИН\n"
        "============================\n"
        "Дата и время : %s\n"
        "Запрос       : \"%s\"\n"
        "Найдено      : %d\n"
        "============================\n\n",
        tbuf, query, sr->count);

    if (sr->count == 0) {
        fprintf(fp, "Совпадений не найдено.\n");
    }
    else {
        for (int i = 0; i < sr->count; i++) {
            print_record(fp, &db->records[sr->indices[i]], i + 1);
            fprintf(fp, "\n");
        }
    }

    fprintf(fp, "============================\n");
    fclose(fp);

    printf("Результаты сохранены: %s\n\n", out_path);
    return OK;
}