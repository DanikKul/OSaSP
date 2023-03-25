//
// Created by dan on 20.2.23.
//

#ifndef TASK_MANAGER_GRAPHICS_H
#define TASK_MANAGER_GRAPHICS_H

#include <ncurses.h>
#include <signal.h>
#include "../processes/processes.h"
#include "../utils/utils.h"

//// Define colors
#define LOW 1           // Black and green
#define MIDDLE 2        // Black and yellow
#define HIGH 3          // Black and red
#define BUTTON 4        // White and black
#define MAGENTA 5       // Black and magenta
#define DEFAULT 99      // Black and white

#define COMMAND_LIMIT 133   // Minimal inner window width to show command column
#define NAME_LIMIT 70       // Minimal inner window width to show name column
#define MEM_LIMIT 55        // Minimal inner window width to show memory usage column
#define CPU_LIMIT 46        // Minimal inner window width to show cpu usage column

//// Global variables
WINDOW *win = NULL, *inner = NULL;  // Init outer and inner windows
int USAGE_LENGTH = 0,               // Length of usage bars
height = 0,                         // Actual outer window height
width = 0,                          // Actual outer window width
cursor_position = 0,                // Actual cursor position in the list
start_position = 0,                 // Actual offset to calculate chosen element from the list
max_list = 0,                       // Maximal amount of lines that fit in inner window
chosen = 0,                         // Index of the chosen element from the list
s_user = 1,                         // Ascending/descending user sort flag
s_pid = 1,                          // Ascending/descending pid sort flag
s_ppid = 1,                         // Ascending/descending ppid sort flag
s_state = 1,                        // Ascending/descending state sort flag
s_cpu = 0,                          // Ascending/descending cpu sort flag
s_mem = 0,                          // Ascending/descending mem sort flag
s_name = 1,                         // Ascending/descending name sort flag
s_nice = 0;                         // Ascending/descending nice sort flag

//// Function that chooses sort method (used in main loop)
void sortProcess(processes *proc, int ch) {

    if (ch == 'q') {
        sortByUser(proc, s_user);       // user sort
    } else if (ch == 'w') {
        sortByPID(proc, s_pid);         // pid sort
    } else if (ch == 'e') {
        sortByPPID(proc, s_ppid);       // ppid sort
    } else if (ch == 'r') {
        sortByState(proc, s_state);     // state sort
    } else if (ch == 't') {
        sortByNice(proc, s_nice);       // nice sort
    } else if (ch == 'y') {
        sortByCPU(proc, s_cpu);         // cpu sort
    } else if (ch == 'u') {
        sortByMem(proc, s_mem);         // memory sort
    } else if (ch == 'i') {
        sortByName(proc, s_name);       // name sort
    }

}

//// Function that decorates message with color pair (Rendering function)
void printAttr(const char* msg, int y, int x, int pair) {
    wattron(win, pair);                 // Enable attribute
    mvwprintw(win, y, x, "%s", msg);    // Print message
    wattroff(win, pair);                // Disable attribute
}

//// Function that clears line located on some coordinate y (Rendering function)
void clearLine(int y) {
    int x = getmaxx(win) - 4;
    char* empty = calloc(x, sizeof(char));
    for (int i = 0; i < x; i++) empty[i] = ' ';
    mvwprintw(win, y, 2, "%s", empty);
    free(empty);
}

//// Function that updates memory usage bar in outer window (Rendering function)
void updateMemoryUsage(double mem_usage) {
    clearLine(4);
    mvwprintw(win, 4, 3, "MEM_USAGE: [");
    char *line = (char*) calloc (USAGE_LENGTH, sizeof(char));
    for (int i = 0; i < USAGE_LENGTH; i++) line[i] = ' ';
    for (int i = 0; i < (int) (USAGE_LENGTH * mem_usage / 100); i++) {
        line[i] = '|';
        if (mem_usage < 33.3) {
            printAttr(line, 4, 15, COLOR_PAIR(LOW));
        } else if (mem_usage < 66.6) {
            printAttr(line, 4, 15, COLOR_PAIR(MIDDLE));
        } else {
            printAttr(line, 4, 15, COLOR_PAIR(HIGH));
        }
    }
    char *percent = (char*) calloc(10, sizeof(char));
    sprintf(percent, "%.1lf", mem_usage);
    percent[strlen(percent)] = '%';
    percent[strlen(percent) + 1] = '\0';
    mvwprintw(win, 4, (int) strlen(line) + 15, "%6s] ", percent);
    free(percent);
    free(line);
}

