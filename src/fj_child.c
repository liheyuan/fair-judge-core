#include <string.h>
#include <errno.h>
#include <sys/wait.h>
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


#include "fj_child.h"


int run_child(struct fj_config *config)
{

    // get cpu / memory / file size
    struct rlimit tLimit, mLimit, fLimit;
    getrlimit(RLIMIT_CPU, &tLimit);
    getrlimit(RLIMIT_AS, &mLimit);
    getrlimit(RLIMIT_FSIZE, &fLimit);

    // set cpu time limit (will SIGKILL(9) if exceed)
    tLimit.rlim_cur = tLimit.rlim_max = config->cpuTimeLimitSec; // only works if cpu 100% run for x seconds
    if (setrlimit(RLIMIT_CPU, &tLimit) != 0)
    {
        printf("set time limit fail\n");
        return EXIT_SET_LIMIT_FAIL;
    }
    // set memory limit (will SIGSEGV(11) if exceed
    mLimit.rlim_cur = mLimit.rlim_max = config->ramLimitMB * 1024 * 1024;
    if (setrlimit(RLIMIT_AS, &mLimit) != 0)
    {
        printf("set memory limit fail\n");
        return EXIT_SET_LIMIT_FAIL;
    }
    // set file size limit (will SIGXFSZ(25) if exceed)
    fLimit.rlim_cur = config->fileLimitMB * 1024 * 1024;
    if (setrlimit(RLIMIT_FSIZE, &fLimit) == -1)
    {
        printf("set file size limit fail\n");
        return EXIT_SET_LIMIT_FAIL;
    }

    // redirect inputFile to stdin
    FILE *fpIn = fopen(config->inputFile, "r");
    if (fpIn == NULL)
    {
        return EXIT_REDIRECT_FAIL;
    }
    if (dup2(fileno(fpIn), STDIN_FILENO) == -1)
    {
        return EXIT_REDIRECT_FAIL;
    }

    // redirect stdout to outputFile
    FILE *fpOut = fopen(config->outputFile, "w");
    if (fpOut == NULL)
    {
        return EXIT_REDIRECT_FAIL;
    }
    if (dup2(fileno(fpOut), STDOUT_FILENO) == -1)
    {
        return EXIT_REDIRECT_FAIL;
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
    if (execl("/usr/bin/cat", "cat", NULL) < 0)
    {
        printf("execl fail: %s\n", strerror(errno));
        return EXIT_EXEC_FAIL;
    }
    return 0;
}