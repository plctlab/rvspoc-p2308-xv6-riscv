/*
 * Converted from https://github.com/Embedfire/embed_linux_tutorial/
                  blob/master/base_code/linux_app/i2c/mpu6050/sources/main.c
   Original author: pengjie
*/

#include "kernel/types.h"
#include "kernel/spinlock.h"
#include "kernel/sleeplock.h"
#include "kernel/fs.h"
#include "kernel/file.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define SMPLRT_DIV      0x19
#define CONFIG          0x1A
#define GYRO_CONFIG     0x1B
#define ACCEL_CONFIG    0x1C
#define ACCEL_XOUT_H    0x3B
#define ACCEL_XOUT_L    0x3C
#define ACCEL_YOUT_H    0x3D
#define ACCEL_YOUT_L    0x3E
#define ACCEL_ZOUT_H    0x3F
#define ACCEL_ZOUT_L    0x40
#define TEMP_OUT_H      0x41
#define TEMP_OUT_L      0x42
#define GYRO_XOUT_H     0x43
#define GYRO_XOUT_L     0x44
#define GYRO_YOUT_H     0x45
#define GYRO_YOUT_L     0x46
#define GYRO_ZOUT_H     0x47
#define GYRO_ZOUT_L     0x48
#define PWR_MGMT_1      0x6B
#define WHO_AM_I        0x75
#define SLAVE_ADDR      0x68       // MPU6050 地址
#define I2C_RETRIES     0x0701
#define I2C_TIMEOUT     0x0702
#define I2C_SLAVE       0x0703
#define I2C_BUS_MODE    0x0780

typedef unsigned char  uint8;
typedef unsigned short uint16;
int fd = -1;

uint8 i2c_write(int fd, uint8 reg, uint8 val)
{
    int retries;
    uint8 data[2];

    data[0] = reg;
    data[1] = val;

    for (retries = 5; retries; retries--) {
        if (write(fd, data, 2) == 2) {
            return 0;
        }

        // sleep for 0.4 seconds
        sleep(1);
    }

    return -1;
}

uint8 i2c_read(int fd, uint8 reg, uint8* val)
{
    int retries;

    for (retries = 5; retries; retries--) {

        if (write(fd, &reg, 1) == 1) {
            if (read(fd, val, 1) == 1) {
                return 0;
            }
        }
    }

    return -1;
}

uint8 mpu6050_init(void)
{
    fd = open("i2c-1", O_RDWR);
    if (fd < 0) {
        // device file, major num, minor num
        mknod("i2c-1", I2C, 1);
        fd = open("i2c-1", O_RDWR);
    }

    printf("open i2c-1 success\n");

    if (ioctl(fd, I2C_SLAVE, SLAVE_ADDR) < 0) { 
        printf("cannot set i2c device slave address\n");
        close(fd);
        exit(1);
    }

    printf("set slave address to 0x%x success!\n", SLAVE_ADDR);

    i2c_write(fd, PWR_MGMT_1, 0X00);
    i2c_write(fd, SMPLRT_DIV, 0X07);
    i2c_write(fd, CONFIG, 0X06);
    i2c_write(fd, ACCEL_CONFIG, 0X01);

    return 0;
}

uint16 mpu6050_get(uint8 reg)
{
    uint8 high, low;

    i2c_read(fd, reg, &high);

    sleep(1);

    i2c_read(fd, reg + 1, &low);

    return (high << 8) + low;
}

int main(void)
{
    mpu6050_init();

    // sleep for 0.4 seconds
    sleep(1);

    for (int i = 0; i < 10; i++) {
        sleep(1);
        printf("ACCE_X: %d\n", mpu6050_get(ACCEL_XOUT_H));
        sleep(1);
        printf("ACCE_Y: %d\n", mpu6050_get(ACCEL_YOUT_H));
        sleep(1);
        printf("ACCE_Z: %d\n", mpu6050_get(ACCEL_ZOUT_H));
        sleep(1);
        printf("GYRO_X: %d\n", mpu6050_get(GYRO_XOUT_H));
        sleep(1);
        printf("GYRO_Y: %d\n", mpu6050_get(GYRO_YOUT_H));
        sleep(1);
        printf("GYRO_Z: %d\n", mpu6050_get(GYRO_ZOUT_H));

        printf("\n");
        sleep(1);
    }

    close(fd);
    exit(0);
}
