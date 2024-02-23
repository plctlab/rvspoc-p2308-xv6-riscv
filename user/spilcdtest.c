// init: The initial user-level program

#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/spinlock.h"
#include "kernel/sleeplock.h"
#include "kernel/fs.h"
#include "kernel/file.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int spilcd_fd;

struct pwm_ctrl {
  uint32_t period;
  uint32_t low_period;
  uint8_t enable;
};

#define SPILCD_CMD_RESET 0
#define SPILCD_CMD_WRITE_REG 1
#define SPILCD_CMD_WRITE_DATA 2

struct spilcd_ctrl {
  uint8_t command;

  union {
    uint8_t data;
  } u;
};

static void lcd_reset(int en) {
  struct spilcd_ctrl lcd;
  lcd.command = SPILCD_CMD_RESET;
  lcd.u.data = en;

  write(spilcd_fd, &lcd, sizeof(lcd));
}

static void lcd_write_reg(uint8_t data) {
  struct spilcd_ctrl lcd;
  int ret;
  lcd.command = SPILCD_CMD_WRITE_REG;
  lcd.u.data = data;

  ret = write(spilcd_fd, &lcd, sizeof(lcd));
  if (ret != sizeof(lcd)) {
    printf("lcd write failed!\n");
  }
}

static void lcd_write_data(uint8_t data) {
  struct spilcd_ctrl lcd;
  int ret;
  lcd.command = SPILCD_CMD_WRITE_DATA;
  lcd.u.data = data;

  ret = write(spilcd_fd, &lcd, sizeof(lcd));
  if (ret != sizeof(lcd)) {
    printf("lcd write failed!\n");
  }
}

#define USE_HORIZONTAL 2
#define LCD_WR_REG(x) lcd_write_reg(x)
#define LCD_WR_DATA8(x) lcd_write_data(x)
#define LCD_WR_DATA(x) do { lcd_write_data((x)>>8); lcd_write_data(x); }while(0)

#if USE_HORIZONTAL==0 || USE_HORIZONTAL==1
#define LCD_W 172
#define LCD_H 320
#else
#define LCD_W 320
#define LCD_H 172
#endif

void LCD_Init(void)
{
  lcd_reset(1);
  sleep(1000);
  lcd_reset(0);
  sleep(1000);
  lcd_reset(1);
  sleep(300);

  LCD_WR_REG(0x11);
//	delay_ms(120);
  LCD_WR_REG(0x36);

  if(USE_HORIZONTAL==0)LCD_WR_DATA8(0x00);
  else if(USE_HORIZONTAL==1)LCD_WR_DATA8(0xC0);
  else if(USE_HORIZONTAL==2)LCD_WR_DATA8(0x70);
  else LCD_WR_DATA8(0xA0);

  LCD_WR_REG(0x3A);
  LCD_WR_DATA8(0x05);

  LCD_WR_REG(0xB2);
  LCD_WR_DATA8(0x0C);
  LCD_WR_DATA8(0x0C);
  LCD_WR_DATA8(0x00);
  LCD_WR_DATA8(0x33);
  LCD_WR_DATA8(0x33);

  LCD_WR_REG(0xB7);
  LCD_WR_DATA8(0x35);

  LCD_WR_REG(0xBB);
  LCD_WR_DATA8(0x35);

  LCD_WR_REG(0xC0);
  LCD_WR_DATA8(0x2C);

  LCD_WR_REG(0xC2);
  LCD_WR_DATA8(0x01);

  LCD_WR_REG(0xC3);
  LCD_WR_DATA8(0x13);

  LCD_WR_REG(0xC4);
  LCD_WR_DATA8(0x20);

  LCD_WR_REG(0xC6);
  LCD_WR_DATA8(0x0F);

  LCD_WR_REG(0xD0);
  LCD_WR_DATA8(0xA4);
  LCD_WR_DATA8(0xA1);

  LCD_WR_REG(0xD6);
  LCD_WR_DATA8(0xA1);

  LCD_WR_REG(0xE0);
  LCD_WR_DATA8(0xF0);
  LCD_WR_DATA8(0x00);
  LCD_WR_DATA8(0x04);
  LCD_WR_DATA8(0x04);
  LCD_WR_DATA8(0x04);
  LCD_WR_DATA8(0x05);
  LCD_WR_DATA8(0x29);
  LCD_WR_DATA8(0x33);
  LCD_WR_DATA8(0x3E);
  LCD_WR_DATA8(0x38);
  LCD_WR_DATA8(0x12);
  LCD_WR_DATA8(0x12);
  LCD_WR_DATA8(0x28);
  LCD_WR_DATA8(0x30);

  LCD_WR_REG(0xE1);
  LCD_WR_DATA8(0xF0);
  LCD_WR_DATA8(0x07);
  LCD_WR_DATA8(0x0A);
  LCD_WR_DATA8(0x0D);
  LCD_WR_DATA8(0x0B);
  LCD_WR_DATA8(0x07);
  LCD_WR_DATA8(0x28);
  LCD_WR_DATA8(0x33);
  LCD_WR_DATA8(0x3E);
  LCD_WR_DATA8(0x36);
  LCD_WR_DATA8(0x14);
  LCD_WR_DATA8(0x14);
  LCD_WR_DATA8(0x29);
  LCD_WR_DATA8(0x32);

// 	LCD_WR_REG(0x2A);
//	LCD_WR_DATA8(0x00);
//	LCD_WR_DATA8(0x22);
//	LCD_WR_DATA8(0x00);
//	LCD_WR_DATA8(0xCD);
//	LCD_WR_DATA8(0x2B);
//	LCD_WR_DATA8(0x00);
//	LCD_WR_DATA8(0x00);
//	LCD_WR_DATA8(0x01);
//	LCD_WR_DATA8(0x3F);
//	LCD_WR_REG(0x2C);
  LCD_WR_REG(0x21);

  LCD_WR_REG(0x11);
  sleep(120);
  LCD_WR_REG(0x29);
}

