#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "utils.h"

struct record_s* lastModified = NULL;
long lastReadPosition = -1;
long lastModifiedPosition = -1;

// SERVICE
int openFile(char *path) {
    int fd;
    if ((fd = open(path, O_RDWR)) == -1) {
        fprintf(stderr, "[ERROR]: Can't open file\n");
        exit(-1);
    }
    return fd;
}

// SERVICE
void closeFile(int fd) {
    close(fd);
}

// SERVICE
int lock_read(int fd, long no) {
    struct flock fl;
    fl.l_len = sizeof(struct record_s);
    fl.l_start = no * fl.l_len;
    fl.l_whence = SEEK_SET;
    fl.l_type = F_RDLCK;
    fl.l_pid = getpid();
    if (fcntl(fd, F_SETLK, &fl) == -1) {
        if (errno == EACCES || errno == EAGAIN) {
            fprintf(stderr, "[ERROR]: Can't unlock\n");
        } else {
            fprintf(stderr, "[ERROR]: Unhandled error while unlocking\n");
        }
        return -1;
    }
    fprintf(stdout, "%ld %d %d %ld %d\n\n", fl.l_len, fl.l_type, fl.l_whence, fl.l_start, fl.l_pid);
    return 0;
}

// SERVICE
int lock_write(int fd, long no) {
    struct flock fl;
    fl.l_len = sizeof(struct record_s);
    fl.l_start = no * fl.l_len;
    fl.l_whence = SEEK_SET;
    fl.l_type = F_WRLCK;
    if (fcntl(fd, F_SETLK, &fl) == -1) {
        if (errno == EACCES || errno == EAGAIN) {
            fprintf(stderr, "[ERROR]: Can't unlock\n");

        } else {
            fprintf(stderr, "[ERROR]: Unhandled error while unlocking\n");
        }
        return -1;
    }
    return 0;
}

void print_lock(int fd, long no) {
    struct flock fl;
    fl.l_len = sizeof(struct record_s);
    fl.l_start = no * fl.l_len;
    fl.l_whence = SEEK_SET;
    fl.l_type = F_WRLCK;
    fl.l_pid = getpid();
    if (fcntl(fd, F_GETLK, &fl) == -1) {
        if (errno == EACCES || errno == EAGAIN) {
            fprintf(stderr, "[ERROR]: Can't unlock\n");
        } else {
            fprintf(stderr, "[ERROR]: Unhandled error while unlocking\n");
        }
    }
    fprintf(stdout, "%ld %d %d %ld %d\n\n", fl.l_len, fl.l_type, fl.l_whence, fl.l_start, fl.l_pid);
}

// SERVICE
int unlock(int fd, long no) {
    struct flock fl;
    fl.l_len = sizeof(struct record_s);
    fl.l_start = no * fl.l_len;
    fl.l_whence = SEEK_SET;
    fl.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &fl) == -1) {
        if (errno == EACCES || errno == EAGAIN) {
            fprintf(stderr, "[ERROR]: Can't unlock\n");

        } else {
            fprintf(stderr, "[ERROR]: Unhandled error while unlocking\n");
        }
        return -1;
    }
    return 0;
}

// READONLY
size_t getFileSize(int fd) {
    size_t old_position = lseek(fd, 0, SEEK_SET);
    size_t size_in_bytes = lseek(fd, 0, SEEK_END);
    lseek(fd, (long) old_position, SEEK_SET);
    return size_in_bytes / sizeof(struct record_s);
}

// READONLY
struct record_s* getRecord(int fd, long no) {
    lseek(fd, 0, SEEK_SET);
    size_t size = getFileSize(fd);

    if (no - 1 >= (long) size) {
        fprintf(stderr, "[WARNING]: Number of the record is greater than amount of records in the file\n");
        return NULL;
    } else if (no <= 0) {
        fprintf(stderr, "[WARNING]: Number of the record is less or equal zero\n");
        return NULL;
    }

    fprintf(stdout, "SEARCHING RECORD %zu %zu\n", size, no);
    struct record_s* record = (struct record_s*) malloc(sizeof(struct record_s));
    lseek(fd, (long)((no - 1) * sizeof(struct record_s)), SEEK_SET);
    read(fd, record, sizeof(struct record_s));

    lastReadPosition = (long) no - 1;

    return record;
}

