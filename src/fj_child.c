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

int set_limit(struct fj_config* config)
{
    // get cpu / memory / file size
    struct rlimit tLimit, mLimit, fLimit;
    getrlimit(RLIMIT_CPU, &tLimit);
    getrlimit(RLIMIT_AS, &mLimit);
    getrlimit(RLIMIT_FSIZE, &fLimit);

    // set cpu time limit (will SIGKILL(9) if exceed)
    tLimit.rlim_cur = tLimit.rlim_max = config->time_limit_sec; // only works if cpu 100% run for x seconds
    if (setrlimit(RLIMIT_CPU, &tLimit) != 0)
    {
        printf("set time limit fail\n");
        return EXIT_SET_LIMIT_FAIL;
    }
    // set memory limit (will SIGSEGV(11) if exceed
    mLimit.rlim_cur = mLimit.rlim_max = config->ram_limit_mb * 1024 * 1024;
    if (setrlimit(RLIMIT_AS, &mLimit) != 0)
    {
        printf("set memory limit fail\n");
        return EXIT_SET_LIMIT_FAIL;
    }
    // set file size limit (will SIGXFSZ(25) if exceed)
    fLimit.rlim_cur = config->file_limit_mb * 1024 * 1024;
    if (setrlimit(RLIMIT_FSIZE, &fLimit) == -1)
    {
        printf("set file size limit fail\n");
        return EXIT_SET_LIMIT_FAIL;
    }
    return 0;
}

int redirect_io(struct fj_config* config)
{
    // redirect inputFile to stdin
    FILE *fpIn = fopen(config->input_file, "r");
    if (fpIn == NULL)
    {
        return EXIT_REDIRECT_FAIL;
    }
    if (dup2(fileno(fpIn), STDIN_FILENO) == -1)
    {
        return EXIT_REDIRECT_FAIL;
    }

    // redirect stdout to outputFile
    FILE *fpOut = fopen(config->output_file, "w");
    if (fpOut == NULL)
    {
        return EXIT_REDIRECT_FAIL;
    }
    if (dup2(fileno(fpOut), STDOUT_FILENO) == -1)
    {
        return EXIT_REDIRECT_FAIL;
    }
    return 0;
}

#define MAX_SPLIT 128
char** split_args(char* str)
{
    char** result = malloc(sizeof(char*) * MAX_SPLIT);
    int i = 0;
    char* token = strtok(str, " \t");
    while (token != NULL)
    {
        result[i] = token;
        i++;
        token = strtok(NULL, " \t");
    }
    result[i] = NULL;
    return result;
}


int run_child(struct fj_config *config)
{
    // set limit
    int ret = set_limit(config);
    if (ret != 0)
    {
        return ret;
    }

    // redirect io
    ret = redirect_io(config);
    if (ret != 0)
    {
        return ret;
    }

    char** args = split_args(config->exec_args);

    if (execv(config->exec_path, args) < 0)
    {
        printf("execl fail: %s\n", strerror(errno));
        return EXIT_EXEC_FAIL;
    }
    return 0;
}