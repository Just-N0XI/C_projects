#define _CRT_SECURE_NO_WARNINGS
#include "vin_search.h"

void safe_strncpy(char* dst, const char* src, size_t size) {
    if (!dst || !src || size == 0) {
        return;
    }

    size_t i;
    for (i = 0; i + 1 < size && src[i]; i++) {
        dst[i] = src[i];
    }

    dst[i] = '\0';
}

void str_trim(char* s) {
    if (!s) {
        return;
    }

    size_t read = 0, write = 0;
    int in_space = 0;

    while (s[read] != '\0') {
        unsigned char ch = (unsigned char)s[read];

        if (isspace(ch)) {
            if (!in_space && write > 0) {
                s[write++] = ' ';
            }
            in_space = 1;
        }
        else {
            s[write++] = s[read];
            in_space = 0;
        }

        ++read;
    }

    if (write > 0 && s[write - 1] == ' ') {
        --write;
    }

    s[write] = '\0';
}

/* --- инициализация --- */

int db_init(Database* db) {
    int total = NUM_SECTIONS * SECTION_INIT;
    db->records = (Record*)calloc((size_t)total, sizeof(Record));

    if (!db->records) {
        return ERR_MEMORY;
    }

    db->total_capacity = total;
    db->num_sections = NUM_SECTIONS;
    db->modified = 0;
    db->filepath[0] = '\0';

    const char* order = SECTION_ORDER;

    for (int i = 0; i < NUM_SECTIONS; i++) {
        db->sections[i].key = order[i];
        db->sections[i].start = i * SECTION_INIT;
        db->sections[i].capacity = SECTION_INIT;
        db->sections[i].count = 0;
    }

    return OK;
}

/* --- освобождение --- */

void db_free(Database* db) {
    if (!db) {
        return;
    }

    free(db->records);
    db->records = NULL;
    db->total_capacity = 0;
    db->modified = 0;
}

/* --- чтение одной непустой строки --- */

static int next_line(FILE* fp, char* buf, int size) {
    while (fgets(buf, size, fp)) {
        str_trim(buf);

        if (buf[0] != '\0' && buf[0] != '#') {
            return 1;
        }
    }

    return 0;
}

/* --- загрузка из файла --- */

int db_load(Database* db, const char* path) {
    FILE* fp = fopen(path, "r");

    if (!fp) {
        fprintf(stderr, "Ошибка: не удалось открыть файл \"%s\".\n", path);
        return ERR_FILE;
    }

    safe_strncpy(db->filepath, path, sizeof(db->filepath));

    char line[MAX_FIELD * 2];
    int  loaded = 0;

    while (1) {
        Record r;
        memset(&r, 0, sizeof(r));

        if (!next_line(fp, line, sizeof(line))) break;
        safe_strncpy(r.vin, line, sizeof(r.vin));

        if (!next_line(fp, line, sizeof(line))) break;
        safe_strncpy(r.plate, line, sizeof(r.plate));

        if (!next_line(fp, line, sizeof(line))) break;
        safe_strncpy(r.brand, line, sizeof(r.brand));

        if (!next_line(fp, line, sizeof(line))) break;
        safe_strncpy(r.model, line, sizeof(r.model));

        if (!next_line(fp, line, sizeof(line))) break;
        safe_strncpy(r.surname, line, sizeof(r.surname));

        if (!next_line(fp, line, sizeof(line))) break;
        safe_strncpy(r.name, line, sizeof(r.name));

        if (!next_line(fp, line, sizeof(line))) break;
        safe_strncpy(r.patronymic, line, sizeof(r.patronymic));

        if (!next_line(fp, line, sizeof(line))) break;
        safe_strncpy(r.birthdate, line, sizeof(r.birthdate));

        to_upper_str(r.vin);

        int rc = db_insert_record(db, &r);

        if (rc != OK) {
            fprintf(stderr, "Предупреждение: не удалось вставить запись %d (код %d).\n",
                loaded + 1, rc);
        }
        else {
            loaded++;
        }
    }

    fclose(fp);
    printf("Загружено записей: %d\n", loaded);

    return OK;
}

/* --- сохранение в тот же файл --- */

int db_save(const Database* db) {
    if (!db || db->filepath[0] == '\0') {
        return ERR_FILE;
    }

    FILE* fp = fopen(db->filepath, "w");

    if (!fp) {
        fprintf(stderr, "Ошибка: не удалось открыть файл для записи: \"%s\".\n",
            db->filepath);
        return ERR_FILE;
    }

    /* обход секций в алфавитном порядке */
    for (int s = 0; s < db->num_sections; s++) {
        const Section* sec = &db->sections[s];

        for (int i = 0; i < sec->count; i++) {
            const Record* r = &db->records[sec->start + i];

            fprintf(fp, "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
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

    fclose(fp);
    return OK;
}