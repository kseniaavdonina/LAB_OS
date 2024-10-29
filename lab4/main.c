#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int is_valid_chmod_command(const char *command) {
    size_t length = strlen(command);
    for (size_t i = 0; i < length; i++) {
        if (!(command[i] == 'u' || command[i] == 'g' || command[i] == 'o' || 
              command[i] == 'a' || command[i] == '+' || command[i] == '-' || 
              command[i] == 'r' || command[i] == 'w' || command[i] == 'x')) {
            return 0;
        }
    }
    return 1;
}

void apply_symbolic(const char *command, mode_t *current_mode) {
    char who[10];
    int add = 0, remove = 0;

    int i = 0;
    while (command[i]) {
        int who_ind = 0;

        while (command[i] == 'u' || command[i] == 'g' || command[i] == 'o' || command[i] == 'a') {
            who[who_ind++] = command[i];
            i++;
        }

        if (who_ind == 0) {
            who[who_ind++] = 'a'; 
        }
        who[who_ind] = '\0';

        if (command[i] == '+' || command[i] == '-') {
            char oper = command[i];
            i++;

            while (command[i] && (command[i] == 'r' || command[i] == 'w' || command[i] == 'x')) {
                switch (command[i]) {
                    case 'r':
                        for (int j = 0; j < who_ind; j++) {
                            switch (who[j]) {
                                case 'u': add |= S_IRUSR; break;
                                case 'g': add |= S_IRGRP; break;
                                case 'o': add |= S_IROTH; break;
                                case 'a': add |= (S_IRUSR | S_IRGRP | S_IROTH); break;
                            }
                        }
                        if (oper == '-') remove |= add;
                        break;
                    case 'w':
                        for (int j = 0; j < who_ind; j++) {
                            switch (who[j]) {
                                case 'u': add |= S_IWUSR; break;
                                case 'g': add |= S_IWGRP; break;
                                case 'o': add |= S_IWOTH; break;
                                case 'a': add |= (S_IWUSR | S_IWGRP | S_IWOTH); break;
                            }
                        }
                        if (oper == '-') remove |= add;
                        break;
                    case 'x':
                        for (int j = 0; j < who_ind; j++) {
                            switch (who[j]) {
                                case 'u': add |= S_IXUSR; break;
                                case 'g': add |= S_IXGRP; break;
                                case 'o': add |= S_IXOTH; break;
                                case 'a': add |= (S_IXUSR | S_IXGRP | S_IXOTH); break;
                            }
                        }
                        if (oper == '-') remove |= add;
                        break;
                }
                i++;
            }

            if (oper == '+') {
                *current_mode |= add;
            } else if (oper == '-') {
                *current_mode &= ~remove;
            }

            add = 0;
            remove = 0;
        } else {
            i++;
        }
    }
}

// Функция для установки прав доступа на основе символьных команд
void change_mode_symbolic(const char *command, const char *filename) {
    struct stat st;

    if (stat(filename, &st) == -1) {
        perror("stat");
        exit(1);
    }

    mode_t new_mode = st.st_mode & 0777;
    apply_symbolic(command, &new_mode);

    if (chmod(filename, new_mode) == -1) {
        perror("chmod");
        exit(1);
    }
}

// Функция для установки прав доступа на основе числовых команд
void change_mode_numeric(const char *command, const char *filename) {
    mode_t mode = (mode_t)strtol(command, NULL, 8);
    
    if (chmod(filename, mode) == -1) {
        perror("chmod");
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Неверное количество аргументов\n");
        return 1;
    }

    const char *command = argv[1];
    const char *filename = argv[2];

    int is_numeric = strspn(command, "01234567") == strlen(command);

    if (is_numeric) {
        change_mode_numeric(command, filename);
    } else {
        if (!is_valid_chmod_command(command)) {
            fprintf(stderr, "Ошибка: неизвестная команда: %s\n", command);
            return 1;
        }
        change_mode_symbolic(command, filename);
    }

    return 0;
}
