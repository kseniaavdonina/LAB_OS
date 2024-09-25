#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <sys/stat.h>

#define MAX_LINE_LENGTH 1024

void grep(FILE* buf, const char* pattern, int isMulty, const char* fileName) {
    regex_t regexp;
    if (regcomp(&regexp, pattern, 0) != 0) {
        fprintf(stderr, "grep: Unmatched pattern\n");
        exit(2);
    }

    char line[MAX_LINE_LENGTH];
    
    while (fgets(line, MAX_LINE_LENGTH, buf) != NULL) {
        regmatch_t match;

        // Проверяем, найдется ли шаблон в строке
        if (regexec(&regexp, line, 0, NULL, 0) == 0) {
            // Если выводим из файлов и их несколько, указываем имя файла
            if (isMulty) {
                printf("\x1b[35m%s\x1b[34m:\x1b[0m ", fileName);
            }

            // Подсвечиваем совпадающую часть
            char* cursor = line;
            int length = strlen(line);
            while (regexec(&regexp, cursor, 1, &match, 0) == 0) {
                // Выводим часть строки до совпадения
                fwrite(cursor, sizeof(char), match.rm_so, stdout);
                // Подсвечиваем совпадение
                printf("\x1b[1;31m"); // Красный цвет
                fwrite(cursor + match.rm_so, sizeof(char), match.rm_eo - match.rm_so, stdout);
                printf("\x1b[0m"); // Сброс цвета
                // Переходим к части строки после совпадения
                cursor += match.rm_eo;
            }
            // Выводим оставшуюся часть строки после последнего совпадения
            printf("%s", cursor);
        }
    }
    regfree(&regexp);
}

int isDir(const char* filePath) {
    struct stat st;
    stat(filePath, &st); //Получение информации о файле или каталоге
    return S_ISDIR(st.st_mode); //Проверка, является ли директорией
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s pattern [file1 file2 ...]\n", argv[0]);
        exit(2);
    }

    const char* pattern = argv[1]; // Шаблон для поиска

    // Если передано менее двух аргументов, используем стандартный ввод
    if (argc == 2) {
        grep(stdin, pattern, 0, NULL);
    } else {
        for (int i = 2; i < argc; ++i) {
            const char* filename = argv[i];
            if (isDir(filename)) {
                fprintf(stderr, "%s: '%s': Is a directory\n", argv[0], filename);
                continue;
            }

            FILE* file = fopen(filename, "r");
            if (file == NULL) {
                fprintf(stderr, "Error: Could not open file '%s'\n", filename);
                continue; // Переходим к следующему файлу
            }

            grep(file, pattern, argc > 3, filename);

            fclose(file);
        }
    }

    return 0;
}
