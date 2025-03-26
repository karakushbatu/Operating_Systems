#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <stdarg.h>

#define MAX_JOBS 100
#define MAX_NAME_LEN 50

// Job yapısı: Scheduler için gerekli bilgileri içerir.
typedef struct {
    char name[MAX_NAME_LEN];
    int arrival_time;
    int priority;
    int exec_time;
    int remaining_time;
    pid_t pid;
    int started;
    int finished;
    int order;
} Job;

// Loglama fonksiyonu: Timestamp ile birlikte mesajları scheduler.log'a yazar.
void log_event(FILE *log_file, const char *format, ...) {
    char time_str[64];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", t);

    fprintf(log_file, "[%s] [INFO] ", time_str);

    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
    fprintf(log_file, "\n");
    fflush(log_file);
}

// Job’ları karşılaştırmak için kullanılan sıralama fonksiyonu
int compare_jobs(const void *a, const void *b) {
    Job *jobA = *(Job **)a;
    Job *jobB = *(Job **)b;
    if (jobA->priority != jobB->priority)
        return jobA->priority - jobB->priority;
    if (jobA->arrival_time != jobB->arrival_time)
        return jobA->arrival_time - jobB->arrival_time;
    if (jobA->remaining_time != jobB->remaining_time)
        return jobA->remaining_time - jobB->remaining_time;
    return jobA->order - jobB->order;
}

// --- Job modu: Eğer program "job" parametresi ile çağrılırsa burası çalışır.
void run_job(const char* job_name, int exec_time) {
    // SIGCONT sinyalini yakalamak için basit bir handler ayarlanabilir.
    signal(SIGCONT, SIG_DFL);
    int remaining_time = exec_time;
    while (remaining_time > 0) {
        sleep(1);
        remaining_time--;
    }
    exit(0);
}

// --- Main fonksiyonu: Mod seçimine göre scheduler veya job modu çalışır.
int main(int argc, char *argv[]) {
    // Eğer program "job" parametresi ile çağrıldıysa job moduna geç.
    if (argc >= 4 && strcmp(argv[1], "job") == 0) {
        int exec_time = atoi(argv[3]);
        run_job(argv[2], exec_time);
        exit(0);
    }

    // --- Scheduler modu başlıyor.
    FILE *fp = fopen("jobs.txt", "r");
    if (!fp) {
        perror("jobs.txt dosyası açılamadı");
        exit(EXIT_FAILURE);
    }
    FILE *log_file = fopen("scheduler.log", "w");
    if (!log_file) {
        perror("scheduler.log dosyası açılamadı");
        exit(EXIT_FAILURE);
    }

    char line[256];
    int time_slice = 0;
    Job jobs[MAX_JOBS];
    int job_count = 0;

    // TimeSlice satırını oku
    if (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "TimeSlice %d", &time_slice) != 1) {
            fprintf(stderr, "TimeSlice formatı hatalı\n");
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "jobs.txt boş\n");
        exit(EXIT_FAILURE);
    }

    // Job satırlarını oku
    while (fgets(line, sizeof(line), fp)) {
        if (strlen(line) <= 1) continue;
        Job j;
        if (sscanf(line, "%s %d %d %d", j.name, &j.arrival_time, &j.priority, &j.exec_time) != 4) {
            fprintf(stderr, "Job formatı hatalı: %s\n", line);
            continue;
        }
        j.remaining_time = j.exec_time;
        j.pid = -1;
        j.started = 0;
        j.finished = 0;
        j.order = job_count;
        jobs[job_count++] = j;
    }
    fclose(fp);

    int current_time = 0;
    int finished_jobs = 0;
    Job *last_job = NULL;

    // Ana scheduling döngüsü
    while (finished_jobs < job_count) {
        // Ready queue: varış zamanı gelmiş ve tamamlanmamış job’lar
        Job *ready_queue[MAX_JOBS];
        int ready_count = 0;
        for (int i = 0; i < job_count; i++) {
            if (!jobs[i].finished && jobs[i].arrival_time <= current_time) {
                ready_queue[ready_count++] = &jobs[i];
            }
        }

        if (ready_count == 0) {
            sleep(1);
            current_time++;
            continue;
        }

        // Sıralama: Öncelik, arrival time, kalan süre, dosya sırası
        qsort(ready_queue, ready_count, sizeof(Job*), compare_jobs);

        // Az önce preempt edilen job’u mümkünse atla
        Job *current_job = ready_queue[0];
        if (ready_count > 1 && last_job != NULL && ready_queue[0] == last_job) {
            current_job = ready_queue[1];
        }

        // Eğer job daha önce başlatılmamışsa fork ve execl kullanarak başlat.
        if (!current_job->started) {
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork hatası");
                exit(EXIT_FAILURE);
            }
            if (pid == 0) {
                // Child process: Aynı binary'yi "job" modu ile çalıştır.
                char exec_time_str[10];
                sprintf(exec_time_str, "%d", current_job->exec_time);
                execl(argv[0], argv[0], "job", current_job->name, exec_time_str, (char *)NULL);
                perror("execl hatası");
                exit(EXIT_FAILURE);
            } else {
                current_job->pid = pid;
                current_job->started = 1;
                log_event(log_file, "Forking new process for %s", current_job->name);
                log_event(log_file, "Executing %s (PID: %d) using exec", current_job->name, pid);
            }
        } else {
            // Daha önce duraklatılan job’u resume et.
            kill(current_job->pid, SIGCONT);
            log_event(log_file, "Resuming %s (PID: %d) - SIGCONT", current_job->name, current_job->pid);
        }

        // Çalıştırma süresi: time slice veya kalan çalışma süresi arasından küçük olanı seç.
        int run_duration = (current_job->remaining_time < time_slice) ? current_job->remaining_time : time_slice;
        sleep(run_duration);
        current_time += run_duration;

        // Eğer job'un kalan süresi run_duration kadar ise; yani bu çalıştırma sonlanıyorsa:
        if (current_job->remaining_time == run_duration) {
            int status;
            // İş tamamlandığı için blocking wait yap.
            waitpid(current_job->pid, &status, 0);
            log_event(log_file, "%s completed execution. Terminating (PID: %d)", current_job->name, current_job->pid);
            current_job->finished = 1;
            finished_jobs++;
        } else {
            // Aksi durumda, time slice dolduğundan SIGSTOP gönder.
            kill(current_job->pid, SIGSTOP);
            log_event(log_file, "%s ran for %d seconds. Time slice expired - Sending SIGSTOP", current_job->name, run_duration);
            current_job->remaining_time -= run_duration;
        }
        last_job = current_job;
    }

    fclose(log_file);
    return 0;
}
