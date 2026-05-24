#define _CRT_SECURE_NO_WARNINGS
#include "vin_search.h"


 /* --- поиск индекса секции по ключевому символу --- */

int section_index_of(const Database* db, char key) {
    key = (char)toupper((unsigned char)key);

    for (int i = 0; i < db->num_sections; i++) {
        if (db->sections[i].key == key) {
            return i;
        }
    }

    return -1;
}

/* --- расширение массива при переполнении секции --- */

/*
 * ѕереполнена секци€ с индексом sx.
 *   1. сохран€ем данные переполненной секции X во временный буфер.
 *   2. расшир€ем главный массив записей realloc-ом ровно на old_cap элементов.
 *   3. сдвигаем данные всех последующих секций (начина€ с sx+1) влево
 *      на место секции X и обновл€ем их индексы start (по их capacity).
 *   4. копируем сохраненные данные X в самый конец главного массива,
 *      где дл€ секции X теперь выделено удвоенное пространство (new_cap = old_cap * 2).
 */
static int overflow_section(Database* db, int sx) {
    Section* secX = &db->sections[sx];
    /* 1. */
    int     old_cap = secX->capacity;
    int     old_count = secX->count;
    int     new_cap = old_cap * 2;
    Record* tmp = (Record*)malloc((size_t)old_count * sizeof(Record));

    if (!tmp) {
        return ERR_MEMORY;
    }

    memcpy(tmp, &db->records[secX->start], (size_t)old_count * sizeof(Record));

    /* 2. */
    int new_total = db->total_capacity + old_cap;
    Record* new_rec = (Record*)realloc(db->records,
        (size_t)new_total * sizeof(Record));

    if (!new_rec) {
        free(tmp);
        return ERR_MEMORY;
    }

    db->records = new_rec;
    db->total_capacity = new_total;

    /* 3. */
    int x_start = secX->start;

    if (sx + 1 < db->num_sections) {
        Section* secY = &db->sections[sx + 1];
        int      y_start = secY->start;

        /* считаем суммарный выделенный объем (capacity) всех последующих секций */
        int capacity_after = 0;
        for (int i = sx + 1; i < db->num_sections; i++) {
            capacity_after += db->sections[i].capacity;
        }

        if (capacity_after > 0) {
            /* перемещаем данные блоками по capacity, чтобы не потер€ть пустые €чейки */
            memmove(&db->records[x_start],
                &db->records[y_start],
                (size_t)capacity_after * sizeof(Record));
        }

        /* обновл€ем start всех секций после X строго по их capacity */
        int cursor = x_start;
        for (int i = sx + 1; i < db->num_sections; i++) {
            db->sections[i].start = cursor;
            cursor += db->sections[i].capacity;
        }
    }

    /* 4. */
    int new_start = db->total_capacity - new_cap;
    memcpy(&db->records[new_start], tmp, (size_t)old_count * sizeof(Record));
    free(tmp);

    secX->start = new_start;
    secX->capacity = new_cap;
    secX->count = old_count;

    return OK;
}

/* --- вставка записи в нужную секцию (с сортировкой) --- */

int db_insert_record(Database* db, const Record* r) {
    /* найти секцию по первому символу VIN */
    int sx = section_index_of(db, r->vin[0]);

    if (sx < 0) {
        return ERR_INVALID;
    }

    Section* sec = &db->sections[sx];

    /* если секци€ переполнена Ч расширить */
    if (sec->count >= sec->capacity) {
        int rc = overflow_section(db, sx);

        if (rc != OK) {
            return rc;
        }

        /* обновл€ем указатель после возможного realloc */
        sec = &db->sections[sx];
    }

    /* найти позицию вставки (бинарный поиск в пределах секции) */
    int i = 0, j = sec->count;

    while (i < j) {
        int mid = (i + j) / 2;

        if (strcmp(db->records[sec->start + mid].vin, r->vin) <= 0) {
            i = mid + 1;
        }
        else {
            j = mid;
        }
    }

    /* lo Ч позици€ вставки внутри секции */
    int insert_pos = sec->start + i;

    /* сдвигаем элементы вправо */
    if (i < sec->count) {
        memmove(&db->records[insert_pos + 1],
            &db->records[insert_pos],
            (size_t)(sec->count - i) * sizeof(Record));
    }

    db->records[insert_pos] = *r;
    sec->count++;
    db->modified = 1;

    return OK;
}


/* --- удаление записи  --- */

int db_delete_record(Database* db, int global_idx) {
    /* найти секцию, которой принадлежит этот индекс */
    for (int s = 0; s < db->num_sections; s++) {
        Section* sec = &db->sections[s];

        if (global_idx >= sec->start && global_idx < sec->start + sec->count) {
            int local = global_idx - sec->start;

            /* сдвигаем элементы влево */
            if (local < sec->count - 1) {
                memmove(&db->records[sec->start + local],
                    &db->records[sec->start + local + 1],
                    (size_t)(sec->count - local - 1) * sizeof(Record));
            }

            sec->count--;
            db->modified = 1;

            return OK;
        }
    }

    return ERR_NOT_FOUND;
}
