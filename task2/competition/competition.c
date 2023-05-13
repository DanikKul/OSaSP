#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "utils.h"

struct record_s* lastModified = NULL;
long lastReadPosition = -1;
long lastModifiedPosition = -1;

// SERVICE
int openFile(char *path) {
    int fd;
    if ((fd = open(path, O_RDWR, 0666)) == -1) {
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
int lock(int fd, long no) {
    struct flock fl;
    if (no != -2) {
        fl.l_len = sizeof(struct record_s);
        fl.l_start = no * fl.l_len;
    } else {
        fl.l_len = 0;
        fl.l_start = 0;
    }
    fl.l_whence = SEEK_SET;
    fl.l_type = F_WRLCK;
    if (fcntl(fd, F_SETLKW, &fl) == -1) {
        if (errno == EACCES || errno == EAGAIN) {
            fprintf(stderr, "[ERROR]: Can't lock write\n");
        } else {
            fprintf(stderr, "[ERROR]: Unhandled error while locking write\n");
        }
        return -1;
    }
    return 0;
}

// SERVICE
int unlock(int fd, long no) {
    struct flock fl;
    if (no != -2) {
        fl.l_len = sizeof(struct record_s);
        fl.l_start = no * fl.l_len;
    } else {
        fl.l_len = 0;
        fl.l_start = 0;
    }
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

void print_lock(int fd, long no) {
    struct flock fl;
    if (no != -2) {
        fl.l_len = sizeof(struct record_s);
        fl.l_start = no * fl.l_len;
    } else {
        fl.l_len = 0;
        fl.l_start = 0;
    }
    fl.l_whence = SEEK_SET;
    fl.l_type = F_UNLCK;
    if (fcntl(fd, F_GETLK, &fl) == -1) {
        if (errno == EACCES || errno == EAGAIN) {
            fprintf(stderr, "[ERROR]: Can't unlock\n");
        } else {
            fprintf(stderr, "[ERROR]: Unhandled error while unlocking\n");
        }
    }
    fprintf(stdout, "[STATUS]: %s, [POSITION]: %ld\n", (fl.l_type == F_WRLCK || fl.l_type == F_RDLCK) ? "LOCKED" : "UNLOCKED", no + 1);
}

// LOGIC
int isEqual (struct record_s* rec1, struct record_s* rec2) {
    if (strcmp(rec1 -> name, rec2 -> name) == 0 &&
    strcmp(rec1 -> address, rec2 -> address) == 0 &&
    rec1 -> semester == rec2 -> semester)
        return 1;
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

    struct record_s* record = (struct record_s*) malloc(sizeof(struct record_s));
    print_lock(fd, no - 1);
    lock(fd, no - 1);
    lseek(fd, (long)((no - 1) * sizeof(struct record_s)), SEEK_SET);
    read(fd, record, sizeof(struct record_s));
    unlock(fd, no - 1);
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
    fprintf(stdout, "\nENTER STUDENT'S SEMESTER\n>");
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
    print_lock(fd, lastModifiedPosition);
    while (1) {
        if (lastModifiedPosition != -1U && lastModified != NULL) {
            struct record_s* saved = getRecord(fd, lastModifiedPosition + 1);
            lock(fd, lastModifiedPosition);
            print_lock(fd, lastModifiedPosition);
            sleep(5);
            struct record_s *new = getRecord(fd, lastModifiedPosition + 1);
            if (isEqual(new, saved)) {
                lseek(fd, (long) (lastModifiedPosition * sizeof(struct record_s)), SEEK_SET);
                write(fd, lastModified, sizeof(struct record_s));
                fprintf(stdout, "\nNO: %zu\nNAME: %s\nADDRESS: %s\nSEMESTER: %u\n\n", lastModifiedPosition + 1,
                        lastModified -> name, lastModified -> address, lastModified -> semester);
                sleep(3);
                unlock(fd, lastModifiedPosition);
                break;
            } else {
                fprintf(stdout, "[WARNING]: Somebody modified record that user trying to modify. Retrying...\n");
                unlock(fd, lastModifiedPosition);
            }
        } else {
            fprintf(stdout, "[INFO]: No modified records\n");
            break;
        }
    }
    fprintf(stdout, "[INFO]: Saved\n");
}

// READONLY
void printFile(int fd) {
    lseek(fd, 0, SEEK_SET);
    fprintf(stdout, "[INFO]: PRINTING FILE\n");
    size_t size = getFileSize(fd);
    for (size_t i = 0; i < size; i++) {
        print_lock(fd, i);
        lock(fd, i);
        struct record_s* record = (struct record_s*) malloc(sizeof(struct record_s));
        read(fd, record, sizeof(struct record_s));
        fprintf(stdout, "\nNO: %zu\nNAME: %s\nADDRESS: %s\nSEMESTER: %u\n\n", i + 1, record -> name, record -> address, record -> semester);
        lastReadPosition = (long) i;
        unlock(fd, i);
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
        } else if (choice == 'm') {
            fprintf(stdout, "MENU:\n[1] - PRINT FILE\n[2] - GET RECORD\n[3] - MODIFY RECORD\n[4] - SAVE LAST MODIFIED RECORD\n[c] - CLEAR SCREEN\n[q] - EXIT\n\n");
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
    menu(fd);
    closeFile(fd);
    return 0;
}
