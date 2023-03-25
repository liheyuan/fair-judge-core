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
        .cpu_time_limit_sec = 1,
        .real_time_limit_sec = 1,
        .ram_limit_mb = 10,
        .file_limit_mb = 512,
        // succ
        //.exec_path = "/usr/bin/cat",
        //.exec_args = "cat",
        // cpu timeout
        //.exec_path = "/usr/bin/yes",
        //.exec_args = "yes",
        // real timeout
        .exec_path = "/usr/bin/sleep",
        .exec_args = "sleep 9",
        .input_file = "input.txt",
        .output_file = "output.txt"
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
