//
// Created by dan on 18.2.23.
//

#ifndef TASK_MANAGER_STRUCTURES_H
#define TASK_MANAGER_STRUCTURES_H

// Main process structure

typedef struct {
    int process_id; // Process id
    int process_ppid; // Parent process id
    char process_state; // State of a process
    char process_name[128]; // Name of a process
    char process_command[128]; // Terminal command that launched process
    double cpu_usage;
    double mem_usage;
    char process_user[128];
} process;

// Main processes structure

typedef struct {
    process *processes; // Structure with all parameters of a process
    int n; // Amount of processes
} processes;

// Mid structure for parsing /proc/status file

typedef struct {
    char *name; // Process name
    int ppid; // Parent pid
    char state; // Current state (can be R-Running, D-Uninterruptible sleep, S-Interruptable sleep, T-Stopped, Z-Zombie)
    char *user;
} status;

// Process CPU time characteristics /proc/stat file

typedef struct {
    double utime; // CPU time spent in user mode
    double stime; // CPU time spent in kernel mode
    double cutime; // Waited-for-children's CPU time spent in user mode
    double cstime; // Waited-for-children's CPU time spent in kernel mode
    double starttime; // Time when the process started
} stat;

// Average CPU usage for last 1, 5 and 15 minutes

typedef struct {
    double avg_minute;
    double avg_5minutes;
    double avg_15minutes;
} avgCPUUsage;

// Memory (variables are written in kB) /proc/meminfo file

typedef struct {
    unsigned long total;
    unsigned long free;
    unsigned long available;
} memory;

// Overall CPU usage /proc/stat

typedef struct {
    double running;
    double total;
} cpu;

// CPU Usage count ticks difference between two snapshots

typedef struct {
    cpu* snapshot1;
    cpu* snapshot2;
    double usage;
} cpuinfo;

// Uptime convert to human-friendly time

typedef struct {
    int hours;
    int minutes;
    int seconds;
} time;

#endif //TASK_MANAGER_STRUCTURES_H
