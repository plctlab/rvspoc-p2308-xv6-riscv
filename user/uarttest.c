
#include "kernel/types.h"
#include "kernel/spinlock.h"
#include "kernel/sleeplock.h"
#include "kernel/fs.h"
#include "kernel/file.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/uart_dev.h"

int main(int argc, char **argv)
{
    if (argc == 1) {
        printf("please invoke %s with expected read length for uart 4\n", argv[0]);
        printf("like: %s 10\n", argv[0]);
        exit(1);
    }

    int fd = -1;

    if (open("ttyS4", O_RDWR) < 0) {
        mknod("ttyS4", UART, 4);
        fd = open("ttyS4", O_RDWR);
    }

    if (fd < 0) {
        printf("cannot open ttyS4\n");
        exit(1);
    }

    printf("open ttyS4 success\n");

    struct uartconfig config = {115200, 8, 1, 'n'};

    if (ioctl(fd, SET_CONFIG, (uint64)(&config)) < 0) {
        printf("cannot config uart 4\n");
        close(fd);
        exit(1);
    }

    printf("config uart 4 success!\n");

    char output[] = "uart 4 test\n";
    if (write(fd, output, strlen(output)) != strlen(output)) {
        printf("write error\n");
        close(fd);
        exit(1);
    }

    int expected_input_len = atoi(argv[1]);
    char input[expected_input_len];
    if (read(fd, input, expected_input_len) != expected_input_len){
        printf("read error\n");
        close(fd);
        exit(1);
    }

    write(fd, "\nuart 4 input:", strlen("\n uart 4 input:"));
    write(fd, input, expected_input_len);

    close(fd);

    printf("uart test successful!\n");

    exit(0);
}