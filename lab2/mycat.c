#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // Для getopt
#define MAX_LINE_LENGTH 1024

int main(int argc, char *argv[]) {
    int print_line_numbers = 0;  // Печатать номера строк
    int print_blank_lines = 0;   // Печатать пустые строки
    int print_dollar_sign = 0;   // Печать символа "$" в конце каждой строки
    FILE *file;                  // Указатель на открытый файл
    char line[MAX_LINE_LENGTH];  // Буфер для хранения текущей строки
    int line_number = 0;         // Номер текущей строки
    int non_blank_line_number = 0; // Номер непустой строки

    // Обработка флагов с помощью getopt
    int opt;
    while ((opt = getopt(argc, argv, "nbE")) != -1) {
        switch (opt) {
            case 'n':
                print_line_numbers = 1;
                break;
            case 'b':
                print_blank_lines = 1;
                break;
            case 'E':
                print_dollar_sign = 1;
                break;
            default:
                fprintf(stderr, "Error: Unknown option '%c'\n", opt);
                return 1;
        }
    }

    // Обработка оставшихся аргументов (файлов)
    for (int i = optind; i < argc; i++) {
        // Открытие файла для чтения
        file = fopen(argv[i], "r");
        if (file == NULL) {
            fprintf(stderr, "Error: Could not open file '%s'\n", argv[i]);
            return 1;
        }

        // Печать содержимого файла
        while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {
            line_number++; // Увеличиваем номер строки для каждой прочитанной строки

            // Проверяем, является ли строка пустой
            int is_blank_line = (strlen(line) == 1 && line[0] == '\n');

            // Если строка не пустая, увеличиваем номер непустой строки
            if (!is_blank_line) {
                non_blank_line_number++;
            }

            // Печатаем номер строки в зависимости от флагов
            if (print_line_numbers) {
                printf("%6d\t", line_number);
            } else if (print_blank_lines && !is_blank_line) {
                printf("%6d\t", non_blank_line_number);
            }

            // Удаляем символ новой строки для корректного вывода
            size_t len = strlen(line);
            if (len > 0 && line[len - 1] == '\n') {
                line[len - 1] = '\0'; // Удаляем символ новой строки
            }
            printf("%s", line); // Печатаем строку

            // Если установлен флаг -E, добавляем знак '$' к непустым строкам
            if (!is_blank_line && print_dollar_sign) {
                printf("$"); // Добавляем символ '$' в конце строки
            }

            // Печатаем знак '$' для пустой строки, если установлен флаг -E
            if (is_blank_line && print_dollar_sign) {
                printf("$"); // Печатаем знак '$' для пустой строки перед переходом на новую
            }

            printf("\n"); // Переход на новую строку после каждой строки
        }

        fclose(file);
    }

    return 0;
}
