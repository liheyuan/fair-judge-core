#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>

#define EXIT_FORK 2;
#define EXIT_SET_LIMIT 3;
#define EXIT_EXEC_FAIL 4;
#define EXIT_INPUT_OPEN_FAIL 5;
#define EXIT_INPUT_DUP_FAIL 6;
#define EXIT_OUTPUT_OPEN_FAIL 7;
#define EXIT_OUTPUT_DUP_FAIL 8;

struct timeout_killer_args
{
    pid_t pid;
    int timeout;
};


void timeout_killer(struct timeout_killer_args *args)
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

int main()
{
    // test args
    int cpuTimeLimitSec = 1;
    int realTimeLimitSec = 1;
    int ramLimitMB = 18;
    int fileLimitMB = 1;
    char* inputFile = "input.txt";
    char* outputFile = "output.txt";

    int s;

    pid_t pid;

    if ((pid = fork()) < 0)
    {
        printf("fork fail\n");
        return EXIT_FORK;
    }
    else if (pid == 0)
    {
        // child process

        // get cpu / memory / file size
        struct rlimit tLimit, mLimit, fLimit;
        getrlimit(RLIMIT_CPU, &tLimit);
        getrlimit(RLIMIT_AS, &mLimit);
        getrlimit(RLIMIT_FSIZE, &fLimit);

        // set cpu / memory / file size
        tLimit.rlim_cur = tLimit.rlim_max = cpuTimeLimitSec; // only works if cpu 100% run for x seconds
        if (setrlimit(RLIMIT_CPU, &tLimit) != 0)
        {
            printf("set time limit fail\n");
            return EXIT_SET_LIMIT;
        }
        mLimit.rlim_cur = mLimit.rlim_max = ramLimitMB * 1024 * 1024;
        if (setrlimit(RLIMIT_AS, &mLimit) != 0)
        {
            printf("set memory limit fail\n");
            return EXIT_SET_LIMIT;
        }
        fLimit.rlim_cur = fileLimitMB * 1024 * 1024;
        if (setrlimit(RLIMIT_FSIZE, &fLimit) == -1)
        {
            printf("set file size limit fail\n");
            return EXIT_SET_LIMIT;
        }

        // redirect inputFile to stdin
        FILE* fpIn = fopen(inputFile, "r");
        if (fpIn == NULL)
        {
            return EXIT_INPUT_OPEN_FAIL;
        }
        if (dup2(fileno(fpIn), STDIN_FILENO) == -1)
        {
            return EXIT_INPUT_DUP_FAIL;
        }

        // redirect stdout to outputFile
        FILE *fpOut = fopen(outputFile, "w");
        if (fpOut == NULL)
        {
            return EXIT_OUTPUT_OPEN_FAIL;
        }
        if (dup2(fileno(fpOut), STDOUT_FILENO) == -1)
        {
            return EXIT_OUTPUT_DUP_FAIL;
        }


        // test for cpu timeout
        // if (execl("/usr/bin/yes", "yes", NULL) < 0) {
        //     printf("execl fail: %s\n", strerror(errno));
        //     return EXIT_EXEC_FAIL;
        // }
        // test for real timeout
        // if (execl("/usr/bin/sleep", "sleep", "9",NULL) < 0) {
        //     printf("execl fail: %s\n", strerror(errno));
        //     return EXIT_EXEC_FAIL;
        // }
        // test for input / output
        if (execl("/usr/bin/cat", "cat", NULL) < 0) {
            printf("execl fail: %s\n", strerror(errno));
            return EXIT_EXEC_FAIL;
        }
    }
    else
    {
        // parent process

        // create a thread to monitor child process , after realTimeLimitSec, kill child process
        pthread_t tid;
        struct timeout_killer_args kill_arg = {pid, realTimeLimitSec};
        pthread_create(&tid, NULL, (void *)timeout_killer, (void *)&kill_arg);
        pthread_detach(tid);

        // wait for child process exit
        waitpid(pid, &s, 0);
        if (WIFEXITED(s))
        {
            printf("exit normally code: %d.\n", WEXITSTATUS(s));
        }
        else
        {
            printf("exit abnormally.\n");
        }
        if (WIFSIGNALED(s))
        {
            printf("exit by signal: %d | %d\n", s, WTERMSIG(s));

            switch (s)
            {
            case SIGKILL:
                printf("cpu limit exceed\n");
                break;
            case SIGSEGV:
                printf("memory limit exceed\n");
                break;
            default:
                break;
            }
        }

        // get child process cpu / memory
        struct rusage usage;
        getrusage(RUSAGE_CHILDREN, &usage);
        double childCpuTimeInMs = (usage.ru_utime.tv_sec + usage.ru_stime.tv_sec) * 1000 + (float)(usage.ru_stime.tv_usec + usage.ru_utime.tv_usec) / 1000;
        printf("child process cpu time: %ld ms\n", (long)childCpuTimeInMs);
        printf("child process memory: %ld kb\n", usage.ru_maxrss); 
        return 0;
    }
}
