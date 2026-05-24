#ifndef VIN_SEARCH_H
#define VIN_SEARCH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

// КОНСТАНТЫ 

#define VIN_LEN 17
#define PLATE_LEN 7
#define MAX_FIELD 64
#define MAX_PATH 512

#define NUM_SECTIONS 33      /* 26 букв (без I,O,Q) + 10 цифр */
#define SECTION_INIT 100     /* начальная ёмкость одной секции */
/* При переполнении секция расширяется вдвое; массив растёт на old_cap */

/* Порядок символов секций: A-Z (без I,O,Q), затем 0-9 */
#define SECTION_ORDER "ABCDEFGHJKLMNPRSTUVWXYZ0123456789"

// СТАТУС КОДЫ

#define OK 0			 // ошибок нет
#define ERR_FILE 1		 // ошибка открытия / чтения файла      
#define ERR_INVALID 2    // некорректный запрос                     
#define ERR_MEMORY 3     // ошибка выделения памяти     
#define ERR_NOT_FOUND 4	 // не найдена ни едмная запись

// СТРУКТУРЫ

typedef struct {
    char vin[VIN_LEN + 1];
    char plate[PLATE_LEN + 1];
    char brand[MAX_FIELD];
    char model[MAX_FIELD];
    char surname[MAX_FIELD];
    char name[MAX_FIELD];
    char patronymic[MAX_FIELD];
    char birthdate[MAX_FIELD];
} Record;

/* Описание одной секции */
typedef struct {
    char  key;      /* первый символ VIN, которому соответствует секция */
    int   start;    /* индекс первого элемента секции в главном массиве */
    int   capacity; /* выделено мест */
    int   count;    /* занято мест */
} Section;

/* База данных */
typedef struct {
    Record* records;                  /* главный массив записей */
    int total_capacity;               /* полная ёмкость records[] */
    Section sections[NUM_SECTIONS];   /* секции */
    int num_sections;                 /* всегда NUM_SECTIONS */
    char filepath[MAX_PATH];          /* путь к загруженному файлу */
    int modified;                     /* флаг несохранённых изменений */
} Database;

/* Результат поиска */
typedef struct {
    int* indices;   /* глобальные индексы в records[] */
    int  count;
} SearchResult;


// ФУНКЦИИ

/* --- Утилиты (loader.c) --- */
void safe_strncpy(char* dst, const char* src, size_t size);
void str_trim(char* s);

/* --- loader.c --- */
int  db_init(Database* db);
int  db_load(Database* db, const char* path);
void db_free(Database* db);
int  db_save(const Database* db); /* сохранить в db->filepath */

/* --- table.c --- */
int  section_index_of(const Database* db, char key);   /* найти секцию */
int  db_insert_record(Database* db, const Record* r);  /* вставить запись */
int  db_delete_record(Database* db, int global_idx);   /* удалить запись */

/* --- validator.c --- */
int  validate_vin(const char* vin); /* OK или ERR_INVALID */
void to_upper_str(char* s);

/* --- search.c --- */
int search_exact(const Database* db, const char* vin);
SearchResult search_partial(const Database* db, const char* query);
void search_result_free(SearchResult* sr);

/* --- saver.c --- */
void print_record(FILE* out, const Record* r, int num);
void print_results(const Database* db, const SearchResult* sr);
int save_results(const Database* db, const SearchResult* sr,
    const char* query, const char* out_path);

/* --- ui.c --- */
void read_line(const char* prompt, char* buf, int size);
int ask_yn(const char* question);        /* цикл до y/n, возврат 1/0   */
int ask_int(const char* prompt, int lo, int hi); /* цикл до корр. числа */

#endif
