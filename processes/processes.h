//
// Created by dan on 20.2.23.
//

#ifndef TASK_MANAGER_PROCESSES_H
#define TASK_MANAGER_PROCESSES_H

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/types.h>
#include "../structures/structures.h"

#define PROC_PATH "/proc"
#define TIME_SNAPSHOT 100
#define ONE_K 1024UL

double getUptime () {

    FILE *f = NULL;
    double uptime = 0;

    if (!(f = fopen("/proc/uptime", "r"))) return -1;

    if (!fscanf(f, "%lf", &uptime)) return -1;

    fclose(f);

    return uptime;

}

unsigned long getResidentMemory (const char *pid) {

    unsigned long resident_mem, virtual_mem;
    FILE *f = NULL;
    char *fullpath = (char*) calloc(256, sizeof(char));

    strcpy(fullpath, PROC_PATH);
    strcat(fullpath, "/");
    strcat(fullpath, pid);
    strcat(fullpath, "/");
    strcat(fullpath, "statm");

    if (!(f = fopen(fullpath, "r"))) return -1;

    if (!fscanf(f, "%lu %lu", &virtual_mem, &resident_mem)) return -1;

    fclose(f);

    return resident_mem;

}

int getThreads (processes *proc) {

    int threads = 0;
    for (int i = 0; i < proc -> n; i++) {

        char *pid = (char*) calloc(20, sizeof(char)), buffer[20];
        FILE *f = NULL;
        char *fullpath = (char*) calloc(256, sizeof(char));
        const char *token = "Threads:";

        sprintf(pid, "%d", proc -> processes[i].process_id);

        strcpy(fullpath, PROC_PATH);
        strcat(fullpath, "/");
        strcat(fullpath, pid);
        strcat(fullpath, "/");
        strcat(fullpath, "status");

        if (!(f = fopen(fullpath, "r"))) continue;

        while (fgets(buffer, sizeof(buffer), f)) {
            if (strncmp(buffer, token, strlen(token)) == 0) {

                int _threads = 0;
                sscanf(buffer, "%*s %d", &_threads);
                threads += _threads;
                break;

            }

        }

        free(fullpath);
        free(pid);
        fclose(f);
    }

    return threads;

}

avgCPUUsage *getLoadAvg () {

    FILE *f = NULL;
    avgCPUUsage *avgUsage = (avgCPUUsage*) calloc(1, sizeof(avgCPUUsage));

    if (!(f = fopen("/proc/loadavg", "r"))) return NULL;

    double avg_minute;
    double avg_5minutes;
    double avg_15minutes;

    if (!fscanf(f, "%lf", &avg_minute)) avg_minute = 0;
    if (!fscanf(f, "%lf", &avg_5minutes)) avg_5minutes = 0;
    if (!fscanf(f, "%lf", &avg_15minutes)) avg_15minutes = 0;

    avgUsage -> avg_minute = avg_minute;
    avgUsage -> avg_5minutes = avg_5minutes;
    avgUsage -> avg_15minutes = avg_15minutes;

    fclose(f);

    return avgUsage;

}

// Parses /proc/[pid]/status and gets actual process name, ppid and state
// returns pointer on structure "status"

