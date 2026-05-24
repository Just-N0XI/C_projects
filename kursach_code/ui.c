#define _CRT_SECURE_NO_WARNINGS
#include "vin_search.h"

void read_line(const char* prompt, char* buf, int size) {
    printf("%s", prompt);
    fflush(stdout);

    if (!fgets(buf, size, stdin)) {
        buf[0] = '\0';
        return;
    }

    int len = (int)strlen(buf);

    if (len > 0 && buf[len - 1] == '\n') {
        buf[--len] = '\0';
    }
    else {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
    }

    str_trim(buf);
}

/* цикл до тех пор, пока пользователь не введёт y/Y или n/N. */
int ask_yn(const char* question) {
    char buf[8];

    while (1) {
        read_line(question, buf, sizeof(buf));

        if (buf[0] == 'y' || buf[0] == 'Y') {
            return 1;
        }

        if (buf[0] == 'n' || buf[0] == 'N') {
            return 0;
        }

        printf("Пожалуйста, введите y или n.\n");
    }
}

/* цикл до тех пор, пока пользователь не введёт целое число в [l, r]. */
int ask_int(const char* prompt, int l, int r) {
    char buf[32];

    while (1) {
        read_line(prompt, buf, sizeof(buf));

        if (buf[0] == '\0') {
            printf("Введите число от %d до %d.\n", l, r);
            continue;
        }

        int v = atoi(buf);
        int ok = 1;

        for (int i = (buf[0] == '-' ? 1 : 0); buf[i]; i++) {
            if (!isdigit((unsigned char)buf[i])) {
                ok = 0;
                break;
            }
        }

        if (!ok || v < l || v > r) {
            printf("Введите число от %d до %d.\n", l, r);
            continue;
        }

        return v;
    }
}