#define _CRT_SECURE_NO_WARNINGS  // Отключает C4996 для strcat/strlen и т.п.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <windows.h>
#include <ctype.h>

int isVowel(char c) {
    return strchr("аеёиоуыэюяАЕЁИОУЫЭЮЯ", c) != NULL;
}

int isConsonant(char c) {
    return strchr("бвгджзйклмнпрстфхцчшщБВГДЖЗЙКЛМНПРСТФХЦЧШЩ", c) != NULL;
}

int isDigitChar(char c) {
    return isdigit((unsigned char)c);
}

int isSymbol(char c) {
    return !isVowel(c) && !isConsonant(c) && !isDigitChar(c) && c != '@';
}

int main() {
    setlocale(LC_ALL, "Russian");
    SetConsoleOutputCP(1251);

    char input[101];
    printf("Введите строку:\n");
    fgets(input, sizeof(input), stdin);

    // Удаляем '\n'
    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '\n') {
        input[len - 1] = '\0';
    }

    // Ограничение 50 символов (безопасно)
    size_t curLen = strlen(input);
    if (curLen > 50) {
        input[50] = '\0';
    }

    // Добавление подстроки
    strcat(input, "+123АБВ");

    // Создание зубчатого массива
    char** jagged = (char**)malloc(4 * sizeof(char*));
    int* sizes = (int*)calloc(4, sizeof(int));
    if (jagged == NULL || sizes == NULL) {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return 1;
    }

    for (int i = 0; i < 4; i++) {
        jagged[i] = NULL;
    }

    // Заполнение массива
    for (int i = 0; input[i] != '\0'; i++) {
        char c = input[i];
        int row = -1;

        if (isVowel(c))           row = 0;
        else if (isConsonant(c))  row = 1;
        else if (isDigitChar(c))  row = 2;
        else if (isSymbol(c))     row = 3;

        if (row == -1) continue;

        // Проверка на дубликат
        int exists = 0;
        for (int j = 0; j < sizes[row]; j++) {
            if (jagged[row][j] == c) {
                exists = 1;
                break;
            }
        }

        if (!exists) {
            sizes[row]++;
            // Безопасный realloc: сохраняем во временную переменную
            char* tmp = (char*)realloc(jagged[row], sizes[row] * sizeof(char));
            if (tmp == NULL) {
                fprintf(stderr, "Ошибка выделения памяти\n");
                // Освобождаем уже выделенное
                for (int k = 0; k < 4; k++) free(jagged[k]);
                free(jagged);
                free(sizes);
                return 1;
            }
            jagged[row] = tmp;
            jagged[row][sizes[row] - 1] = c;
        }
    }

    // Вывод массива
    printf("Массив:\n");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < sizes[i]; j++) {
            printf("%c ", jagged[i][j]);
        }
        printf("\n");
    }

    // Раскраска через ANSI
    printf("Результат:\n");
    for (int i = 0; input[i] != '\0'; i++) {
        char c = input[i];
        const char* color = "\033[37m";

        int found = 0;
        for (int j = 0; j < sizes[0] && !found; j++)
            if (jagged[0][j] == c) { color = "\033[31m"; found = 1; }
        for (int j = 0; j < sizes[1] && !found; j++)
            if (jagged[1][j] == c) { color = "\033[34m"; found = 1; }
        for (int j = 0; j < sizes[2] && !found; j++)
            if (jagged[2][j] == c) { color = "\033[32m"; found = 1; }
        for (int j = 0; j < sizes[3] && !found; j++)
            if (jagged[3][j] == c) { color = "\033[33m"; found = 1; }

        if (c == '@') color = "\033[37m";

        printf("%s%c", color, c);
    }
    printf("\033[0m\n");

    // Освобождение памяти
    for (int i = 0; i < 4; i++) {
        free(jagged[i]);
    }
    free(jagged);
    free(sizes);

    return 0;
}