status* parseStatus(const char* pid) {

    FILE *f = NULL;
    status* stat = (status*) calloc(1, sizeof(status));

    char *fullpath = (char*) calloc(256, sizeof(char));

    strcpy(fullpath, PROC_PATH);
    strcat(fullpath, "/");
    strcat(fullpath, pid);
    strcat(fullpath, "/");
    strcat(fullpath, "status");

    if (!(f = fopen(fullpath, "r"))) return NULL;

    char* name = (char*) calloc(100, sizeof(char));

    fgets(name, 100, f);

    char* parsedName = (char*) calloc(100, sizeof(char));

    for (size_t i = 6; i < strlen(name) - 1; i++) {

        parsedName[i - 6] = name[i];

    }

    parsedName[strlen(name) - 7] = '\0';
    stat -> name = (char*) calloc(1, strlen(parsedName) + 1);
    strcpy(stat -> name, parsedName);

    free(parsedName);
    free(name);
    fclose(f);

    if (!(f = fopen(fullpath, "r"))) return NULL;

    char buffer[10];
    const char* token = "PPid:";
    unsigned long ppid = 0;

    while (fgets(buffer, sizeof(buffer), f)) {

        if (strncmp(buffer, token, strlen(token)) == 0) {

            sscanf(buffer, "%*s %lu", &ppid);
            break;

        }

    }

    stat -> ppid = (int) ppid;
    fclose(f);

    if (!(f = fopen(fullpath, "r"))) return NULL;

    const char* token_1 = "State:";
    char state = 'U';

    while (fgets(buffer, sizeof(buffer), f)) {

        if (strncmp(buffer, token_1, strlen(token_1)) == 0) {

            sscanf(buffer, "%*s %c", &state);
            break;

        }

    }

    stat -> state = state;

    fclose(f);

    if (!(f = fopen(fullpath, "r"))) return NULL;

    const char* token_2 = "Uid:";
    int uid = 0;

    while (fgets(buffer, sizeof(buffer), f)) {

        if (strncmp(buffer, token_2, strlen(token_2)) == 0) {

            sscanf(buffer, "%*s %d", &uid);
            break;

        }

    }

    stat -> user = (char*) calloc (100, sizeof(char));
    strcpy(stat -> user, getpwuid(uid) -> pw_name);

    free(fullpath);
    fclose(f);

    return stat;

}

// Parses /proc/[pid]/stat file and gets utime, stime and starttime
// returns pointer on structure "stat"

stat* parseStat(const char* pid) {

    FILE *f = NULL;
    stat* stats = (stat*) calloc(1, sizeof(stat));
    char *fullpath = (char*) calloc(256, sizeof(char));

    strcpy(fullpath, PROC_PATH);
    strcat(fullpath, "/");
    strcat(fullpath, pid);
    strcat(fullpath, "/");
    strcat(fullpath, "stat");

    if (!(f = fopen(fullpath, "r"))) return NULL;

    char* name = (char*) calloc(100, sizeof(char));
    unsigned long utime_ticks = 1,
    stime_ticks = 1,
    cutime_ticks = 1,
    cstime_ticks = 1,
    starttime_ticks = 1;

    for (int i = 0; i < 25; i++) {

        fscanf(f, "%s", name);

        char *end;

        if (i == 13) utime_ticks = strtol(name, &end, 10);
        else if (i == 14) stime_ticks = strtol(name, &end, 10);
        else if (i == 15) cutime_ticks = strtol(name, &end, 10);
        else if (i == 16) cstime_ticks = strtol(name, &end, 10);
        else if (i == 21) starttime_ticks = strtol(name, &end, 10);

    }

    stats -> stime = (double) stime_ticks;
    stats -> utime = (double) utime_ticks;
    stats -> cutime = (double) cutime_ticks;
    stats -> cstime = (double) cstime_ticks;
    stats -> starttime = (double) starttime_ticks;

    free(fullpath);
    free(name);
    fclose(f);

    return stats;

}

double calculateProcessCPUUsage(stat *stats) {

    double totalTime = stats -> utime + stats -> stime;
    totalTime += stats -> cutime + stats -> cstime;
    double SECONDS_PER_CLOCKS = (double) sysconf(_SC_CLK_TCK);
    double seconds = getUptime() - (stats -> starttime / SECONDS_PER_CLOCKS);

    return 100 * ((totalTime / SECONDS_PER_CLOCKS) / seconds);

}

double calculateProcessMemoryUsage (double residentMem, unsigned long totalMemory) {

    unsigned PAGESIZE = sysconf(_SC_PAGESIZE);
    double PAGESIZE_KB = (double) PAGESIZE / ONE_K;


    residentMem *= PAGESIZE_KB;

    return residentMem / (double) totalMemory * 100.;

}