//// Function that updates cpu usage bar in outer window (Rendering function)
void updateCpuUsage(cpuinfo *cpu_usage) {
    clearLine(2);
    mvwprintw(win, 2, 3, "CPU_USAGE: [");
    char *line = (char*) calloc (USAGE_LENGTH, sizeof(char));
    for (int i = 0; i < USAGE_LENGTH; i++) line[i] = ' ';
    for (int i = 0; i < (int) (USAGE_LENGTH * cpu_usage -> usage / 100); i++) {
        line[i] = '|';
        if (cpu_usage -> usage < 33.3) {
            printAttr(line, 2, 15, COLOR_PAIR(LOW));
        } else if (cpu_usage -> usage < 66.6) {
            printAttr(line, 2, 15, COLOR_PAIR(MIDDLE));
        } else {
            printAttr(line, 2, 15, COLOR_PAIR(HIGH));
        }
    }
    char *percent = (char*) calloc(10, sizeof(char));
    sprintf(percent, "%.1lf", cpu_usage -> usage);
    percent[strlen(percent)] = '%';
    percent[strlen(percent) + 1] = '\0';
    mvwprintw(win, 2, (int) strlen(line) + 15, "%6s] ", percent);
    free(percent);
    free(line);
}

//// Function that updates other common characteristics (Rendering function)
void updateOverall (
        int threads,
        int running, avgCPUUsage *average, time *up) {
    int length = USAGE_LENGTH + 40;
    mvwprintw(win, 2, length, "Threads: %d, running: %d", threads, running);
    if (average != NULL)
        mvwprintw(win, 3, length, "Load Average: %.2lf %.2lf %.2lf",
                  average -> avg_minute,
                  average -> avg_5minutes,
                  average -> avg_15minutes);
    if (up != NULL)
        mvwprintw(win, 4, length, "Uptime: %02d:%02d:%02d",
                  up -> hours,
                  up -> minutes,
                  up -> seconds);
}

//// Function that updates offset in the list (Common function)
void updateStartPosition(processes *proc) {
    if (cursor_position == max_list - 1 && chosen < proc -> n - 1) start_position++;
    if (cursor_position == 0 && start_position != 0) start_position--;
}

//// Function that prints lower menu (Rendering function)
void printMenu() {
    wattron(win, COLOR_PAIR(BUTTON));
    mvwprintw(win, height - 2, 6, "HELP");
    mvwprintw(win, height - 2, 14, "KILL");
    mvwprintw(win, height - 2, 22, "STOP");
    mvwprintw(win, height - 2, 30, "CONTINUE");
    mvwprintw(win, height - 2, 42, "EXIT");
    mvwprintw(win, height - 2, 47, "STATE");
    wattroff(win, COLOR_PAIR(BUTTON));

    wattron(win, COLOR_PAIR(DEFAULT));
    mvwprintw(win, height - 2, 3, "F1");
    mvwprintw(win, height - 2, 11, "F2");
    mvwprintw(win, height - 2, 19, "F3");
    mvwprintw(win, height - 2, 27, "F4");
    mvwprintw(win, height - 2, 39, "F5");
    wattroff(win, COLOR_PAIR(DEFAULT));

}

