#include "thread.h"

extern pthread_t threads[];
extern size_t countThreads;
pthread_barrier_t barrier_merge;
pthread_barrier_t barrier_sort;
pthread_barrier_t barrier_start;
pthread_barrier_t barrier_merge_end;
pthread_mutex_t mutex;
pthread_mutex_t merge_mutex;

index_hdr_s* indices;
pthread_t threads[8192];
size_t countThreads;
blocks_map* map;
char* ptr;
long BLOCKS;
const char PROJECT[] = "lab6";
const char GENFILES_PATH[] = "genfile/generated_files/";

void openFile(char* path, char* filename, long memsize) {

    // Generating path to file

    char* absolute_filename = (char*) malloc(250 * sizeof(char));
    for (size_t i = 0; i < strlen(path); i++) {
        absolute_filename[i] = path[i];
        if (i >= strlen(PROJECT)) {
            int k = 0;
            for (size_t j = i - strlen(PROJECT); j < i; j++, k++) {
                if (PROJECT[k] != absolute_filename[j]) {
                    break;
                }
            }
            if (k == strlen(PROJECT)) {
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

    ptr = mmap(0, memsize * sizeof(index_s) + sizeof(uint64_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    indices = (index_hdr_s*) ptr;

    close(fd);
}

void atExit(long memsize) {
    if (msync(ptr, memsize * sizeof(index_s) + sizeof(uint64_t), MS_SYNC) < 0) {
        fprintf(stderr, "[ERROR]: syncing failed\n");
        exit(-8);
    } else fprintf(stdout, "Synced\n");
    munmap(ptr, memsize * sizeof(index_s) + sizeof(uint64_t));
}


int compare(const void *a, const void *b) {
    index_s *indexA = (index_s *)a;
    index_s *indexB = (index_s *)b;
    double cmp = indexA -> time_mark - indexB -> time_mark;
    return cmp > 0 ? 1 : cmp < 0 ? -1 : 0;
}

void merge(long memsize, long blocks, long no) {
    index_s idx1[memsize / blocks], idx2[memsize / blocks];
    memcpy(idx1, indices -> idx + (memsize / blocks) * (no),  (memsize / blocks) * sizeof(index_s));
    memcpy(idx2, indices -> idx + (memsize / blocks) * (no + 1),  (memsize / blocks) * sizeof(index_s));
    size_t i = 0, j = 0, k = (memsize / blocks) * (no), n = (memsize / blocks);

    while (i < n && j < n) {
        if (idx1[i].time_mark < idx2[j].time_mark) {
            indices -> idx[k++] = idx1[i++];
        } else {
            indices -> idx[k++] = idx2[j++];
        }
    }

    while (i < n) {
        indices -> idx[k++] = idx1[i++];
    }
    while (j < n) {
        indices -> idx[k++] = idx2[j++];
    }
    fprintf(stdout, "MERGED BLOCK_%ld and BLOCK_%ld at addr1_%ld and addr2_%ld with sizes sz1_%ld and sz2_%ld\n", no, no + 1, (memsize / blocks) * (no), (memsize / blocks) * (no + 1), (memsize / blocks), (memsize / blocks));
}

void mergePhase(args* arguments) {
    while (BLOCKS > 4) {
        pthread_barrier_wait(&barrier_merge);
        if (arguments -> no == 0) {
            BLOCKS /= 2;
            for (int i = 0; i < BLOCKS; i++) {
                map[i].isBusy = 0;
            }
        }
        pthread_barrier_wait(&barrier_merge);
        for (size_t i = 0; i <= (size_t) BLOCKS; i += 2) {
            long sav = -1;
            pthread_mutex_lock(&merge_mutex);
            if (map[i].isBusy != 1) {
                map[i].isBusy = 1;
                sav = map[i].block;
            }
            pthread_mutex_unlock(&merge_mutex);
            if (sav != -1) {
                merge(arguments->memsize, BLOCKS, (long) sav);
            }
        }
        pthread_barrier_wait(&barrier_merge);
    }
    fprintf(stdout, "THREAD_%lu: ENDED MERGING\n", arguments -> no);
}

void sortPhase(args* arguments) {
    map[arguments -> no].isBusy = 1;
    fprintf(stdout, "NO: %zu OFFSET: %zu PROCESS BLOCKS: %ld\n", arguments -> no, (arguments -> memsize / arguments -> blocks) * arguments -> no, arguments -> memsize / arguments -> blocks);
    qsort(indices -> idx + ((arguments -> memsize / arguments -> blocks) * arguments -> no), arguments -> memsize / arguments -> blocks, sizeof(index_s), compare);
    for (size_t i = arguments -> no; i < (size_t) arguments -> blocks; i++) {
        if (map[i].isBusy != 1) {
            pthread_mutex_lock(&mutex);
            if (map[i].isBusy != 1) {
                map[i].isBusy = 1;
            } else {
                pthread_mutex_unlock(&mutex);
                continue;
            }
            pthread_mutex_unlock(&mutex);
            // Even if left less than (arguments -> memsize / arguments -> blocks) elements qsort will sort them without errors
            qsort(indices -> idx + ((arguments -> memsize / arguments -> blocks) * i), arguments -> memsize / arguments -> blocks, sizeof(index_s), compare);
        }
    }
}

static void* execute(args* arguments) {
    fprintf(stdout, "THREAD_%lu: STARTING...\n", arguments -> no);
    pthread_barrier_wait(&barrier_start);
    fprintf(stdout, "THREAD_%lu: START SORTING\n", arguments -> no);
    sortPhase(arguments);
    fprintf(stdout, "THREAD_%lu: WAITING OTHER THREADS\n", arguments -> no);
    pthread_barrier_wait(&barrier_sort);
    fprintf(stdout, "THREAD_%lu: START MERGING\n", arguments -> no);
    mergePhase(arguments);
    pthread_barrier_wait(&barrier_merge_end);
    fprintf(stdout, "THREAD_%lu: FINISHED WORK...\n", arguments -> no);
    return NULL;
}

void createThreads(long amount, long memsize, long blocks, char* path, char* filename) {
    pthread_barrier_init(&barrier_start, NULL, amount + 1);
    pthread_barrier_init(&barrier_sort, NULL, amount + 1);
    pthread_barrier_init(&barrier_merge_end, NULL, amount + 1);
    pthread_barrier_init(&barrier_merge, NULL, amount + 1);
    BLOCKS = 2 * blocks;
    openFile(path, filename, memsize);
    for (int i = 0; i < amount; i++) {
        args* arguments = (args*) malloc(sizeof(args));
        arguments -> addr = ptr;
        arguments -> memsize = memsize;
        arguments -> blocks = blocks;
        arguments -> no = countThreads + 1;
        int err = pthread_create(&threads[countThreads], NULL, (void *(*)(void *)) execute, arguments);
        if (err == EAGAIN) {
            fprintf(stderr, "The system lacked the necessary resources to create a thread\n");
            return;
        } else if (err == EPERM) {
            fprintf(stderr, "The caller doesn't have appropriate permission to set the required scheduling parameters or policy\n");
            return;
        } else if (err == EINVAL) {
            fprintf(stderr, "The value specified by attr is invalid\n");
            return;
        }
        countThreads++;
    }
    map = (blocks_map*) malloc(blocks * sizeof(blocks_map));
    for (int i = 0; i < blocks; i++) {
        map[i].block = i;
        if (i < amount) map[i].isBusy = 1;
        else map[i].isBusy = 0;
    }
    args* arguments = (args*) malloc(sizeof(args));
    arguments -> addr = ptr;
    arguments -> memsize = memsize;
    arguments -> blocks = blocks;
    arguments -> no = 0;
    pthread_mutex_init(&mutex, NULL);
    pthread_barrier_wait(&barrier_start);
    sortPhase(arguments);
    pthread_mutex_init(&merge_mutex, NULL);
    pthread_barrier_wait(&barrier_sort);
    fprintf(stdout, "THREAD_%lu: START MERGING\n", arguments -> no);
    mergePhase(arguments);
    merge(memsize, 2, 0);
    pthread_barrier_wait(&barrier_merge_end);
    atExit(memsize);
}

void joinThreads(int amount) {
    for (int i = 0; i < amount; i++) {
        pthread_join(threads[i], NULL);
    }
}