double calculateMemoryUsage (memory *mem) {

    if (mem == NULL) return 0;

    return (double)(mem -> total - mem -> available) / (double) mem -> total * 100.;

}

char* parseCommand(const char* pid) {

    FILE *f = NULL;
    char *command = (char*) calloc(128, sizeof(char));
    char *fullpath = (char*) calloc(256, sizeof(char));

    strcpy(fullpath, PROC_PATH);
    strcat(fullpath, "/");
    strcat(fullpath, pid);
    strcat(fullpath, "/");
    strcat(fullpath, "cmdline");

    if (!(f = fopen(fullpath, "r"))) return NULL;

    fgets(command, 128, f);

    for (int i = 0; i < strlen(command); i++) {

        if (command[i] > 240 || command[i] < 32) {

            command = (char *) calloc(128, sizeof(char));

            strcpy(command, " ");
            break;

        }

    }

    for (int i = 0; i < strlen(command); i++) {

        if (command[i] == '\n') command[i] = ' ';

    }

    command[strlen(command)] = '\0';

    free(fullpath);
    fclose(f);

    return command;

}

time *processUptime(double up) {

    int uptime = (int) up;
    time *t = (time*) calloc(1, sizeof(time));

    t -> hours = (int) uptime / 3600;
    uptime = uptime % 3600;
    t -> minutes = (int) uptime / 60;
    uptime = uptime % 60;
    t -> seconds = uptime;

    return t;

}

// Prints all processes in stdout

void printProcesses (processes *proc) {

    printf("| %-25s | %-8s | %-8s | %-8s | %-8s | %-8s | %-40s | %-70s |\n",
           "USER",
           "PID",
           "PPID",
           "STATE",
           "CPU",
           "MEM",
           "NAME",
           "COMMAND");

    for (int i = 0; i < proc -> n; i++) {

        printf("| %-25s | %-8d | %-8d | %-8c | %-5.2lf%%%-2s | %-5.2lf%%%-2s | %-40s | %-70s |\n",
               proc -> processes[i].process_user,
               proc -> processes[i].process_id,
               proc -> processes[i].process_ppid,
               proc -> processes[i].process_state,
               proc -> processes[i].cpu_usage,
               "",
               proc -> processes[i].mem_usage,
               "",
               proc -> processes[i].process_name,
               proc -> processes[i].process_command);

    }

}

// Prints info about memory

void printMemory (memory *mem) {

    printf("MEMORY: TOTAL: %lukB FREE: %lukB AVAILABLE: %lukB PERCENTAGE: %lf%%\n",
           mem -> total,
           mem -> free,
           mem -> available,
           (double)(mem -> total - mem -> available) / (double) mem -> total * 100.);

}

void printCPU (cpuinfo *CPU) {

    printf("CPU USAGE: %lf %%\n", CPU -> usage);

}

void printLoadAvg (avgCPUUsage *avgUsage) {

    printf("LOAD_AVG: %lf, %lf, %lf\n",
           avgUsage -> avg_minute,
           avgUsage -> avg_5minutes,
           avgUsage -> avg_15minutes);

}

void printUptime(double uptime) {

    time *up = processUptime(uptime);

    printf("UPTIME: %02d:%02d:%02d\n",
           up -> hours,
           up -> minutes,
           up -> seconds);

}

void printThreads (int threads) {

    printf("THREADS: %d\n", threads);

}

// Gets actual info about memory
// returns pointer on structure "memory"

memory* getMemory() {

    FILE *f = NULL;
    memory *mem = (memory*) calloc(1, sizeof(memory));
    char buffer[512];

    if (!(f = fopen("/proc/meminfo", "r"))) return NULL;

    const char* token_1 = "MemTotal:";
    unsigned long total = 0;

    while (fgets(buffer, sizeof(buffer), f)) {

        if (strncmp(buffer, token_1, strlen(token_1)) == 0) {

            sscanf(buffer, "%*s %lu", &total);
            break;

        }

    }

    const char* token_2 = "MemFree:";
    unsigned long free = 0;

    while (fgets(buffer, sizeof(buffer), f)) {

        if (strncmp(buffer, token_2, strlen(token_2)) == 0) {

            sscanf(buffer, "%*s %lu", &free);
            break;

        }

    }

    const char* token_3 = "MemAvailable:";
    unsigned long available = 0;

    while (fgets(buffer, sizeof(buffer), f)) {

        if (strncmp(buffer, token_3, strlen(token_3)) == 0) {

            sscanf(buffer, "%*s %lu", &available);
            break;

        }

    }

    mem -> total = total;
    mem -> free = free;
    mem -> available = available;

    fclose(f);

    return mem;

}

