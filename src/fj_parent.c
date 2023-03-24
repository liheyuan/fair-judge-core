#include <stdio.h>
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
    sleep(args->timeout + 1);
    if(kill(args->pid, SIGKILL) != 0) {
        return;
    }
}


int run_parent(struct fj_config *config, int child_pid)
{
    // watch start
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // create a thread to monitor child process , after realTimeLimitSec, kill child process
    pthread_t tid;
    struct fj_timeout_args kill_arg = {child_pid, config->realTimeLimitSec};
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
    char err_msg[1024] = {0};
    if (WIFEXITED(s))
    {
        printf("exit normally code: %d.\n", WEXITSTATUS(s));
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
    if (WIFSIGNALED(s))
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

    // get child process cpu / memory
    struct rusage usage;
    getrusage(RUSAGE_CHILDREN, &usage);
    double childCpuTimeInMs = (usage.ru_utime.tv_sec + usage.ru_stime.tv_sec) * 1000 + (float)(usage.ru_stime.tv_usec + usage.ru_utime.tv_usec) / 1000;

    printf("{\"cpu_time_ms\": %ld, \"real_time_ms\": %ld, \"memory_kb\": %ld, \"err_code\": %d, \"err_msg\": \"%s\"}\n", 
    (long)childCpuTimeInMs, (long)childRealTimeInMs, usage.ru_maxrss, err_code, err_msg);
    return 0;
}