void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
  if(USE_HORIZONTAL==0) {
    LCD_WR_REG(0x2a);
    LCD_WR_DATA(x1+34);
    LCD_WR_DATA(x2+34);
    LCD_WR_REG(0x2b);
    LCD_WR_DATA(y1);
    LCD_WR_DATA(y2);
    LCD_WR_REG(0x2c);
  }else if(USE_HORIZONTAL==1){
    LCD_WR_REG(0x2a);
    LCD_WR_DATA(x1+34);
    LCD_WR_DATA(x2+34);
    LCD_WR_REG(0x2b);
    LCD_WR_DATA(y1);
    LCD_WR_DATA(y2);
    LCD_WR_REG(0x2c);
  }else if(USE_HORIZONTAL==2){
    LCD_WR_REG(0x2a);
    LCD_WR_DATA(x1);
    LCD_WR_DATA(x2);
    LCD_WR_REG(0x2b);
    LCD_WR_DATA(y1+34);
    LCD_WR_DATA(y2+34);
    LCD_WR_REG(0x2c);
  }else{
    LCD_WR_REG(0x2a);
    LCD_WR_DATA(x1);
    LCD_WR_DATA(x2);
    LCD_WR_REG(0x2b);
    LCD_WR_DATA(y1+34);
    LCD_WR_DATA(y2+34);
    LCD_WR_REG(0x2c);
  }
}

void LCD_Fill(uint16_t xsta, uint16_t ysta, uint16_t xend,uint16_t yend, uint16_t color)
{
	uint16_t i,j;

	LCD_Address_Set(xsta,ysta,xend-1,yend-1);

	for(i=ysta;i<yend;i++) {
		for(j=xsta;j<xend;j++) {
			LCD_WR_DATA(color);
		}
	}
}

int
main(void)
{
  int pwm_fd;

  if((pwm_fd = open("pwm", O_RDWR)) < 0){
    mknod("pwm", PWM, 0);
    pwm_fd = open("pwm", O_RDWR);
  }

  if((spilcd_fd = open("spilcd", O_RDWR)) < 0){
    mknod("spilcd", SPI, 0);
    spilcd_fd = open("spilcd", O_RDWR);
  }

  if (pwm_fd < 0) {
    printf("pwm open failed!\n");
    exit(1);
  }

  if (spilcd_fd < 0) {
    printf("spilcd open failed!\n");
    exit(1);
  }

  printf("spilcd open success! pwm fd:%d, lcd fd:%d\n", pwm_fd, spilcd_fd);

  struct pwm_ctrl pwm;

  /* back light: 75% */
  pwm.period = 100;
  pwm.low_period = 25;
  pwm.enable = 1;
  write(pwm_fd, &pwm, sizeof(pwm));

  LCD_Init();

  while(1) {
    //printf("refresh\n");
    LCD_Fill(0, 0, 40, 40, 0xFFFF);
  }

  close(pwm_fd);
  close(spilcd_fd);

  printf("lcd test done!\n");

  exit(0);
}
