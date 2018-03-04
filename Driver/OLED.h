#ifndef __OLED_H__
#define __OLED_H__

#include "stm32f10x.h"
/* 命令 */
#define OLED_COMMAND 0x00
/* 数据 */
#define OLED_DATA 0x40
extern uint8_t oled_init[27],display_on[3],display_off[3];

/* 启动向OLED发送数据或者指令 */
void Driver_OLED_Send_START(uint32_t dataType);

/* 发送一串数据，数据中不包括地址 */
void Driver_OLED_SendDatas(uint8_t *commands, uint32_t length);

/* 屏幕输出 false：全黑，true全白，这是命令*/
void Driver_OLED_Fill(uint8_t data);

/* 清除一行,行距地址line 0-7 */
void Driver_OLED_ClearRow(uint8_t line);

/* 设置位置X：0-127，Y：0-7，这是命令*/
void Driver_OLED_Set_Position(uint8_t x, uint8_t y);

/* 显示字符，这是数据 */
void Driver_OLED_Show_Char(uint8_t x, uint8_t y, uint8_t chr, uint8_t charSize, uint8_t rever);

/* 显示字符串 */
void Driver_OLED_ShowString(uint8_t x,uint8_t y,uint8_t *chr,uint8_t Char_Size, uint8_t rever);

/* 显示显示BMP图片128×64起始点坐标(x,y),x的范围0～128，y为页的范围0～8 */
void Driver_OLED_DrawBMP(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,const uint8_t *BMP);

#endif