//// Function that updates processes in the list (Rendering function)
void renderProcesses(processes *PROCESSES) {

    int line = 1;
    wattron(inner, COLOR_PAIR(BUTTON));
    char *params = (char*) calloc (getmaxx(inner) - 2, sizeof(char));
    char *buff = (char*) calloc(100, sizeof(char));

    sprintf(buff, "%-15s ", "USER");
    strcpy(params, buff);

    sprintf(buff, "%-8s ", "PID");
    strcat(params, buff);

    sprintf(buff, "%-8s ", "PPID");
    strcat(params, buff);

    sprintf(buff, "%-7s ", "STATE");
    strcat(params, buff);

    sprintf(buff, "%-7s ", "NICE");
    strcat(params, buff);

    if (CPU_LIMIT < getmaxx(inner) - 2) {                   // Check if CPU column fits inner window
        sprintf(buff, "%-8s ", "CPU");
        strcat(params, buff);
    }

    if (MEM_LIMIT < getmaxx(inner) - 2) {                   // Check if MEM column fits inner window
        sprintf(buff, "%-8s ", "MEM");
        strcat(params, buff);
    }

    if (NAME_LIMIT < getmaxx(inner) - 2) {                  // Check if NAME column fits inner window
        sprintf(buff, "%-30s ", "NAME");
        strcat(params, buff);
    }

    if (COMMAND_LIMIT < getmaxx(inner) - 2) {               // Check if COMMAND column fits inner window
        sprintf(buff, "%-30s ", "COMMAND");
        strcat(params, buff);
    }

    for (int i = 0; i < getmaxx(inner) - 2; i++) if (params[i] == '\0') params[i] = ' ';

    params[strlen(params)] = '\0';
    mvwprintw(inner, 1, 1, "%s", params);
    wattroff(inner, COLOR_PAIR(BUTTON));
    chosen = start_position + cursor_position;

    // Handle situations when one or more processes were killed or deleted from list

    if (chosen >= PROCESSES -> n) {                         // Handle situation when chosen can be bigger that list amount
        while (chosen >= PROCESSES -> n) {
            chosen--;
            start_position--;
        }
    }

    if (start_position + getmaxy(inner) - 3 > PROCESSES -> n) { // Handle situation when offset is too big
        while (start_position + getmaxy(inner) - 3 > PROCESSES -> n) start_position--;
    }

    for (int i = 0; i < getmaxy(inner) - 3; i++, line += 2) {

        if (i == cursor_position) wattron(inner, COLOR_PAIR(BUTTON));

        mvwprintw(inner, i + 2, 1, "%-15s ", PROCESSES -> processes[i + start_position].process_user);
        mvwprintw(inner, i + 2, 17, "%-8d ", PROCESSES -> processes[i + start_position].process_id);
        mvwprintw(inner, i + 2, 26, "%-8d ", PROCESSES -> processes[i + start_position].process_ppid);
        mvwprintw(inner, i + 2, 35, "%-7c ", PROCESSES -> processes[i + start_position].process_state);
        mvwprintw(inner, i + 2, 43, "%-8d ", PROCESSES -> processes[i + start_position].process_nice);

        // Check if cpu column fits in inner window

        if (CPU_LIMIT < getmaxx(inner) - 2) mvwprintw(inner, i + 2, 51, "%-5.2lf%%%-2s ",
                                                      PROCESSES -> processes[i + start_position].cpu_usage, "");

        // Check if memory column fits in inner window

        if (MEM_LIMIT < getmaxx(inner) - 2) mvwprintw(inner, i + 2, 60, "%-5.2lf%%%-2s ",
                                                      PROCESSES -> processes[i + start_position].mem_usage, "");

        // Check if name column fits in inner window

        if (NAME_LIMIT < getmaxx(inner) - 2) mvwprintw(inner, i + 2, 69, "%-30s ",
                                                       PROCESSES -> processes[i + start_position].process_name);

        // Check if command column fits in inner window

        if (COMMAND_LIMIT < getmaxx(inner) - 2) {
            mvwprintw(inner, i + 2, 100, "%-90.90s ",
                      PROCESSES->processes[i + start_position].process_command);
            mvwprintw(inner, i + 2, 100 + 90, "%50s", " ");
        }
        if (i == cursor_position) wattroff(inner, COLOR_PAIR(BUTTON));
    }
    box(inner, 0, 0);       // Draws a box around inner window
    free(params);
    free(buff);
}

