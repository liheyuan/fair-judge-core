#ifndef FJ_COMMON_H
#define FJ_COMMON_H

#define MAX_STR 1024

struct fj_config {
    int cpu_time_limit_sec;
    int real_time_limit_sec;
    int ram_limit_mb;
    int file_limit_mb;

    char exec_path[MAX_STR];
    char exec_args[MAX_STR];
    char input_file[MAX_STR];
    char output_file[MAX_STR];
};

struct fj_timeout_args
{
    pid_t pid;
    int timeout;
};

#define EXIT_FORK_FAIL 2;
#define EXIT_SET_LIMIT_FAIL 3;
#define EXIT_EXEC_FAIL 4;
#define EXIT_REDIRECT_FAIL 5;
#define EXIT_CHROOT_FAIL 6;

#define ERR_OK 0;
#define ERR_CHILD_EXIT_NONZERO 101;
#define ERR_CHILD_EXIT_ABNORMALLY 102;
#define ERR_MEMORY_LIMIT_EXCEED 103;
#define ERR_TIME_LIMIT_EXCEED 104;
#define ERR_UNKNOWN_SIGNAL 105;



#endif