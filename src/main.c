#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include "fj_common.h"
#include "fj_child.h"
#include "fj_parent.h"

void print_usage() {
    printf("Usage:\n");
    printf("fjcore -p <abs_path_bin> -c <bin_full_cmd> -i <input_file> -o <output_file> -a <answer_file, skip check if not provided>");
    printf("-t <time_limit_sec, default 1> -m <mem_limit_mb, default 8> -f <file_limit_mb, default 16> -s <limit_sys_call, default 1>\n");
}

int parse_config(int argc, char **argv, struct fj_config *config)
{    
    // parse argc & argv into fj_config
    int opt;
    opterr = 0;

    while ((opt = getopt(argc, argv, "t:m:f:p:c:o:i:a:s:")) != EOF)
    {
        switch (opt)
        {
        case 't':
            config->time_limit_sec = atoi(optarg);
            break;
        case 'm':
            config->ram_limit_mb = atoi(optarg);
            break;
        case 'f':
            config->file_limit_mb = atoi(optarg);
            break;
        case 'p':
            strncpy(config->exec_path, optarg, MAX_STR_LEN - 1);
            break;
        case 'c':
            strncpy(config->exec_args, optarg, MAX_STR_LEN - 1);
            break;
        case 'o':
            strncpy(config->output_file, optarg, MAX_STR_LEN - 1);
            break;
        case 'i':
            strncpy(config->input_file, optarg, MAX_STR_LEN - 1);
            break;
        case 'a':
            strncpy(config->answer_file, optarg, MAX_STR_LEN - 1);
            break;
        case 's':
            config->syscall_limit = atoi(optarg);
            break;
        default:
            break;
        }
    }

    if (config->time_limit_sec <= 0 || config->ram_limit_mb <= 0 || config->file_limit_mb <= 0)
    {
        printf("limit setting is invalid\n");
        return -1;
    }
    
    if (strlen(config->exec_path) == 0 || strlen(config->exec_args) == 0)
    {
        printf("exec path or args is empty\n");
        return -1;
    }

    if (strlen(config->input_file) == 0 || strlen(config->output_file) == 0)
    {
        printf("input file or output file is empty\n");
        return -1;
    }
    return 0;
}

int main(int argc, char **argv)
{

    // default config
    struct fj_config config = {
        .time_limit_sec = 1,
        .ram_limit_mb = 8,
        .file_limit_mb = 128,
        .syscall_limit = 1,
        .exec_path = "",
        .exec_args = "",
        .input_file = "",
        .output_file = "",
        .answer_file = ""
    };
    

    // parse config
    if (parse_config(argc, argv, &config) != 0) {
        print_usage();
        return EXIT_CONFIG_INVALID;
    }
    
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
