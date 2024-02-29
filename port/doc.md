# 移植文档

## 驱动

- i2c

  SOC 提供了 5 个 i2c controller，开发板从 controller 0、1、3 引出引脚。

  驱动支持每次控制 i2c 0、1、2、3、4 中的任意 1 个，ioctl 配置 slave 地址，并 read write 值。

  为了演示方便，测试示例只驱动 i2c 1。

- uart

  SOC 提供了 5 个 uart controller，开发板对 5 个 controller 都提供引脚。

  uart 0 与 console 相连，不被驱动。

  驱动支持每次控制 uart 1、2、3、4 中的任意 1 个，ioctl 配置 uart 参数，并 read write 值。

  为了演示方便，测试示例只驱动 uart 4。

- adc

  SOC 提供了 1 个 adc controller，开发板从这个 controller 引出引脚。

  驱动支持每次控制这个 controller 的任意 1 个 channel，并 read 值。

  为了演示方便，测试示例只驱动 channel 1。

- pwm

  SOC 提供了 4 个 pwm controller，开发板从 controller 1 和 2 引出引脚。

  驱动支持每次控制 controller 1 和 2 中 1 个 controller 的任意 1 个 channel，ioctl 配置并使能 channel。//TODO

  为了演示方便，测试示例只驱动 pwm5（pwm 1 的 channel 1）。

- gpio

  SOC 提供了 4 个 gpio controller，开发板从 4 个 controller 都有引出 gpio 引脚。

  驱动支持每次控制任意 1 个 controller 的任意 1 个 channel，ioctl 配置为输入或输出，并 read 或 write 值。

  为了演示方便，测试示例只驱动开发板上的 GP0（gpio 0 的 channel 28）和 GP25（gpio 2 的 channel 24）。

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
