#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include <stdarg.h>

char buf[1024];

int read_stdin(char* buf) {
    /*
    Description: Read stdin into buf
    Example:
        - read_stdin(); // Read the stdin into buf
    Parameters:
        - buf (char*): A buffer to store all characters
    Return:
        - 0 (int)
    */
    // Your code here
    // End
    int pos=0;
    while(pos<1023){
        char c;
        int n=read(0,&c,1);
        if(n<0){
            return -1;
        }
        if(n==0){
            break;
        }
        buf[pos++]=c;
        if(c=='\n'){
            break;
        }
    }
    buf[pos]='\0';
    return 0;
}

int log_stdout(uint i) {
    /*
    Description: Redirect stdout to a log file named i.log
    Example:
        - log_stdout(1); // Redirect the stdout to 1.log and return 0
    Parameters:
        - i (uint): A number
    Return:
        - 0 (int)
    */
    char log_name[15] = "0.log";
    // Your code here
    // End
    char digits[10];
    int n = 0;
    int pos = 0;

    // 将数字倒序转换成字符
    do {
        digits[n++] = '0' + i % 10;
        i /= 10;
    } while (i != 0);

    // 正序写入 log_name
    while (n > 0) {
        log_name[pos++] = digits[--n];
    }

    strcpy(log_name + pos, ".log");
    int fd=open(log_name,O_CREATE | O_WRONLY | O_TRUNC);
    if(fd<0){
        return -1;
    }
    if(fd == 1){
        return 0;
    }
    close(1);
    int newfd=dup(fd);
    close(fd);
    if(newfd!=1){
        if(newfd>=0){
            close(newfd);
        }
        return -1;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(2, "Usage: log_stdout number\n");
        exit(1);
    }
    if (log_stdout(atoi(argv[1])) != 0) {
        fprintf(2, "log_stdout: log_stdout failed\n");
        exit(1);
    }
    if (read_stdin(buf) != 0) {
        fprintf(2, "log_stdout: read_stdin failed\n");
        exit(1);
    }
    printf(buf);
    exit(0);
}