//// Function that clears inner and outer window and writes help window (Rendering + Docs function)

void helpWindow () {

    nodelay(inner, FALSE); // Enable delay for inputs (getch, wgetch)

    for (int i = 1; i < getmaxx(win) - 1; i++) { // Clear screen
        clearLine(i);
    }

    box(win, 0, 0); // Draw a box around outer window

    printAttr("TASK_MANAGER 0.0.1 - (C) 2023 DANIIL KULAKOVICH", 1, 5, COLOR_PAIR(LOW));
    printAttr("BARS", 3, 5, COLOR_PAIR(HIGH));
    mvwprintw(win, 5, 10, "CPU usage bar: [low : mid : high : PERCENT]");
    printAttr("LOW", 5, 26, COLOR_PAIR(LOW));
    printAttr("MID", 5, 32, COLOR_PAIR(MIDDLE));
    printAttr("HIGH", 5, 38, COLOR_PAIR(HIGH));
    mvwprintw(win, 6, 10, "Memory usage bar: [low : mid : high : PERCENT]");
    printAttr("LOW", 6, 29, COLOR_PAIR(LOW));
    printAttr("MID", 6, 35, COLOR_PAIR(MIDDLE));
    printAttr("HIGH", 6, 41, COLOR_PAIR(HIGH));
    printAttr("AVAILABLE COLUMNS", 8, 5, COLOR_PAIR(HIGH));
    mvwprintw(win, 10, 10, "USER - process owner");
    mvwprintw(win, 11, 10, "PID - process id");
    mvwprintw(win, 12, 10, "PPID - parent process id");
    mvwprintw(win, 13, 10, "STATE - state of a process ('R' - RUNNING, 'D' - UNINTERRUPTIBLE SLEEP, 'S' - INTERRUPTABLE SLEEP, 'T' - STOPPED, 'Z' - ZOMBIE, 'I' - IDLE)");
    mvwprintw(win, 14, 10, "NICE- shows priority of a process");
    mvwprintw(win, 15, 10, "CPU - CPU time usage");
    mvwprintw(win, 16, 10, "MEM - memory usage");
    mvwprintw(win, 17, 10, "NAME - process name");
    mvwprintw(win, 18, 10, "COMMAND - command that started process");
    printAttr("CONTROLS", 20, 5, COLOR_PAIR(HIGH));
    printAttr("KEYBOARD", 22, 10, COLOR_PAIR(MAGENTA));
    mvwprintw(win, 24, 15, "F1 - OPENS HELP WINDOW    'q' - SORTS BY USER     'y' - SORTS BY CPU");
    mvwprintw(win, 25, 15, "F2 - KILLS PROCESS        'w' - SORTS BY PID      'u' - SORTS BY MEMORY");
    mvwprintw(win, 26, 15, "F3 - STOPS PROCESS        'e' - SORTS BY PPID     'i' - SORTS BY NAME");
    mvwprintw(win, 27, 15, "F4 - CONTINUES PROCESS    'r' - SORTS BY STATE");
    mvwprintw(win, 28, 15, "F5 - EXIT                 't' - SORTS BY NICE");
    printAttr("MOUSE", 30, 10, COLOR_PAIR(MAGENTA));
    mvwprintw(win, 32, 15, "MOUSE SCROLL - scrolling processes");
    mvwprintw(win, 33, 15, "LEFT MOUSE BUTTON - selecting processes / choosing sorts / choosing actions");
    printAttr("ACTION PANEL", 35, 5, COLOR_PAIR(HIGH));
    mvwprintw(win, 37, 10, "Action panel shows last clicked process / last sort you applied / last action you clicked");
    printAttr("PRESS ANY KEY TO CONTINUE", 40, 5, COLOR_PAIR(MIDDLE));
    wrefresh(stdscr); // Refresh stdscr
    wrefresh(win); // Refresh outer window
    getch(); // Get any input
    nodelay(inner, TRUE); // Disable delay in input (getch, wgetch)

    for (int i = 1; i < getmaxx(win) - 2; i++) { // Clear screen
        clearLine(i);
    }

    box(win, 0, 0); // Draws a box around outer window
    box(inner, 0, 0); // Draws a box around inner window

}

