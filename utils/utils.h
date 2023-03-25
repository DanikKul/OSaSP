//
// Created by dan on 23.2.23.
//

#ifndef TASK_MANAGER_UTILS_H
#define TASK_MANAGER_UTILS_H

#include <string.h>
#include <stdlib.h>
#include "../structures/structures.h"
#include "../processes/processes.h"

int cmpUser (const process *proc1, const process *proc2) {
    return strcmp(proc1 -> process_user, proc2 -> process_user) < 0;
}

int cmpName (const process *proc1, const process *proc2) {
    return strcmp(proc1 -> process_name, proc2 -> process_name) < 0;
}

int cmpPID (const process *proc1, const process *proc2) {
    return proc1 -> process_id < proc2 -> process_id;
}

int cmpPPID (const process *proc1, const process *proc2) {
    return proc1 -> process_ppid < proc2 -> process_ppid;
}

int cmpState (const process *proc1, const process *proc2) {
    return proc1 -> process_state < proc2 -> process_state;
}

int cmpCPU (const process *proc1, const process *proc2) {
    return proc1 -> cpu_usage < proc2 -> cpu_usage;
}

int cmpMem (const process *proc1, const process *proc2) {
    return proc1 -> mem_usage < proc2 -> mem_usage;
}

int cmpNice (const process *proc1, const process *proc2) {
    return proc1 -> process_nice < proc2 -> process_nice;
}

void sortByUser (processes *proc, int ascending) {
    for (int i = 0; i < proc -> n; i++) {
        for (int j = 0; j < proc -> n; j++) {
            int cmp = cmpUser(&(proc -> processes[i]), &(proc -> processes[j]));
            if (ascending * cmp + !ascending * !cmp) {
                process buff = proc -> processes[i];
                proc -> processes[i] = proc -> processes[j];
                proc -> processes[j] = buff;
            }
        }
    }
}

void sortByPID (processes *proc, int ascending) {
    for (int i = 0; i < proc -> n; i++) {
        for (int j = 0; j < proc -> n; j++) {
            int cmp = cmpPID(&(proc -> processes[i]), &(proc -> processes[j]));
            if (ascending * cmp + !ascending * !cmp) {
                process buff = proc -> processes[i];
                proc -> processes[i] = proc -> processes[j];
                proc -> processes[j] = buff;
            }
        }
    }
}

void sortByPPID (processes *proc, int ascending) {
    for (int i = 0; i < proc -> n; i++) {
        for (int j = 0; j < proc -> n; j++) {
            int cmp = cmpPPID(&(proc -> processes[i]), &(proc -> processes[j]));
            if (ascending * cmp + !ascending * !cmp) {
                process buff = proc -> processes[i];
                proc -> processes[i] = proc -> processes[j];
                proc -> processes[j] = buff;
            }
        }
    }
}

void sortByState (processes *proc, int ascending) {
    for (int i = 0; i < proc -> n; i++) {
        for (int j = 0; j < proc -> n; j++) {
            int cmp = cmpState(&(proc -> processes[i]), &(proc -> processes[j]));
            if (ascending * cmp + !ascending * !cmp) {
                process buff = proc -> processes[i];
                proc -> processes[i] = proc -> processes[j];
                proc -> processes[j] = buff;
            }
        }
    }
}

void sortByCPU (processes *proc, int ascending) {
    for (int i = 0; i < proc -> n; i++) {
        for (int j = 0; j < proc -> n; j++) {
            int cmp = cmpCPU(&(proc -> processes[i]), &(proc -> processes[j]));
            if (ascending * cmp + !ascending * !cmp) {
                process buff = proc -> processes[i];
                proc -> processes[i] = proc -> processes[j];
                proc -> processes[j] = buff;
            }
        }
    }
}

void sortByMem (processes *proc, int ascending) {
    for (int i = 0; i < proc -> n; i++) {
        for (int j = 0; j < proc -> n; j++) {
            int cmp = cmpMem(&(proc -> processes[i]), &(proc -> processes[j]));
            if (ascending * cmp + !ascending * !cmp) {
                process buff = proc -> processes[i];
                proc -> processes[i] = proc -> processes[j];
                proc -> processes[j] = buff;
            }
        }
    }
}

void sortByName (processes *proc, int ascending) {
    for (int i = 0; i < proc -> n; i++) {
        for (int j = 0; j < proc -> n; j++) {
            int cmp = cmpName(&(proc -> processes[i]), &(proc -> processes[j]));
            if (ascending * cmp + !ascending * !cmp) {
                process buff = proc -> processes[i];
                proc -> processes[i] = proc -> processes[j];
                proc -> processes[j] = buff;
            }
        }
    }
}

void sortByNice (processes *proc, int ascending) {
    for (int i = 0; i < proc -> n; i++) {
        for (int j = 0; j < proc -> n; j++) {
            int cmp = cmpNice(&(proc -> processes[i]), &(proc -> processes[j]));
            if (ascending * cmp + !ascending * !cmp) {
                process buff = proc -> processes[i];
                proc -> processes[i] = proc -> processes[j];
                proc -> processes[j] = buff;
            }
        }
    }
}

#endif //TASK_MANAGER_UTILS_H
