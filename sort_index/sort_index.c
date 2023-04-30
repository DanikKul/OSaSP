#include "sort_index.h"
#include "thread.h"
#include "utils.h"

const char PROJECT[] = "OSaSP";
const char GENFILES_PATH[] = "genfile/generated_files/";

index_hdr_s* indices;
pthread_t threads[8192];
size_t countThreads;
char* ptr;

int compare(const void *a, const void *b) {
    index_s *indexA = (index_s *)a;
    index_s *indexB = (index_s *)b;
    double cmp = indexB -> time_mark - indexA -> time_mark;
    return cmp > 0 ? 1 : cmp < 0 ? -1 : 0;
}


void openFile(char* path, char* filename) {

    // Generating path to file

    char* absolute_filename = (char*) malloc(250 * sizeof(char));
    for (size_t i = 0; i < strlen(path); i++) {
        absolute_filename[i] = path[i];
        if (i >= 5) {
            int k = 0;
            for (size_t j = i - 5; j < i; j++, k++) {
                if (PROJECT[k] != absolute_filename[j]) {
                    break;
                }
            }
            if (k == 5) {
                break;
            }
        }
    }
    strcat(absolute_filename, GENFILES_PATH);
    strcat(absolute_filename, filename);

    // Opening file

    int fd;
    if (!(fd = open(absolute_filename, O_RDWR))) {
        fprintf(stderr, "Can't open file\n");
        exit(-7);
    }

    // Mapping file into virtual memory

    ptr = mmap(0, sizeof(index_hdr_s), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    close(fd);
}

void atExit() {
    if (msync(ptr, sizeof(index_hdr_s), MS_SYNC) < 0) {
        fprintf(stderr, "[ERROR]: syncing failed\n");
        exit(-8);
    } else fprintf(stdout, "Synced\n");
    munmap(ptr, sizeof(index_hdr_s));
}

int main(int argc, char* argv[]) {

    struct rlimit limit;
    getrlimit(RLIMIT_STACK, &limit);
    limit.rlim_cur = limit.rlim_max;
    setrlimit(RLIMIT_STACK, &limit);

    // Check args

    if (argc < 4) {
        fprintf(stderr, "[ERROR]: Not enough arguments, please run program with memory size, blocks amount, threads amount and filename\n");
        return -1;
    }

    long memsize = strtol(argv[1], NULL, 10);
    if (memsize == 0) {
        fprintf(stderr, "[ERROR]: Memory size is zero or not a number\n");
        return -2;
    }
    if (memsize % getpagesize() != 0) {
        fprintf(stderr, "[ERROR]: Memory size is not multiple of %d\n", getpagesize());
        return -3;
    }
    long blocks = strtol(argv[2], NULL, 10);
    if (blocks == 0) {
        fprintf(stderr, "[ERROR]: Blocks amount is zero or not a number\n");
        return -2;
    }
    long tmp = blocks;
    while (tmp != 1) {
        if (tmp % 2 != 0) {
            fprintf(stderr, "[ERROR]: Blocks amount is not power of 2\n");
            return -4;
        }
        tmp /= 2;
    }
    long _threads = strtol(argv[3], NULL, 10);
    if (_threads == 0) {
        fprintf(stderr, "[ERROR]: Threads amount is zero or not a number\n");
        return -2;
    }
    if (_threads < MIN_THREADS) {
        fprintf(stderr, "[ERROR]: Threads amount is less than amount of kernels\n");
        return -5;
    }
    if (_threads > MAX_THREADS) {
        fprintf(stderr, "[ERROR]: Threads amount is greater than max amount of threads\n");
        return -5;
    }
    if (_threads >= blocks) {
        fprintf(stderr, "[ERROR]: Threads amount is greater than blocks amount\n");
        return -6;
    }
    openFile(argv[0], argv[4]);
    createThreads(_threads, ptr, memsize, blocks);
    sleep(1);
    joinThreads();
    pthread_barrier_init
    return 0;
}