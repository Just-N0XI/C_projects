#define _CRT_SECURE_NO_WARNINGS
#include "vin_search.h"

void to_upper_str(char* s) {
    if (!s) return;
    for (int i = 0; s[i]; i++)
        s[i] = (char)toupper((unsigned char)s[i]);
}

static int is_valid_vin_char(char c) {
    c = (char)toupper((unsigned char)c);
    if (c == 'I' || c == 'O' || c == 'Q') 
        return 0;
    if (c >= 'A' && c <= 'Z') 
        return 1;
    if (c >= '0' && c <= '9') 
        return 1;
    return 0;
}

int validate_vin(const char* vin) {
    if (!vin || vin[0] == '\0') {
        printf("Ошибка: VIN не может быть пустым.\n");
        return ERR_INVALID;
    }
    int len = (int)strlen(vin);
    if (len != VIN_LEN) {
        printf("Ошибка: VIN должен содержать ровно %d символов (введено %d).\n",
            VIN_LEN, len);
        return ERR_INVALID;
    }
    for (int i = 0; i < len; i++) {
        if (!is_valid_vin_char(vin[i])) {
            printf("Ошибка: символ '%c' (поз. %d) недопустим. "
                "Разрешены A-Z (кроме I,O,Q) и 0-9.\n", vin[i], i + 1);
            return ERR_INVALID;
        }
    }
    return OK;
}
