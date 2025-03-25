#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>

#define MAX_JOBS 100

// İş yapısını tanımla
typedef struct {
    char name[32];
    int arrival_time;
    int priority;
    int burst_time;
    pid_t pid;
    int remaining_time;
    int completed;
} Job;

Job jobs[MAX_JOBS];
int job_count = 0;
int time_slice = 0;

// Zaman damgası oluştur
void log_time(char *buffer) {
    time_t now = time(NULL);
    strftime(buffer, 32, "%Y-%m-%d %H:%M:%S", localtime(&now));
}

// İşlem hareketlerini kaydet
void log_event(const char *message) {
    FILE *log_file = fopen("scheduler.log", "a");
    if (!log_file) return;
    char timestamp[32];
    log_time(timestamp);
    fprintf(log_file, "[%s] [INFO] %s\n", timestamp, message);
    fclose(log_file);
}

// jobs.txt dosyasını oku
void read_jobs(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening jobs.txt");
        exit(EXIT_FAILURE);
    }

    fscanf(file, "TimeSlice %d", &time_slice);
    while (fscanf(file, "%s %d %d %d", jobs[job_count].name, &jobs[job_count].arrival_time, &jobs[job_count].priority, &jobs[job_count].burst_time) == 4) {
        jobs[job_count].remaining_time = jobs[job_count].burst_time;
        jobs[job_count].completed = 0;
        job_count++;
    }
    fclose(file);
}

// Öncelik sıralaması (küçük olan önceliklidir)
int compare_jobs(const void *a, const void *b) {
    Job *jobA = (Job *)a;
    Job *jobB = (Job *)b;

    if (jobA->arrival_time != jobB->arrival_time)
        return jobA->arrival_time - jobB->arrival_time;

    if (jobA->priority != jobB->priority)
        return jobA->priority - jobB->priority;

    if (jobA->remaining_time != jobB->remaining_time)
        return jobA->remaining_time - jobB->remaining_time;

    return 0;
}

// En yüksek öncelikli işi seç
int select_next_job(int current_time) {
    int idx = -1;
    for (int i = 0; i < job_count; i++) {
        if (!jobs[i].completed && jobs[i].arrival_time <= current_time) {
            if (idx == -1 || compare_jobs(&jobs[i], &jobs[idx]) < 0) {
                idx = i;
            }
        }
    }
    return idx;
}

int main() {
    read_jobs("jobs.txt");

    int current_time = 0;
    int current_job = -1;

    while (1) {
        current_job = select_next_job(current_time);
        if (current_job == -1) break;

        if (jobs[current_job].pid == 0) {
            pid_t pid = fork();
            if (pid == 0) {
                char *args[] = {jobs[current_job].name, NULL};
                execvp(jobs[current_job].name, args);
                perror("execvp failed");
                exit(EXIT_FAILURE);
            }
            jobs[current_job].pid = pid;
            char msg[128];
            sprintf(msg, "Forking new process for %s (PID: %d)", jobs[current_job].name, pid);
            log_event(msg);
        } else {
            kill(jobs[current_job].pid, SIGCONT);
            char msg[128];
            sprintf(msg, "Resuming %s (PID: %d) - SIGCONT", jobs[current_job].name, jobs[current_job].pid);
            log_event(msg);
        }

        sleep(time_slice);
        current_time += time_slice;

        if (jobs[current_job].remaining_time > time_slice) {
            jobs[current_job].remaining_time -= time_slice;
            kill(jobs[current_job].pid, SIGSTOP);
            char msg[128];
            sprintf(msg, "%s ran for %d seconds. Time slice expired - Sending SIGSTOP", jobs[current_job].name, time_slice);
            log_event(msg);
        } else {
            waitpid(jobs[current_job].pid, NULL, 0);
            jobs[current_job].completed = 1;
            char msg[128];
            sprintf(msg, "%s completed execution. Terminating (PID: %d)", jobs[current_job].name, jobs[current_job].pid);
            log_event(msg);
        }
    }

    printf("All jobs completed.\n");
    return 0;
}