// Main function that construct processes using parsers
// returns pointer on structure "processes"

processes* getProcesses () {

    struct dirent **nameList;
    int n = scandir(PROC_PATH, &nameList, 0, NULL);
    int process_amount = 0;
    process *_processes = (process*) calloc(n, sizeof(process));

    for (int i = 0; i < n; i++) {

        if (strcmp(nameList[i] -> d_name, ".") != 0 && strcmp(nameList[i] -> d_name, "..") != 0) {

            char *end;
            int pid = (int) strtol(nameList[i] -> d_name, &end, 10);

            if (pid == 0) continue;

            _processes[process_amount].process_id = pid;
            status *_status = parseStatus(nameList[i] -> d_name);
            stat *_stat = parseStat(nameList[i] -> d_name);
            if (_stat == NULL || _status == NULL) continue;
            strcpy(_processes[process_amount].process_name, _status -> name);
            strcpy(_processes[process_amount].process_user, _status -> user);
            _processes[process_amount].process_ppid = _status -> ppid;
            _processes[process_amount].process_state = _status -> state;
            char *command= parseCommand(nameList[i] -> d_name);
            if (command) {
                strcpy(_processes[process_amount].process_command, parseCommand(nameList[i]->d_name));
            }
            _processes[process_amount].cpu_usage = calculateProcessCPUUsage(_stat);
            _processes[process_amount].mem_usage = calculateProcessMemoryUsage(
                    (double) getResidentMemory(nameList[i] -> d_name), getMemory() -> total);
            process_amount++;

        }

    }

    processes *proc = (processes*) calloc(1, sizeof(processes));

    proc -> processes = _processes;
    proc -> n = process_amount;

    return proc;
}

cpu *parseCpu() {

    cpu *CPU = (cpu*) calloc (1, sizeof(cpu));
    FILE *f = NULL;
    char *buf = (char*) calloc(128, sizeof(char));
    int total_1 = 0,
    running_1 = 0,
    buff = 0;

    if (!(f = fopen("/proc/stat", "r"))) return NULL;

    fscanf(f, "%s", buf);

    for (int i = 0; i < 4; i++) {

        fscanf(f, "%d", &buff);
        total_1 += buff;

        if (i == 3) continue;

        running_1 += buff;

    }

    CPU -> running = (double) running_1;
    CPU -> total = (double) total_1;

    fclose(f);

    return CPU;

}

cpuinfo *getCpu () {

    cpuinfo *CPU = (cpuinfo*) calloc(1, sizeof(cpuinfo));
    cpu *CPU1 = parseCpu();
    usleep(TIME_SNAPSHOT * 1000);
    cpu *CPU2 = parseCpu();

    if (CPU1 == NULL || CPU2 == NULL) return NULL;

    CPU -> snapshot1 = CPU1;
    CPU -> snapshot2 = CPU2;
    if ((double)(CPU -> snapshot2 -> total - CPU -> snapshot1 -> total) == 0) return 0;
    CPU -> usage = (double)(CPU -> snapshot2 -> running - CPU -> snapshot1 -> running) / (double)(CPU -> snapshot2 -> total - CPU -> snapshot1 -> total) * 100;
    return CPU;

}

int getRunningProcesses(processes *proc) {

    int n = 0;

    for (int i = 0; i < proc -> n; i++) if (proc -> processes[i].process_state == 'R') n++;

    return n;

}

#endif //TASK_MANAGER_PROCESSES_H
