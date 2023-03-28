#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

struct stats {
    int n1;
    int n2;
};

struct processes {
    pid_t pid;
    int state;
};

struct processes* children;
struct sigaction act, oldact;
struct stats st, *mem;
int len = 0,
stop = 0,
allow = 0,
lenMem = 0;

/** @p Signal handler for child processes **/
static void childHandler (int signal) {
    if (signal == SIGALRM) {
        int flag = 1;
        for (int i = 0; i < lenMem; i++) {
            if (mem[i].n1 == st.n1 && mem[i].n2 == st.n2) {
                flag = 0;
            }
        }
        if (flag) {
            mem[lenMem].n1 = st.n1;
            mem[lenMem].n2 = st.n2;
            lenMem++;
        }

        stop = 1;
    } else if (signal == SIGUSR2) {
        allow = 1;
    } else if (signal == SIGUSR1) {
        allow = 0;
    } else if (signal == SIGTERM) {
        kill(getppid(), SIGCHLD);
        exit(0);
    }
}

/** @p Function that searches index from "children" by PID **/
int searchByValue (pid_t pid) {
    for (int i = 0; i < len; i++) {
        if (children[i].pid == pid) return i;
    }
    return -1;
}

/** @p Function that every child process executes **/
void childProcess () {
    struct timespec tw = {0, 50000000};
    int ticker = 0;
    struct stats zeros = {0, 0}, ones = {1, 1};
    mem = (struct stats*) malloc (10 * sizeof(struct stats));
    signal(SIGALRM, childHandler);
    signal(SIGUSR1, childHandler);
    signal(SIGUSR2, childHandler);
    signal(SIGTERM, childHandler);
    while (1) {
        nanosleep(&tw, NULL);
        kill(getpid(), SIGALRM);
        while (1) {
            st = zeros;
            st = ones;
            if (stop) {
                stop = 0;
                break;
            }
        }
        if (ticker == 101) {
            kill(getppid(), SIGUSR1);
            if (allow) {
                char *pid = (char*) malloc (20 * sizeof(char));
                sprintf(pid, "%d", getpid());
                for (int i = 0; i < strlen(pid); i++) {
                    fputc(pid[i], stdout);
                }
                fputc(' ', stdout);
                free(pid);
                pid = (char*) malloc (20 * sizeof(char));
                sprintf(pid, "%d", getppid());
                for (int i = 0; i < strlen(pid); i++) {
                    fputc(pid[i], stdout);
                }
                fputc(' ', stdout);
                free(pid);

                for (int i = 0; i < lenMem; i++) {
                    pid = (char*) malloc (20 * sizeof(char));
                    sprintf(pid, "%d", mem[i].n1);
                    for (int j = 0; j < strlen(pid); j++) {
                        fputc(pid[j], stdout);
                    }
                    fputc(' ', stdout);
                    free(pid);
                    pid = (char*) malloc (20 * sizeof(char));
                    sprintf(pid, "%d", mem[i].n2);
                    for (int j = 0; j < strlen(pid); j++) {
                        fputc(pid[j], stdout);
                    }
                    fputc(' ', stdout);
                    free(pid);
                }
                fputc('\n', stdout);
            }
            ticker = 0;
        }
        ticker++;
    }
}

/** @p Function that creates new process**/
void addProcess () {
    fprintf(stdout, "\n\nAdding process...\n");
    pid_t childPid;
    if ((childPid = fork()) == 0) {
        childProcess();
    } else if (childPid > 0) {
        fprintf(stdout, "Created process with pid: %d\n", childPid);
    } else {
        fprintf(stdout, "Cannot create child process\n\n");
        return;
    }
    children[len].pid = childPid;
    children[len].state = 0;
    len++;
    fprintf(stdout, "Child process created successfully\n");
}

/** @p Function that kills last created process **/
void subProcess () {
    if (len == 0) {
        fprintf(stdout, "There's nothing to delete\n");
        return;
    }
    fprintf(stdout, "\n\nDeleting last created process...\n");
    fprintf(stdout, "Deleting child process with pid: %d\n", children[len - 1].pid);
    kill(children[len - 1].pid, SIGTERM);
    len--;
    fprintf(stdout, "Deleting completed\n\n");
}

/** @p Function that prints all parent and child processes **/
void printAllProcesses () {
    fprintf(stdout, "\n\nParent pid: %d\nChildren:\n", getpid());
    if (len == 0) {
        fprintf(stdout, "There's no child processes\n");
        return;
    }
    for (int i = 0; i < len; i++) {
        fprintf(stdout, "№%d pid = %d\n", i + 1, children[i].pid);
    }
    fprintf(stdout, "\n\n");
}

