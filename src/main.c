#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include "fj_common.h"
#include "fj_child.h"
#include "fj_parent.h"

int main()
{
    // test args

    struct fj_config config = {
        .cpuTimeLimitSec = 1,
        .realTimeLimitSec = 1,
        .ramLimitMB = 10,
        .fileLimitMB = 18,
        .inputFile = "input.txt",
        .outputFile = "output.txt"
    };

    pid_t pid;

    if ((pid = fork()) < 0)
    {
        printf("fork fail\n");
        return EXIT_FORK_FAIL;
    }
    else if (pid == 0)
    {
        return run_child(&config);
    }
    else
    {
        return run_parent(&config, pid);
    }
}
