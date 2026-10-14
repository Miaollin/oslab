#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include <stdarg.h>

void sub_process(int p_left[2], int i) __attribute__((noreturn));
int log_stdout(uint i) {
    /*
    Description: Redirect stdout to a log file named i.log.
    Example:
        - log_stdout(1); // Redirect the stdout to 1.log and return 0.
    Parameters:
        - i (uint): A number
    Return:
        - 0 (int)
    */
    char log_name[15] = "0.log";
    // Your code here
    uint base = 1, i_temp;
    if (i != 0) {
        for (base = 0, i_temp = i; i_temp != 0; ++base, i_temp /= 10);
        for (uint base_temp = 0, i_temp = i; i_temp != 0; ++base_temp, i_temp /= 10) {
            log_name[base - base_temp - 1] = '0' + i_temp % 10;
        }
        strcpy(log_name + base, ".log");
    }
    close(1);
    if (open(log_name, O_CREATE | O_WRONLY) != 1) {
        fprintf(2, "log_stdout: open failed\n");
        return -1;
    }
    // End
    return 0;
}

void
sub_process(int p_left[2], int i)
{
    close(p_left[1]);

    char prime, m;
    int p_right[2];
    int pid = -1;
    int num_read;

    num_read = read(p_left[0], &prime, 1);
    if (num_read != 1) {
        close(p_left[0]);
        exit(num_read < 0 ? 1 : 0);
    }

    if (log_stdout(i) < 0) {
        fprintf(2, "composites: log_stdout %d failed\n", i);
        close(p_left[0]);
        exit(1);
    }

    printf("prime %d\n", prime);

    while ((num_read = read(p_left[0], &m, 1)) == 1) {
        if (m % prime == 0) {
            // 能整除：由当前进程记录，不再向右传
            printf("composite %d\n", m);
        } else {
            // 第一次需要向右传数时，才创建下一个进程
            if (pid == -1) {
                if (pipe(p_right) < 0) {
                    fprintf(2, "composites: pipe failed\n");
                    close(p_left[0]);
                    exit(1);
                }

                pid = fork();

                if (pid < 0) {
                    fprintf(2, "composites: fork failed\n");
                    close(p_left[0]);
                    close(p_right[0]);
                    close(p_right[1]);
                    exit(1);
                }

                if (pid == 0) {
                    // 子进程使用新管道，不需要旧管道
                    close(p_left[0]);
                    sub_process(p_right, i + 1);
                }

                // 父进程只向右写，不从右边读
                close(p_right[0]);
            }

            // 用一个字节传递数字，而不是传递数字字符串
            if (write(p_right[1], &m, 1) != 1) {
                fprintf(2, "composites: write failed\n");
                close(p_left[0]);
                close(p_right[1]);
                wait(0);
                exit(1);
            }
        }
    }

    close(p_left[0]);

    if (pid != -1) {
        // 关闭写端，让右边进程读到 EOF
        close(p_right[1]);

        // 等待直接子进程；它也会等待自己的子进程
        wait(0);
    }

    if (num_read < 0) {
        fprintf(2, "composites: read failed\n");
        exit(1);
    }

    exit(0);
}


void
composites(void)
{
    int p_right[2];
    int pid;
    int failed = 0;

    if (pipe(p_right) < 0) {
        fprintf(2, "composites: pipe failed\n");
        exit(1);
    }

    pid = fork();

    if (pid < 0) {
        fprintf(2, "composites: fork failed\n");
        close(p_right[0]);
        close(p_right[1]);
        exit(1);
    }

    if (pid == 0) {
        // 第一个筛选进程，日志名为 0.log
        sub_process(p_right, 0);
    }

    // 主进程只负责发送数字
    close(p_right[0]);

    for (char m = 2; m <= 35; m++) {
        if (write(p_right[1], &m, 1) != 1) {
            fprintf(2, "composites: write failed\n");
            failed = 1;
            break;
        }
    }
    close(p_right[1]);

    // 等整个进程链结束
    wait(0);
    exit(failed);
}

int main(int argc, char* argv[]) {
    if (argc != 1) {
        fprintf(2, "Usage: composites\n");
        exit(1);
    }
    composites();
    exit(0);
}
