#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

// ANSI-коды для сброса и цветов терминала
#define RESET   "\033[0m"
#define BLUE    "\033[38;5;12m"
#define GREEN   "\033[38;5;10m"
#define CYAN    "\033[38;5;14m"

// Структура для хранения информации о файле
typedef struct file_info {
    char *name;    // Имя файла
    struct stat st;    // Статистика файла
    int is_dir;    // Флаг, указывающий, является ли файл директорией
    int is_exec;    // Флаг, указывающий, является ли файл исполняемым
    int is_link;    // Флаг, указывающий, является ли файл символьной ссылкой
} file_info_t;

// Функция сравнения для сортировки файлов по имени
int compare_files(const void *a, const void *b) {
    const file_info_t *file_a = (const file_info_t *)a;
    const file_info_t *file_b = (const file_info_t *)b;
    return strcmp(file_a->name, file_b->name);
}

// Функция для выводения информации о файле
void print_file_info(file_info_t *file, int show_hidden) {
    // Если не нужно отображать скрытые файлы и имя файла начинается с "."
    if (!show_hidden && file->name[0] == '.')
        return;

    // Если файл является символьной ссылкой
    if (file->is_link) {
        // Выводим имя ссылки в циановом цвете
        printf(CYAN "%s " RESET, file->name);
        return;
    }
    
    // Определяем цвет в зависимости от типа файла
    const char *color = RESET; // Обычный цвет

    if (file->is_dir) {
        color = BLUE;
    } else if (file->is_exec) {
        color = GREEN;
    }

    printf("%s%s%s ", color, file->name, RESET);
}

// Функция для вывода информации о файлах в длинном формате
void print_long_format(file_info_t *files, int num_files, int show_hidden, char* dir_path) {
    struct passwd *pw;
    struct group *gr;
    char time_str[100];
    long long total_blocks = 0;

    // Вычисляем общий размер всех файлов и каталогов
    for (int i = 0; i < num_files; i++) {
        file_info_t *file = &files[i];
        if (!show_hidden && file->name[0] == '.')
            continue;
        total_blocks += file->st.st_blocks;
    }

    // Выводим общее число файлов и каталогов, а также общий размер
    printf("total %lld\n", total_blocks / 2);

    // Перебираем все файлы и выводим их информацию в длинном формате
    for (int i = 0; i < num_files; i++) {
        file_info_t *file = &files[i];
        if (!show_hidden && file->name[0] == '.')
            continue;

        // Выводим тип файла
        if (file->is_link) {
            printf("l"); // Символьная ссылка
        } else {
            printf((S_ISDIR(file->st.st_mode)) ? "d" : "-"); // Директория или обычный файл
        }

        // Выводим права доступа владельца, группы и других
        printf((file->st.st_mode & S_IRUSR) ? "r" : "-");
        printf((file->st.st_mode & S_IWUSR) ? "w" : "-");
        printf((file->st.st_mode & S_IXUSR) ? "x" : "-");
        printf((file->st.st_mode & S_IRGRP) ? "r" : "-");
        printf((file->st.st_mode & S_IWGRP) ? "w" : "-");
        printf((file->st.st_mode & S_IXGRP) ? "x" : "-");
        printf((file->st.st_mode & S_IROTH) ? "r" : "-");
        printf((file->st.st_mode & S_IWOTH) ? "w" : "-");
        printf((file->st.st_mode & S_IXOTH) ? "x" : "-");

        // Выводим количество жестких ссылок
        printf(" %lu", file->st.st_nlink);

        // Выводим имя владельца и группы
        pw = getpwuid(file->st.st_uid);
        gr = getgrgid(file->st.st_gid);

        if (pw && gr) {
            printf(" %-8s %-8s", (pw) ? pw->pw_name : "", (gr) ? gr->gr_name : "");
        } else {
            printf(" %8d %8d", file->st.st_uid, file->st.st_gid);
        }
	    // Выводим размер файла
        printf(" %8lld", (long long)file->st.st_size);

        // Выводим время последнего изменения файла
        strftime(time_str, sizeof(time_str), "%b %d %H:%M", localtime(&file->st.st_mtime));
        printf(" %s ", time_str);

        // Выводим информацию о файле
        print_file_info(file, show_hidden);

        // Если файл является символьной ссылкой, выводим стрелку и путь
        if (file->is_link) {
            char full_path[PATH_MAX];
            sprintf(full_path, "%s/%s", dir_path, file->name);
            char target[1024]; // предполагаемая длина пути
            ssize_t len = readlink(full_path, target, sizeof(target) - 1);

            if (len != -1) {
                target[len] = '\0'; // Завершение строки
                //printf("%s", target);
                struct stat target_stat;
                char target_path[PATH_MAX];
                strcpy(target_path, dir_path);
                if(strcmp(dir_path, "/")) {
                    strcat(target_path, "/");
                }
                strcat(target_path, target);
                //printf("%s\n", target_path);
                if (lstat(target_path, &target_stat) == 0) {
                    if(S_ISDIR(target_stat.st_mode)) {
                        printf("-> " BLUE "%s" RESET, target);
                    } else if(target_stat.st_mode & S_IXUSR) {
                        printf("-> " GREEN "%s" RESET, target);
                    } else {
                        printf("-> %s", target);
                    }
                } else {
                    perror("Error: could not read link");
                }
            } else {
                perror("Error: could not read link");
            }
        }

        printf("\n");
    }
}


