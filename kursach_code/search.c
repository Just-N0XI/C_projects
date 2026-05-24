#define _CRT_SECURE_NO_WARNINGS
#include "vin_search.h"

/* --- точный поиск --- */

static int interpolate(int lo, int hi, const char* key, const char* lo_val, const char* hi_val) {
    int d = 0;

    while (lo_val[d] && hi_val[d] && lo_val[d] == hi_val[d]) {
        d++;
    }

    unsigned char kc = (unsigned char)key[d];
    unsigned char lc = (unsigned char)lo_val[d];
    unsigned char hc = (unsigned char)hi_val[d];

    if (hc == lc) {
        return lo;
    }

    int pos = lo + (int)(((double)(kc - lc) / (double)(hc - lc)) * (hi - lo));

    if (pos < lo) {
        pos = lo;
    }

    if (pos > hi) {
        pos = hi;
    }

    return pos;
}

int search_exact(const Database* db, const char* vin) {
    if (!db || !vin || vin[0] == '\0') {
        return -1;
    }

    int sx = section_index_of(db, vin[0]);

    if (sx < 0) {
        return -1;
    }

    const Section* sec = &db->sections[sx];

    if (sec->count == 0) {
        return -1;
    }

    int lo = 0, hi = sec->count - 1;

    while (lo <= hi) {
        const char* lo_vin = db->records[sec->start + lo].vin;
        const char* hi_vin = db->records[sec->start + hi].vin;

        if (strcmp(vin, lo_vin) < 0 || strcmp(vin, hi_vin) > 0) {
            break;
        }

        int pos = interpolate(lo, hi, vin, lo_vin, hi_vin);
        int cmp = strcmp(vin, db->records[sec->start + pos].vin);

        if (cmp == 0) {
            return sec->start + pos; /* глобальный индекс */
        }
        else if (cmp > 0) {
            lo = pos + 1;
        }
        else {
            hi = pos - 1;
        }
    }

    return -1;
}

/* --- частичный поиск --- */

SearchResult search_partial(const Database* db, const char* query) {
    SearchResult sr = { NULL, 0 };

    if (!db || !query || query[0] == '\0') {
        return sr;
    }

    /* выделяем максимально возможное число результатов */
    int total = 0;

    for (int s = 0; s < db->num_sections; s++) {
        total += db->sections[s].count;
    }

    if (total == 0) {
        return sr;
    }

    sr.indices = (int*)malloc((size_t)total * sizeof(int));

    if (!sr.indices) {
        return sr;
    }

    /*
     * частичный поиск подразумевает, что пользователь начнет 
     * вводить именно начало строки
     */
    int sx = section_index_of(db, query[0]);
    int s_from = (sx >= 0) ? sx : 0;
    int s_to = (sx >= 0) ? sx + 1 : db->num_sections;

    for (int s = s_from; s < s_to; s++) {
        const Section* sec = &db->sections[s];

        for (int i = 0; i < sec->count; i++) {
            const Record* r = &db->records[sec->start + i];

            if (strstr(r->vin, query) != NULL) {
                sr.indices[sr.count++] = sec->start + i;
            }
        }
    }

    return sr;
}

void search_result_free(SearchResult* sr) {
    if (sr && sr->indices) {
        free(sr->indices);
        sr->indices = NULL;
        sr->count = 0;
    }
}