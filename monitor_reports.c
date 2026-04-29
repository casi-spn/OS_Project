#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

#define PID_FILE ".monitor_pid"

static volatile sig_atomic_t running = 1;

// SIGUSR1 → new report
void handle_sigusr1(int sig) {
    (void)sig;
    write(STDOUT_FILENO, "New report added\n", 17);
}

// SIGINT → stop
void handle_sigint(int sig) {
    (void)sig;
    write(STDOUT_FILENO, "Shutting down\n", 14);
    running = 0;
}

int main() {
    int fd;
    char buffer[32];
    pid_t pid = getpid();

    // 1. create/overwrite PID file
    fd = open(PID_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    // write PID as text
    int len = snprintf(buffer, sizeof(buffer), "%d\n", pid);
    write(fd, buffer, len);
    close(fd);

    printf("Monitor running (PID=%d)\n", pid);

    // 2. setup signals
    struct sigaction sa1, sa2;

    sa1.sa_handler = handle_sigusr1;
    sigemptyset(&sa1.sa_mask);
    sa1.sa_flags = 0;
    sigaction(SIGUSR1, &sa1, NULL);

    sa2.sa_handler = handle_sigint;
    sigemptyset(&sa2.sa_mask);
    sa2.sa_flags = 0;
    sigaction(SIGINT, &sa2, NULL);

    // 3. wait forever
    while (running) {
        pause();
    }

    // 4. delete PID file
    unlink(PID_FILE);

    return 0;
}