/** @p Function that deletes all C_k and notifies user about it **/
void deleteAll () {
    if (len == 0) {
        fprintf(stdout, "There's no processes to delete\n");
        return;
    }
    fprintf(stdout, "\n\nDeleting all child processes...\n");
    for (int i = 0; i < len; i++) {
        fprintf(stdout, "Deleting child process with pid: %d\n", children[i].pid);
        kill(children[i].pid, SIGTERM);
    }
    len = 0;
    fprintf(stdout, "Deleting completed\n\n");
}

/** @p Function that denies all C_k to display statistics **/
void denyAllStats () {
    for (int i = 0; i < len; i++) {
        children[i].state = 0;
        kill(children[i].pid, SIGUSR1);
    }
}

/** @p Function that allows all C_k to display statistics **/
void allowAllStats () {
    for (int i = 0; i < len; i++) {
        children[i].state = 1;
        kill(children[i].pid, SIGUSR2);
    }
}

/** Function that denies C_num to display statistics **/
void denyStats (int num) {
    int idx = searchByValue(num);
    if (idx != -1) {
        children -> state = 0;
        fprintf(stdout, "Denied access to stdout for process with pid: %d\n", num);
        kill(num, SIGUSR1);
    } else {
        fprintf(stdout, "No such child process found\n");
    }
}

/** @p Function that allows C_<num> to display statistics **/
void allowStats (int num) {
    int idx = searchByValue(num);
    if (idx != -1) {
        children -> state = 1;
        fprintf(stdout, "Allowed access to stdout for process with pid: %d\n", num);
        kill(num, SIGUSR2);
    } else {
        fprintf(stdout, "No such child process found\n");
    }
}

/** @p Function that denies all C_k to display output and requests C_num to display it statistics
*   @note Timeout 5 seconds **/
void requestStats (int num) {
    int idx = searchByValue(num);
    if (idx != -1) {
        denyAllStats();
        children -> state = 1;
        fprintf(stdout, "Requested statistics for process with pid: %d\n", num);
        kill(num, SIGUSR2);
        alarm(11);
    } else {
        fprintf(stdout, "No such child process found\n");
    }
}

/** @p Signal handler for parent process **/
void parentHandler (int signal, siginfo_t *info, void *ptr) {
    if (signal == SIGUSR1) {
        fprintf(stdout, "Got request from child process with %d\n", info -> si_pid);
        int state = children[searchByValue(info -> si_pid)].state;
        if (!state) {
            fprintf(stdout, "Denied access to stdout for process %d\n", info -> si_pid);
            kill(info -> si_pid, SIGUSR1);
        } else {
            fprintf(stdout, "Allowed access to stdout for process %d\n", info -> si_pid);
            kill(info -> si_pid, SIGUSR2);
        }
    } else if (signal == SIGALRM) {
        allowAllStats();
    } else if (signal == SIGCHLD) {
        int status;
        wait(&status);
    }
}

int main() {
    children = (struct processes*) malloc(200 * sizeof(struct processes));
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = parentHandler;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &act, &oldact);
    sigaction(SIGALRM, &act, &oldact);
    sigaction(SIGCHLD, &act, &oldact);
    while (1) {
        char* input = (char*) malloc(20 * sizeof(char));
        scanf("%s", input);
        if (strlen(input) == 1) {
            if (strcmp(input, "l") == 0) {
                printAllProcesses();
            } else if (strcmp(input, "k") == 0) {
                deleteAll();
            } else if (strcmp(input, "s") == 0) {
                denyAllStats();
            } else if (strcmp(input, "+") == 0) {
                addProcess();
            } else if (strcmp(input, "-") == 0) {
                subProcess();
            } else if (strcmp(input, "g") == 0) {
                allowAllStats();
            } else if (strcmp(input, "q") == 0) {
                deleteAll();
                break;
            }
        } else if (strlen(input) > 1) {
            int no = 0;
            for (int i = 2; i < strlen(input) - 1; i++) {
                no *= 10;
                no += input[i] - '0';
            }
            if (input[0] == 's') {
                denyStats(no);
            } else if (input[0] == 'g') {
                allowStats(no);
            } else if (input[0] == 'p') {
                requestStats(no);
            }
        }
        fflush(stdin);
        free(input);
    }
    free(children);
    return 0;
}
