#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

#define LINE_SIZE 512

// 创建子进程执行命令，父进程等待它结束
void
run(char *args[])
{
    int pid = fork();

    if (pid < 0) {
        fprintf(2, "xargs: fork failed\n");
        exit(1);
    }

    if (pid == 0) {
        exec(args[0], args);

        // exec 成功不会返回，执行到这里说明失败
        fprintf(2, "xargs: exec %s failed\n", args[0]);
        exit(1);
    }

    wait(0);
}

int
main(int argc, char *argv[])
{
    char *args[MAXARG];
    char line[LINE_SIZE];
    char c;
    int pos = 0;
    int n;

    if (argc < 2) {
        fprintf(2, "Usage: xargs command [args ...]\n");
        exit(1);
    }

    // 为追加的一行以及最后的空指针预留位置
    if (argc >= MAXARG) {
        fprintf(2, "xargs: too many arguments\n");
        exit(1);
    }

    // 跳过 argv[0]（xargs），复制要执行的命令和参数
    int count = 0;
    for (int i = 1; i < argc; i++) {
        args[count++] = argv[i];
    }

    // 最后一个参数放输入的行
    args[count] = line;
    args[count + 1] = 0;

    // 每次读取一个字符
    while ((n = read(0, &c, 1)) == 1) {
        if (c == '\n') {
            line[pos] = '\0';
            run(args);
            pos = 0;
        } else {
            if (pos >= LINE_SIZE - 1) {
                fprintf(2, "xargs: input line too long\n");
                exit(1);
            }

            line[pos++] = c;
        }
    }

    if (n < 0) {
        fprintf(2, "xargs: read failed\n");
        exit(1);
    }

    // 处理最后一行没有换行符的情况
    if (pos > 0) {
        line[pos] = '\0';
        run(args);
    }

    exit(0);
}