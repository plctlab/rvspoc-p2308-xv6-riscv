# 移植文档

## 烧录

执行根目录下的 script，提供 sd 卡的设备文件名，如 `./script /dev/sdb`，产生的二进制文件路径为 `port/image/output/milkv-duo.img`。

## 驱动

- i2c

  SOC 提供了 5 个 i2c controller，开发板从 controller 0、1、3 引出引脚。

  驱动支持每次控制 i2c 0、1、2、3、4 中的任意 1 个，ioctl 配置 slave 地址，并 read write 值。

  为了演示方便，测试示例只驱动 i2c 1。

  测试示例需要连接陀螺仪传感器 MPU6050，可以获得三轴加速度和角速度（数据为未经处理的原始数据）。

- uart

  SOC 提供了 5 个 uart controller，开发板对 5 个 controller 都提供引脚。

  uart 0 与 console 相连，不被驱动。

  驱动支持每次控制 uart 1、2、3、4 中的任意 1 个，ioctl 配置 uart 参数，并 read write 值。

  为了演示方便，测试示例只驱动 uart 4。

  测试示例需要把开发板的 uart 4 连接到电脑的串口工具中。一串字符会先被输出到串口工具中，然后我们可以在串口工具中打字，打字的长度由测试示例的第一个参数决定，串口工具中输入的字符会重新在串口工具中显示。

- adc

  SOC 提供了 1 个 adc controller，开发板从这个 controller 引出引脚。

  驱动支持每次控制这个 controller 的任意 1 个 channel，并 read 值。

  为了演示方便，测试示例只驱动 channel 1。

  测试示例会读 10 次 channel 1 的输入值，可以将 3.3v 输入 channel 1 然后取消输入，观察 channel 1 读取到的数字量的变化。

- pwm

  SOC 提供了 4 个 pwm controller，开发板从 controller 1 和 2 引出引脚。

  驱动支持每次控制任意 1 个 controller 的任意 1 个 channel，ioctl 配置并使能 channel。

  为了演示方便，测试示例只驱动 pwm5（pwm 1 的 channel 1）。

  测试示例需要连接 DF9GMS 180 微型舵机，可以对 pwm5 设置不同的 duty，使舵机的舵进行不同角度旋转。

- gpio

  SOC 提供了 4 个 gpio controller，开发板从 4 个 controller 都有引出 gpio 引脚。

  驱动支持每次控制任意 1 个 controller 的任意 1 个 channel，ioctl 配置为输入或输出，并 read 或 write 值。

  为了演示方便，测试示例只驱动开发板上的 GP0（gpio 0 的 channel 28）和 GP25（gpio 2 的 channel 24）。

  测试示例会先使开发板的蓝色 led 进行闪烁，然后读取 10 次 GP0 处的输入。GP0 的原始输入是 1，接地后输入为 0。

- spi

  SOC 提供了 4 个 spi controller，开发板从 controller 2 引出引脚。

  驱动支持每次控制任意 1 个 controller，ioctl 配置模式与频率，并启动读写。

  为了演示方便，测试示例只驱动 spi 2。

  测试示例为 spi 回环测试，需要短接 spi 2 的两个数据引脚。发出的数据会一摸一样得被接收到。

### Implementation Notes

#### I2C

- read(fd, val, 2)

  read -> i2cdev_read -> i2c_transfer -> i2c_dw_xfer -> i2c_dw_xfer_init

  处理 syscall 的时候 cpu 允许中断，xfer_init 里设备开中断以后马上产生 tx empty 中断，进 kernel_vec 处理，kernel_vec 里 cpu 不允许中断

  kernel_vec -> dev_intr -> i2c_dw_intr -> i2c_dw_xfer_msg

  i2c_dw_xfer_msg 中，往 DATA_CMD 里写 0x100，再往 DATA_CMD 里写 0x300（含 STOP），这两条命令进入 tx fifo

  master 根据第一条 0x100，开始发送 start、slave address、read

  slave 会发送 ack、data，一个 data 进入 rx fifo，设备直接产生 rx full 中断

  master 处理第二条 0x300，另一个 data 进入 rx fifo，rx full 中断一直在，并且设备的 stop det 中断也开了

  kernel_vec 处理好以后，sret 到 xfer_init，sret 使 SIE 为 1，cpu 允许中断，设备的中断马上被发现，又进入 kernel_vec，kernel_vec 里 cpu 不允许中断

  kernel_vec -> dev_intr -> i2c_dw_intr -> i2c_dw_read

  i2c_dw_read 里从 rx fifo 取出数据，rx full 中断消失

- write(fd, &reg, 2)

  write -> i2cdev_write -> i2c_transfer -> i2c_dw_xfer -> i2c_dw_xfer_init

  处理 syscall 的时候 cpu 允许中断，xfer_init 里设备开中断以后马上产生 tx empty 中断，进 kernel_vec 处理，kernel_vec 里 cpu 不允许中断

  kernel_vec -> dev_intr -> i2c_dw_intr -> i2c_dw_xfer_msg

  i2c_dw_xfer_msg 中，往 DATA_CMD 里写 000、data，把 msg 数组的所有 msg 处理完，每一次往 DATA_CMD 里写的内容都会放进 tx fifo

  在 i2c_dw_xfer_msg 最后遮住 tx empty 中断，中断处理完毕，在 i2c_dw_xfer 里 sleep

  当 tx fifo 里内容全部发出去后，最后会发一个 stop 信号，设备会产生一个 stop det 中断，再次进入 dev_intr，这时 cpu 不允许中断

  dev_intr -> i2c_dw_intr -> i2c_dw_irq_handler

  i2c_dw_irq_handler 中检测到 stop det 中断并清除，wakeup i2c_dw_xfer

  i2c_dw_xfer 醒了以后 disable 设备，返回传输的 msg 个数，大功告成
