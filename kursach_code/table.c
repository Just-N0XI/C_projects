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

static int overflow_section(Database* db, int sx) {
    Section* secX = &db->sections[sx];
    int old_cap = secX->capacity;
    int old_count = secX->count;
    int new_cap = old_cap * 2;

    if (sx == db->num_sections - 1) {
        int new_total = db->total_capacity + old_cap;
        Record* new_rec = (Record*)realloc(db->records, (size_t)new_total * sizeof(Record));
        if (!new_rec) return ERR_MEMORY;

        db->records = new_rec;
        db->total_capacity = new_total;
        secX->capacity = new_cap;
        return OK;
    }

    /* если секци€ не последн€€, выдел€ем место дл€ нее в конце общего буфера */
    int new_total = db->total_capacity + new_cap;
    Record* new_rec = (Record*)realloc(db->records, (size_t)new_total * sizeof(Record));

    if (!new_rec) {
        return ERR_MEMORY;
    }

    db->records = new_rec;

    secX = &db->sections[sx];

    int old_start_x = secX->start;
    int new_start_x = db->total_capacity; 

    memcpy(&db->records[new_start_x], &db->records[old_start_x], (size_t)old_count * sizeof(Record));

    Section* secY = &db->sections[sx + 1];

    if (secY->count > 0) {
        memmove(&db->records[old_start_x],
            &db->records[secY->start],
            (size_t)secY->count * sizeof(Record));
    }

    secY->start = old_start_x;
    secY->capacity += old_cap;

    secX->start = new_start_x;
    secX->capacity = new_cap;

    db->total_capacity = new_total;

    return OK;
}

/* --- вставка записи в нужную секцию (с сортировкой) --- */

int db_insert_record(Database* db, const Record* r) {
 
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
