#define HLPERIOD0   0x000 // PWM0 低电平拍数
#define PERIOD0     0x004 // PWM0 方波周期拍数
#define HLPERIOD1   0x008 // PWM1 低电平拍数
#define PERIOD1     0x00c // PWM1 方波周期拍数
#define HLPERIOD2   0x010 // PWM2 低电平拍数
#define PERIOD2     0x014 // PWM2 方波周期拍数
#define HLPERIOD3   0x018 // PWM3 低电平拍数
#define PERIOD3     0x01c // PWM3 方波周期拍数
#define POLARITY    0x040 // PWM 模式设定
#define PWMSTART    0x044 // 使能 PWM 输出
#define PWMDONE     0x048 // PWM 结束状态
#define PWMUPDATE   0x04c // 动态加载 PWM 周期参数
#define PCOUNT0     0x050 // 设定 PWM0 脉冲数
#define PCOUNT1     0x054 // 设定 PWM1 脉冲数
#define PCOUNT2     0x058 // 设定 PWM2 脉冲数
#define PCOUNT3     0x05c // 设定 PWM3 脉冲数
#define PULSECOUNT0 0x060 // PWM0 已输出脉冲计数器状态
#define PULSECOUNT1 0x064 // PWM1 已输出脉冲计数器状态
#define PULSECOUNT2 0x068 // PWM2 已输出脉冲计数器状态
#define PULSECOUNT3 0x06c // PWM3 已输出脉冲计数器状态
#define SHIFTCOUNT0 0x080 // 同步模式 PWM0 初始相差
#define SHIFTCOUNT1 0x084 // 同步模式 PWM1 初始相差
#define SHIFTCOUNT2 0x088 // 同步模式 PWM2 初始相差
#define SHIFTCOUNT3 0x08c // 同步模式 PWM3 初始相差
#define SHIFTSTART  0x090 // PWM 同步模式使能
#define PWM_OE      0x0d0 // PWM IO output enable

#define SET_CHANNEL 0x1000
#define SET_PERIOD  0x1001
#define SET_DUTY    0x2000
#define SET_POLAR   0x3000
#define ENABLE      0x4000
#define UPDATE_DUTY 0x5000
