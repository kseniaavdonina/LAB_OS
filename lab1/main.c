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
#define BLUE    "\033[34m"
#define GREEN   "\033[32m"
#define CYAN    "\033[36m"

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

// Функция для вывода информации о файле
void print_file_info(file_info_t *file, int show_hidden) {
    // Если не нужно отображать скрытые файлы и имя файла начинается с "."
    if (!show_hidden && file->name[0] == '.')
        return;
    // Если файл является директорией, выводим его имя синим цветом
    if (file->is_dir)
        printf(BLUE "%s" RESET " ", file->name);
        // Если файл является исполняемым, выводим его имя зеленым цветом
    else if (file->is_exec)
        printf(GREEN "%s" RESET " ", file->name);
        // Если файл является символьной ссылкой, выводим его имя циановым цветом
    else if (file->is_link)
        printf(CYAN "%s" RESET " ", file->name);
        // Иначе выводим имя файла без цвета
    else
        printf("%s ", file->name);
}

// Функция для вывода информации о файлах в длинном формате
void print_long_format(file_info_t *files, int num_files, int show_hidden) {
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
    printf("total %lld\n", total_blocks/2);

    // Перебираем все файлы и выводим их информацию в длинном формате
    for (int i = 0; i < num_files; i++) {
        file_info_t *file = &files[i];
        if (!show_hidden && file->name[0] == '.')
            continue;

        // Выводим тип файла (директория или обычный файл)
        printf((S_ISDIR(file->st.st_mode)) ? "d" : "-");

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
        printf(" %-8s %-8s", (pw) ? pw->pw_name : "", (gr) ? gr->gr_name : "");

        // Выводим размер файла
        printf(" %5lld", (long long)file->st.st_size);

        // Выводим время последнего изменения файла
        strftime(time_str, sizeof(time_str), "%b %d %H:%M", localtime(&file->st.st_mtime));
        printf(" %s ", time_str);

        // Выводим имя файла
        print_file_info(file, show_hidden);
        printf("\n");
    }
}

// Функция для вывода информации о файлах
void print_files(file_info_t *files, int num_files, int show_hidden, int long_format) {
    // Если нужен длинный формат, вызываем print_long_format()
    if (long_format)
        print_long_format(files, num_files, show_hidden);
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

        // Используйте fstatat() вместо stat()
        if (fstatat(dirfd(dir), ent->d_name, &file->st, 0) == -1) {
            fprintf(stderr, "Error: could not get file information for '%s'\n", ent->d_name);
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
    print_files(files, num_files, show_hidden, long_format);

    // Очищаем память
    for (int i = 0; i < num_files; i++)
        free(files[i].name);
    free(files);
    closedir(dir);

    return 0;
}