// LOGIC
void modifyRecord(int fd, long no) {
    struct record_s* record = getRecord(fd, no);
    if (record == NULL) {
        fprintf(stdout, "[WARNING]: Can't modify nonexistent record\n");
        return;
    }
    fprintf(stdout, "[INFO]: MODIFYING RECORD\n");
    lseek(fd, 0, SEEK_SET);
    fprintf(stdout, "\nENTER STUDENT'S NAME\n>");
    char* tmp = (char*) malloc(20 * sizeof(char));
    getchar();
    fflush(stdin);
    fseek(stdin, 0, SEEK_SET);
    fgets(record -> name, 80, stdin);
    fprintf(stdout, "\nENTER STUDENT'S ADDRESS\n>");
    fflush(stdin);
    fseek(stdin, 0, SEEK_SET);
    fgets(record -> address, 150, stdin);
    fprintf(stdout, "\nENTER STUDENT'S ADDRESS\n>");
    fflush(stdin);
    fseek(stdin, 0, SEEK_SET);
    fgets(tmp, 20, stdin);
    record -> semester = strtol(tmp, NULL, 10);
    fprintf(stdout, "\n");
    record -> name[strlen(record -> name) - 1] = '\0';
    record -> address[strlen(record -> address) - 1] = '\0';
    lastModified = record;
    lastModifiedPosition = lastReadPosition;
}

// WRITEONLY
void putFile(int fd) {
    fprintf(stdout, "[INFO]: SAVING LAST MODIFIED RECORD\n");
    if (lastModifiedPosition != -1U && lastModified != NULL) {
        lseek(fd, (long) (lastModifiedPosition * sizeof(struct record_s)), SEEK_SET);
        write(fd, lastModified, sizeof(struct record_s));
        fprintf(stdout, "\nNO: %zu\nNAME: %s\nADDRESS: %s\nSEMESTER: %u\n\n", lastModifiedPosition + 1,
                lastModified -> name, lastModified -> address, lastModified -> semester);
    } else {
        fprintf(stdout, "[INFO]: No modified records\n");
    }
}

// READONLY
void printFile(int fd) {
    lseek(fd, 0, SEEK_SET);
    fprintf(stdout, "[INFO]: PRINTING FILE\n");
    size_t size = getFileSize(fd);
    for (size_t i = 0; i < size; i++) {
        struct record_s* record = (struct record_s*) malloc(sizeof(struct record_s));
        read(fd, record, sizeof(struct record_s));
        fprintf(stdout, "\nNO: %zu\nNAME: %s\nADDRESS: %s\nSEMESTER: %u\n\n", i + 1, record -> name, record -> address, record -> semester);
        lastReadPosition = (long) i;
    }
}

void menu(int fd) {
    char choice;
    fprintf(stdout, "MENU:\n[1] - PRINT FILE\n[2] - GET RECORD\n[3] - MODIFY RECORD\n[4] - SAVE LAST MODIFIED RECORD\n[c] - CLEAR SCREEN\n[q] - EXIT\n\n");
    while (1) {
        fscanf(stdin, "%c", &choice);
        if (choice == '1') {
            printFile(fd);
        } else if (choice == '2') {
            int no = 0;
            fprintf(stdout, "ENTER NUMBER OF THE RECORD\n");
            fscanf(stdin, "%d", &no);
            struct record_s* record = getRecord(fd, no);
            if (record != NULL)
                fprintf(stdout, "\nNO: %d\nNAME: %s\nADDRESS: %s\nSEMESTER: %u\n\n", no, record -> name, record -> address, record -> semester);
            free(record);
        } else if (choice == '3') {
            int no = 0;
            fprintf(stdout, "ENTER NUMBER OF THE RECORD\n");
            fscanf(stdin, "%d", &no);
            modifyRecord(fd, no);
        } else if (choice == '4') {
            putFile(fd);
        } else if (choice == 'q') {
            fprintf(stdout, "EXITTING...\n");
            break;
        } else if (choice == 'c') {
            system("clear");
        }
    }
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "[ERROR]: No path to file provided\n");
        fprintf(stderr, "\nRun program like:\n\n./competition /path/to/file.txt\n\n");
        return -1;
    }

    if (argc > 2) {
        fprintf(stderr, "[ERROR]: Too many arguments provided\n");
        fprintf(stderr, "\nRun program like:\n\n./competition /path/to/file.txt\n\n");
        return -1;
    }

    char *path = (char *) malloc(sizeof(char) * (strlen(argv[1]) + 1));
    strncpy(path, argv[1], strlen(argv[1]) * sizeof(char));
    path[strlen(argv[1]) * sizeof(char)] = '\0';
    int fd = openFile(path);
//    menu(fd);
    lock_read(fd, 1);
    lock_write(fd, 1);
    print_lock(fd, 1);
    printFile(fd);
//    struct record_s* record = getRecord(fd, 1);
//    fprintf(stdout, "\nNAME: %s\nADDRESS: %s\nSEMESTER: %u\n\n", record -> name, record -> address, record -> semester);
    sleep(20);
    closeFile(fd);

    return 0;
}
