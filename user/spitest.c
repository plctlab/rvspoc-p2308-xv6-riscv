#include "kernel/types.h"
#include "kernel/spinlock.h"
#include "kernel/sleeplock.h"
#include "kernel/fs.h"
#include "kernel/file.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "kernel/spi.h"

int main(void)
{
    int fd = open("spi-2", O_RDWR);
    if (fd < 0) {
        mknod("spi-2", SPI, 2);
        fd = open("spi-2", O_RDWR);
    }

    uint8 mode = SPI_MODE_2;
    uint32 speed = 5 * 1000 * 1000;

    int ret = ioctl(fd, SPI_IOC_WR_MODE, mode);
    if (ret == -1) exit(1);

    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, speed);
    if (ret == -1) exit(1);

    char tx_buf[] = "testing spi right now...";
    uint len = strlen(tx_buf);
    char rx_buf[len + 1];

    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx_buf,
        .rx_buf = (unsigned long)rx_buf,
        .len = len,
    };

    ret = ioctl(fd, SPI_IOC_MESSAGE | 1, (uint64) &tr);
    if (ret == -1) exit(1);

    rx_buf[len] = '\0';

    printf("tx_buffer: %s\n", tx_buf);
    printf("rx_buffer: %s\n", rx_buf);

    close(fd);
    exit(0);
}
