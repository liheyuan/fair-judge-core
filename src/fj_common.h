#ifndef FJ_COMMON_H
#define FJ_COMMON_H

#define MAX_STR_LEN 1024

struct fj_config {
    int time_limit_sec;
    int ram_limit_mb;
    int file_limit_mb;
    int syscall_limit;

    char exec_path[MAX_STR_LEN];
    char exec_args[MAX_STR_LEN];
    char input_file[MAX_STR_LEN];
    char output_file[MAX_STR_LEN];
    char answer_file[MAX_STR_LEN];
};

struct fj_timeout_args
{
    pid_t pid;
    int timeout;
};

#define EXIT_CONFIG_INVALID 1
#define EXIT_FORK_FAIL 2
#define EXIT_SET_LIMIT_FAIL 3
#define EXIT_EXEC_FAIL 4
#define EXIT_REDIRECT_FAIL 5
#define EXIT_CHROOT_FAIL 6
#define EXIT_SET_SEC_FAIL 7

#define ERR_OK 0
#define ERR_CHILD_EXIT_NONZERO 101
#define ERR_CHILD_EXIT_ABNORMALLY 102
#define ERR_MEMORY_LIMIT_EXCEED 103
#define ERR_TIME_LIMIT_EXCEED 104
#define ERR_UNKNOWN_SIGNAL 105
#define ERR_WRONG_ANSWER 106
#define ERR_PRESENTATION_ERROR 107
#define ERR_SYSTEM_CALL 108


#endif