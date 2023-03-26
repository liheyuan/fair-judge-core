#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <limits.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

#include "fj_common.h"
#include "fj_parent.h"

#define MAX_ERR_MSG 1024

void timeout_killer(struct fj_timeout_args *args)
{
    if (args == NULL)
    {
        return;
    }
    // sleep for timeout + 1 seconds
    sleep(args->timeout);
    if(kill(args->pid, SIGKILL) != 0) {
        return;
    }
}

int compare_files(char *file1, char * file2) {
    FILE *fp1 = fopen(file1, "r");
    FILE *fp2 = fopen(file2, "r");
    if (fp1 == NULL || fp2 == NULL) {
        return -1;
    }
    char c1, c2;
    while (1) {
        c1 = fgetc(fp1);
        c2 = fgetc(fp2);
        if (c1 == EOF && c2 == EOF) {
            fclose(fp1);
            fclose(fp2);
            return 0;
        }
        if (c1 != c2) {
            fclose(fp1);
            fclose(fp2);
            return -2;
        }
    }
}

int compare_files_ignore_space(char *file1, char * file2) {
    FILE *fp1 = fopen(file1, "r");
    FILE *fp2 = fopen(file2, "r");
    if (fp1 == NULL || fp2 == NULL) {
        return -1;
    }
    char c1, c2;
    while (1) {
        while ((c1 = fgetc(fp1)) == ' ' || c1 == '\t' || c1 == '\r' || c1 == '\n');
        while ((c2 = fgetc(fp2)) == ' ' || c2 == '\t' || c2 == '\r' || c2 == '\n');
        if (c1 == EOF && c2 == EOF) {
            fclose(fp1);
            fclose(fp2);
            return 0;
        }
        if (c1 != c2) {
            fclose(fp1);
            fclose(fp2);
            return -2;
        }
    }
}

int run_parent(struct fj_config *config, int child_pid)
{
    // watch start
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // create a thread to monitor child process , after realTimeLimitSec, kill child process
    pthread_t tid;
    struct fj_timeout_args kill_arg = {child_pid, config->time_limit_sec};
    pthread_create(&tid, NULL, (void *)timeout_killer, (void *)&kill_arg);
    pthread_detach(tid);

    // wait for child process exit
    int s;
    waitpid(child_pid, &s, 0);

    // watch stop
    gettimeofday(&end, NULL);
    double childRealTimeInMs = (end.tv_sec - start.tv_sec) * 1000 + (float)(end.tv_usec - start.tv_usec) / 1000;

    // check child process exit status
    int err_code = ERR_OK;
    char err_msg[1024] = "accepted";
    if (WIFEXITED(s))
    {
        int child_exit_code = WEXITSTATUS(s);
        if (child_exit_code != 0) {
            err_code = ERR_CHILD_EXIT_NONZERO;
            snprintf(err_msg, MAX_ERR_MSG, "child process exit with code %d", child_exit_code);
        }
    }
    else
    {
        err_code = ERR_CHILD_EXIT_ABNORMALLY;
        snprintf(err_msg, MAX_ERR_MSG, "child process exit abnormally");
    }
    if (err_code == ERR_OK && WIFSIGNALED(s))
    {
        int sig = WTERMSIG(s);
        
        switch(sig)
        {
        case SIGKILL:
            err_code = ERR_TIME_LIMIT_EXCEED;
            snprintf(err_msg, MAX_ERR_MSG, "child process time limit exceed");
            break;
        case SIGSEGV:
            err_code = ERR_MEMORY_LIMIT_EXCEED;
            snprintf(err_msg, MAX_ERR_MSG, "child process memory limit exceed");
            break;
        default:
            err_code = ERR_UNKNOWN_SIGNAL;
            snprintf(err_msg, MAX_ERR_MSG, "child process exit with unknown signal %d", sig);
            break;
        }
    }

    // check answer if give answer file and not error yet
    if (strlen(config->answer_file) > 0 && err_code == ERR_OK) {
        if (compare_files(config->answer_file, config->output_file) != 0) {
            if (compare_files_ignore_space(config->answer_file, config->output_file) != 0) {
                err_code = ERR_WRONG_ANSWER;
                snprintf(err_msg, MAX_ERR_MSG, "wrong answer");
            } else {
                err_code = ERR_PRESENTATION_ERROR;
                snprintf(err_msg, MAX_ERR_MSG, "may be presentation problem");
            }
        }
    }

    // get child process cpu / memory
    struct rusage usage;
    getrusage(RUSAGE_CHILDREN, &usage);
    double childCpuTimeInMs = (usage.ru_utime.tv_sec + usage.ru_stime.tv_sec) * 1000 + (float)(usage.ru_stime.tv_usec + usage.ru_utime.tv_usec) / 1000;

    printf("{\"cpu_time_ms\": %ld, \"real_time_ms\": %ld, \"memory_kb\": %ld, \"err_code\": %d, \"err_msg\": \"%s\"}\n", 
    (long)childCpuTimeInMs, (long)childRealTimeInMs, err_code == ERR_MEMORY_LIMIT_EXCEED ? config->ram_limit_mb * 1024 * 1024 : usage.ru_maxrss, err_code, err_msg);
    return 0;
}