void freeProcesses(processes *proc) {
    free(proc->processes);
    free(proc);
}

//// Function that runs application (Common + Rendering function)

void run () {

    initscr(); // Initialize screen
    getmaxyx(stdscr, height, width); // Get max height and width of terminal
    USAGE_LENGTH = width / 2 - 10; // Calculate optimal bars length
    win = newwin(height, width, 0, 0); // Initialize outer window
    inner = derwin(win, height - 8, width - 4, 6, 2); // Initialize inner window
    cbreak(); // Disable buffer
    noecho(); // On input make no echo
    nodelay(inner, TRUE); // Do not wait for wgetch, continue loop
    keypad(inner, TRUE); // Enable keypad
    scrollok(inner, TRUE); // Enable scrolling
    wscrl(inner, 1); // Enable scrolling in inner window
    curs_set(0); // Disable cursor
    MEVENT event; // Initialize mouse events
    mousemask(ALL_MOUSE_EVENTS, NULL); // Getting all mouse events

    // Draw boxes around windows

    box(win, 0, 0); // Draws a box around outer window
    box(inner, 0, 0); // Draws a box around inner window

    // Initialize all color pairs

    if (has_colors() == FALSE) return; // If no colors exit
    start_color(); // Initialize colors
    init_pair(LOW, COLOR_GREEN, COLOR_BLACK); // Initializes color pairs
    init_pair(MIDDLE, COLOR_YELLOW, COLOR_BLACK);
    init_pair(HIGH, COLOR_RED, COLOR_BLACK);
    init_pair(MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(BUTTON, COLOR_BLACK, COLOR_WHITE);
    init_pair(DEFAULT, COLOR_WHITE, COLOR_BLACK);
    assume_default_colors(COLOR_WHITE, COLOR_BLACK);

    mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION:%-20s", "");

    int flag = 'w'; // Default sort is PID sort

    while (1) {
        max_list = getmaxy(inner) - 3; // Calculate maximal amount of processes that fit in the list
        getmaxyx(stdscr, height, width); // Get max height and width of terminal
        USAGE_LENGTH = width / 2 - 10; // Recalculte bar length
        printMenu(); // Render menu
        processes *PROCESSES = NULL; // Initialize processes
        PROCESSES = getProcesses(); // Update processes
        sortProcess(PROCESSES, flag); // Sort processes
        renderProcesses(PROCESSES); // Put all processes in the list
        updateMemoryUsage(calculateMemoryUsage(getMemory())); // Render memory usage bar
        updateCpuUsage(getCpu()); // Render CPU usage bar
        mvwprintw(win, height - 2, 53, "CURSOR:%-3d", cursor_position);
        mvwprintw(win, height - 2, 64, "START:%-3d", start_position);
        mvwprintw(win, height - 2, 74, "CHOSEN:%-3d", chosen);
        mvwprintw(win, height - 2, 84, "AMOUNT:%-3d", PROCESSES -> n);
        // Render other characteristics
        updateOverall(getThreads(PROCESSES),getRunningProcesses(PROCESSES),getLoadAvg(),processUptime(getUptime()));
        int ch = wgetch(inner); // Get key
        if (ch == KEY_UP || ch == KEY_NPAGE) { // Move cursor up
            if (!(cursor_position <= 0)) cursor_position--;
            updateStartPosition(PROCESSES);
        } else if (ch == KEY_DOWN || ch == KEY_PPAGE) { // Move cursor down
            if (!(cursor_position >= max_list - 1)) cursor_position++;
            updateStartPosition(PROCESSES);
        } else if (ch == KEY_F(5)) { // Exit
            break;
        } else if (ch == KEY_F(1)) { // Open HELP window
            helpWindow();
        } else if (ch == KEY_F(2)) {
            kill(PROCESSES -> processes[chosen].process_id, SIGKILL); // kills process
            mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION:%-20s", "");
            mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION: KILL proc: %d", PROCESSES -> processes[chosen].process_id);
        } else if (ch == KEY_F(3)) {
            kill(PROCESSES -> processes[chosen].process_id, SIGSTOP); // stops process
            mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION:%-20s", "");
            mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION: STOP proc: %d", PROCESSES -> processes[chosen].process_id);
        } else if (ch == KEY_F(4)) {
            kill(PROCESSES -> processes[chosen].process_id, SIGCONT); // continue stopped process
            mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION:%-20s", "");
            mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION: CONTINUE proc: %d", PROCESSES -> processes[chosen].process_id);
        } else if (ch == 'q') {
            // user sort
            flag = 'q';
            s_user = !s_user;
            mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION:%-20s", "");
            mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION: SORTED BY USER");
            start_position = 0, cursor_position = 0, chosen = 0;
        } else if (ch == 'w') {
            // pid sort
            flag = 'w';
            s_pid = !s_pid;
            mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION:%-20s", "");
            mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION: SORTED BY PID");
            start_position = 0, cursor_position = 0, chosen = 0;
        } else if (ch == 'e') {
            // ppid sort
            flag = 'e';
            s_ppid = !s_ppid;
            mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION:%-20s", "");
            mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION: SORTED BY PPID");
            start_position = 0, cursor_position = 0, chosen = 0;
        } else if (ch == 'r') {
            // state sort
            flag = 'r';
            s_state = !s_state;
            mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION:%-20s", "");
            mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION: SORTED BY STATE");
            start_position = 0, cursor_position = 0, chosen = 0;
        } else if (ch == 't') {
            // nice sort
            flag = 't';
            s_nice = !s_nice;
            mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION:%-20s", "");
            mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION: SORTED BY NICE");
            start_position = 0, cursor_position = 0, chosen = 0;
        } else if (ch == 'y') {
            // cpu sort
            flag = 'y';
            s_cpu = !s_cpu;
            mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION:%-20s", "");
            mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION: SORTED BY CPU");
            start_position = 0, cursor_position = 0, chosen = 0;
        } else if (ch == 'u') {
            // mem sort
            flag = 'u';
            s_mem = !s_mem;
            mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION:%-20s", "");
            mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION: SORTED BY MEM");
            start_position = 0, cursor_position = 0, chosen = 0;
        } else if (ch == 'i') {
            // name sort
            flag = 'i';
            s_name = !s_name;
            mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION:%-20s", "");
            mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION: SORTED BY NAME");
            start_position = 0, cursor_position = 0, chosen = 0;
        } else if (getmouse(&event) == OK) { // If mouse event then
            if (event.bstate & BUTTON1_CLICKED) { // If LMB clicked
                mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION:%-25s", "");
                if (event.y - 8 >= 0 && event.y - 8 < max_list) {
                    cursor_position = event.y - 8;
                    chosen = cursor_position + start_position;
                    mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION: PRESSED ON %d", PROCESSES -> processes[chosen].process_id);
                } else if (event.y == height - 2 && event.x >= 3 && event.x <= 9) { // Open HELP window
                    helpWindow();
                } else if (event.y == height - 2 && event.x >= 11 && event.x <= 17) {
                    kill(PROCESSES -> processes[chosen].process_id, SIGKILL); // kills process
                    mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION:%-20s", "");
                    mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION: KILL proc: %d", PROCESSES -> processes[chosen].process_id);
                } else if (event.y == height - 2 && event.x >= 19 && event.x <= 25) {
                    kill(PROCESSES -> processes[chosen].process_id, SIGSTOP); // stops process
                    mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION:%-20s", "");
                    mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION: STOP proc: %d", PROCESSES -> processes[chosen].process_id);
                } else if (event.y == height - 2 && event.x >= 27 && event.x <= 37) {
                    kill(PROCESSES -> processes[chosen].process_id, SIGCONT); // continues stopped process
                    mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION:%-20s", "");
                    mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION: CONTINUE proc: %d", PROCESSES -> processes[chosen].process_id);
                } else if (event.y == height - 2 && event.x >= 39 && event.x <= 45) { // exit
                    break;
                } else if (event.y == 7 && event.x >= 3 && event.x <= 18) {
                    // user sort
                    flag = 'q';
                    s_user = !s_user;
                    mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION:%-20s", "");
                    mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION: SORTED BY USER");
                    start_position = 0, cursor_position = 0, chosen = 0;
                } else if (event.y == 7 && event.x >= 19 && event.x <= 27) {
                    // pid sort
                    flag = 'w';
                    s_pid = !s_pid;
                    mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION:%-20s", "");
                    mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION: SORTED BY PID");
                    start_position = 0, cursor_position = 0, chosen = 0;
                } else if (event.y == 7 && event.x >= 28 && event.x <= 36) {
                    // ppid sort
                    flag = 'e';
                    s_ppid = !s_ppid;
                    mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION:%-20s", "");
                    mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION: SORTED BY PPID");
                    start_position = 0, cursor_position = 0, chosen = 0;
                } else if (event.y == 7 && event.x >= 37 && event.x <= 44) {
                    // state sort
                    flag = 'r';
                    s_state = !s_state;
                    mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION:%-20s", "");
                    mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION: SORTED BY STATE");
                    start_position = 0, cursor_position = 0, chosen = 0;
                } else if (event.y == 7 && event.x >= 45 && event.x <= 52) {
                    // nice sort
                    flag = 't';
                    s_nice = !s_nice;
                    mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION:%-20s", "");
                    mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION: SORTED BY NICE");
                    start_position = 0, cursor_position = 0, chosen = 0;
                }  else if (event.y == 7 && event.x >= 53 && event.x <= 61) {
                    // cpu sort
                    flag = 'y';
                    s_cpu = !s_cpu;
                    mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION:%-20s", "");
                    mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION: SORTED BY CPU");
                    start_position = 0, cursor_position = 0, chosen = 0;
                } else if (event.y == 7 && event.x >= 62 && event.x <= 70) {
                    // mem sort
                    flag = 'u';
                    s_mem = !s_mem;
                    mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION:%-20s", "");
                    mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION: SORTED BY MEM");
                    start_position = 0, cursor_position = 0, chosen = 0;
                } else if (event.y == 7 && event.x >= 71 && event.x <= 101) {
                    // name sort
                    flag = 'i';
                    s_name = !s_name;
                    mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION:%-20s", "");
                    mvwprintw(win, height - 2, (int) (width * 0.8), "ACTION: SORTED BY NAME");
                    start_position = 0, cursor_position = 0, chosen = 0;
                }
            } else if (event.bstate & BUTTON5_PRESSED) { // Move cursor down
                if (!(cursor_position >= max_list - 1)) cursor_position++;
                updateStartPosition(PROCESSES);
            } else if (event.bstate & BUTTON4_PRESSED) { // Move cursor up
                if (!(cursor_position <= 0)) cursor_position--;
                updateStartPosition(PROCESSES);
            }
        }
        flushinp(); // Clear input (without that function getch and wgetch work slowly)
        wrefresh(win); // Refresh outer window
        wrefresh(inner); // Refresh inner window
        freeProcesses(PROCESSES);
    }
    endwin(); // Safe exit application
}


#endif //TASK_MANAGER_GRAPHICS_H