// Функция для вывода информации о файлах
void print_files(file_info_t *files, int num_files, int show_hidden, int long_format, char* dir_path) {
    // Если нужен длинный формат, вызываем print_long_format()
    if (long_format)
        print_long_format(files, num_files, show_hidden, dir_path);
        // Иначе выводим имена файлов в одну строку
    else {
        for (int i = 0; i < num_files; i++)
            print_file_info(&files[i], show_hidden);
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    int opt;
    int show_hidden = 0;
    int long_format = 0;
    char *dir_path = ".";
    DIR *dir;
    struct dirent *ent;
    file_info_t *files = NULL;
    int num_files = 0;

    // Обработка аргументов командной строки
    while ((opt = getopt(argc, argv, "al")) != -1) {
        switch (opt) {
            case 'a':
                show_hidden = 1;
                break;
            case 'l':
                long_format = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-a] [-l] [directory]\n", argv[0]);
                return 1;
        }
    }

    // Если есть аргумент, это путь к каталогу
    if (optind < argc)
        dir_path = argv[optind];

    // Открываем каталог
    dir = opendir(dir_path);
    if (dir == NULL) {
        fprintf(stderr, "Error: could not open directory '%s'\n", dir_path);
        return 1;
    }

    // Читаем содержимое каталога и собираем информацию о файлах
    while ((ent = readdir(dir)) != NULL) {
        file_info_t *file = (file_info_t *)malloc(sizeof(file_info_t));
        file->name = strdup(ent->d_name);

        char full_path[PATH_MAX];
        sprintf(full_path, "%s/%s", dir_path, ent->d_name);

        if (lstat(full_path, &file->st) == -1) {
            perror("Error: could not get file information");
            free(file->name);
            free(file);
            continue;
        }

        file->is_dir = S_ISDIR(file->st.st_mode);
        file->is_exec = file->st.st_mode & S_IXUSR;
        file->is_link = S_ISLNK(file->st.st_mode);
        files = (file_info_t *)realloc(files, (num_files + 1) * sizeof(file_info_t));
        files[num_files++] = *file;
        free(file);
    }

    // Сортируем файлы по имени
    qsort(files, num_files, sizeof(file_info_t), compare_files);

    // Выводим информацию о файлах
    print_files(files, num_files, show_hidden, long_format, dir_path);

    // Очищаем память
    for (int i = 0; i < num_files; i++)
        free(files[i].name);
    free(files);
    closedir(dir);

    return 0;
}
