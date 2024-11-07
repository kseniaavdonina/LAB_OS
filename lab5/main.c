#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_FILENAME_LEN 256

typedef struct {
    char filename[MAX_FILENAME_LEN];
    off_t size;
    mode_t mode;
} FileEntry;

void usage() {
    printf("Usage: ./archiver <archive_name> <action> [files...]\n");
    printf("Actions:\n");
    printf("  -i, --input <files>    Add files to the archive\n");
    printf("  -e, --extract <files>  Extract files from the archive\n");
    printf("  -s, --stat             Show current state of the archive\n");
    printf("  -h, --help             Show this help message\n");
}

int add_file_to_archive(const char *archive_path, const char *file_path) {
    int archive_fd = open(archive_path, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (archive_fd < 0) {
        perror("Failed to open archive for writing\n");
        return -1;
    }

    int file_fd = open(file_path, O_RDONLY);
    if (file_fd < 0) {
        perror("Failed to open file for reading\n");
        close(archive_fd);
        return -1;
    }

    struct stat file_stat;
    if (fstat(file_fd, &file_stat) < 0) {
        perror("Failed to get file stats");
        close(file_fd);
        close(archive_fd);
        return -1;
    }

    // Записываем информацию о файле в архив
    FileEntry entry;
    strncpy(entry.filename, file_path, MAX_FILENAME_LEN);
    entry.size = file_stat.st_size;
    entry.mode = file_stat.st_mode;

    if (write(archive_fd, &entry, sizeof(FileEntry)) != sizeof(FileEntry)) {
        perror("Failed to write file entry to archive\n");
        close(file_fd);
        close(archive_fd);
        return -1;
    }

    char buffer[4096];
    ssize_t bytes_read;
    while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0) {
        if (write(archive_fd, buffer, bytes_read) != bytes_read) {
            perror("Failed to write file data to archive\n");
            close(file_fd);
            close(archive_fd);
            return -1;
        }
    }

    close(file_fd);
    close(archive_fd);
    return 0;
}

int extract_file_from_archive(const char *archive_path, const char *file_name) {
    int archive_fd = open(archive_path, O_RDWR);
    if (archive_fd < 0) {
        perror("Failed to open archive for reading\n");
        return -1;
    }

    FileEntry entry;
    off_t pos = 0;
    int found = 0;

    int file_fd = -1;

    while (read(archive_fd, &entry, sizeof(FileEntry)) == sizeof(FileEntry)) {
        if (strcmp(entry.filename, file_name) == 0) {
            found = 1;
            file_fd = open(entry.filename, O_WRONLY | O_CREAT | O_TRUNC, entry.mode);
            if (file_fd < 0) {
                perror("Failed to create extracted file\n");
                close(archive_fd);
                return -1;
            }

            // Извлекаем содержимое файла из архива
            char buffer[4096];
            ssize_t bytes_read;
            off_t bytes_to_read = entry.size;

            // Положение в архиве для последующей записи
            off_t data_start_pos = pos + sizeof(FileEntry);
            lseek(archive_fd, data_start_pos, SEEK_SET);

            while (bytes_to_read > 0 && (bytes_read = read(archive_fd, buffer, sizeof(buffer))) > 0) {
                if (bytes_read > bytes_to_read) {
                    bytes_read = bytes_to_read; // Не читаем больше чем нужно
                }
                if (write(file_fd, buffer, bytes_read) != bytes_read) {
                    perror("Failed to write extracted data");
                    close(file_fd);
                    close(archive_fd);
                    return -1;
                }
                bytes_to_read -= bytes_read;
            }
            close(file_fd);
            break; // Выходим, если нашли файл
        }
        pos += sizeof(FileEntry) + entry.size; // Перейдем к следующему файлу
        lseek(archive_fd, pos, SEEK_SET); // Сдвигаем указатель в архиве
    }

    if (!found) {
        printf("File not found in archive: %s\n", file_name);
        close(archive_fd);
        return -1;
    }

    // Теперь необходимо создать новый архив без удаляемого файла
    int new_archive_fd = open("temp_archive", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (new_archive_fd < 0) {
        perror("Failed to create temporary archive\n");
        close(archive_fd);
        return -1;
    }

    // Скопировать все файлы, кроме удаляемого
    lseek(archive_fd, 0, SEEK_SET); // Вернемся в начало архива

    while (read(archive_fd, &entry, sizeof(FileEntry)) == sizeof(FileEntry)) {
        if (strcmp(entry.filename, file_name) != 0) {
            // Записываем информацию о файле в новый архив
            write(new_archive_fd, &entry, sizeof(FileEntry));

            // Копируем данные файла
            char buffer[4096];
            read(archive_fd, buffer, entry.size);
            write(new_archive_fd, buffer, entry.size);
        } else {
            // Пропускаем удаляемый файл
            lseek(archive_fd, entry.size, SEEK_CUR);
        }
    }

    close(archive_fd);
    close(new_archive_fd);

    // Переименуем новый архив до оригинального имени
    rename("temp_archive", archive_path);
    return 0;
}


int list_files_in_archive(const char *archive_path) {
    int archive_fd = open(archive_path, O_RDONLY);
    if (archive_fd < 0) {
        perror("Failed to open archive for reading\n");
        return -1;
    }

    FileEntry entry;
    printf("Files in archive '%s':\n", archive_path);
    while (read(archive_fd, &entry, sizeof(FileEntry)) == sizeof(FileEntry)) {
        printf("Name: %s, Size: %ld bytes, Mode: %o\n", entry.filename, entry.size, entry.mode);
        lseek(archive_fd, entry.size, SEEK_CUR); // Пропускаем данные файла
    }

    close(archive_fd);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        usage();
        return 1;
    }

    const char *action = argv[2];
    const char *archive_path = argv[1];
if (strcmp(action, "-i") == 0 || strcmp(action, "--input") == 0) {
        for (int i = 3; i < argc; ++i) {
            if (add_file_to_archive(archive_path, argv[i]) != 0) {
                fprintf(stderr, "Failed to add file '%s' to archive\n", argv[i]);
                return 1;
            }
        }
    } else if (strcmp(action, "-e") == 0 || strcmp(action, "--extract") == 0) {
        for (int i = 3; i < argc; ++i) {
            if (extract_file_from_archive(archive_path, argv[i]) != 0) {
                fprintf(stderr, "Failed to extract file '%s' from archive\n", argv[i]);
                return 1;
            }
        }
    } else if (strcmp(action, "-s") == 0 || strcmp(action, "--stat") == 0) {
        if (list_files_in_archive(archive_path) != 0) {
            return 1; // Ошибка при выводе состояния архива
        }
    } else if (strcmp(action, "-h") == 0 || strcmp(action, "--help") == 0) {
        usage();
    } else {
        fprintf(stderr, "Unknown action '%s'\n", action);
        usage();
        return 1;
    }

    return 0;
}
