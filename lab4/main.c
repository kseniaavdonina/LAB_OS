#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// Функция для установки прав доступа на основе символьных команд
void change_mode_symbolic(const char *command, const char *filename) {
    struct stat st;

    if (stat(filename, &st) == -1) {
        perror("stat");
        exit(EXIT_FAILURE);
    }

    mode_t new_mode = st.st_mode & 0777; // Инициализация новых прав
    size_t length = strlen(command);

    printf("Текущие права: %o\n", new_mode);
    printf("Команда: %s\n", command);

    if (strcmp(command, "u-x") == 0) {
        new_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    } else if (strcmp(command, "ugo-rwx") == 0) {
        new_mode = 0; // Убираем все права
    } else {
        for (size_t i = 0; i < length; i++) {
            char who = 'a'; // По умолчанию применяем ко всем

            if (command[i] == 'u' || command[i] == 'g' || command[i] == 'o') {
                who = command[i];
                i++;
            }

            int add = 0, remove = 0;
            if (command[i] == '+') {
                add = 1;
                i++;
            } else if (command[i] == '-') {
                remove = 1;
                i++;
            }

            // Обрабатываем разрешения
            while (i < length && (command[i] == 'r' || command[i] == 'w' || command[i] == 'x')) {
                if (who == 'u' || who == 'a') {
                    if (command[i] == 'r') new_mode = add ? (new_mode | S_IRUSR) : (new_mode & ~S_IRUSR);
                    if (command[i] == 'w') new_mode = add ? (new_mode | S_IWUSR) : (new_mode & ~S_IWUSR);
                    if (command[i] == 'x') new_mode = add ? (new_mode | S_IXUSR) : (new_mode & ~S_IXUSR);
                }
                if (who == 'g' || who == 'a') {
                    if (command[i] == 'r') new_mode = add ? (new_mode | S_IRGRP) : (new_mode & ~S_IRGRP);
                    if (command[i] == 'w') new_mode = add ? (new_mode | S_IWGRP) : (new_mode & ~S_IWGRP);
                    if (command[i] == 'x') new_mode = add ? (new_mode | S_IXGRP) : (new_mode & ~S_IXGRP);
                }
                if (who == 'o' || who == 'a') {
                    if (command[i] == 'r') new_mode = add ? (new_mode | S_IROTH) : (new_mode & ~S_IROTH);
                    if (command[i] == 'w') new_mode = add ? (new_mode | S_IWOTH) : (new_mode & ~S_IWOTH);
                    if (command[i] == 'x') new_mode = add ? (new_mode | S_IXOTH) : (new_mode & ~S_IXOTH);
                }
                i++;
            }
        }
    }

    // Применяем изменения
    if (chmod(filename, new_mode) == -1) {
        perror("chmod");
        exit(EXIT_FAILURE);
    }

    printf("Права успешно изменены на: %o\n", new_mode);
}

// Функция для установки прав доступа на основе числовых команд
void change_mode_numeric(const char *command, const char *filename) {
    mode_t mode = (mode_t)strtol(command, NULL, 8);
    
    // Устанавливаем режим
    if (chmod(filename, mode) == -1) {
        perror("chmod");
        exit(EXIT_FAILURE);
    }

    printf("Права успешно изменены на: %o\n", mode);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Неверное количество аргументов.\n");
        return EXIT_FAILURE;
    }

    const char *command = argv[1];
    const char *filename = argv[2];

    int is_numeric = strspn(command, "01234567") == strlen(command);

    if (is_numeric) {
        change_mode_numeric(command, filename);
    } else {
        change_mode_symbolic(command, filename);
    }

    return 0;
}
