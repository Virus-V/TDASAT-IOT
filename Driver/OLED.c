/* OLED驱动，使用I2C1，地址是0x78 */
/* 2016年7月14日16:06:06 */
/* Virus.V */
#include <stdio.h>
#include "stm32f10x_conf.h"
#include "initialize.h"
#include "oledfont.h"
#include "OLED.h"

#define OLED_HARDWARE I2C2
#define _TRY(xx) while(xx)
               
uint8_t oled_init[] = {0xae, 0x00, 0x10, 0x40, 0xb0, 0x81, 0xff, 0xa1, 0xa6, 0xa8, 0x3f, 0xc8, 0xd3,0x00, 0xd5, 0x80, 0xd8, 0x05, 0xd9, 0xf1, 0xda, 0x12,0xdb, 0x30, 0x8d, 0x14, 0xaf};

uint8_t display_on[] = {0x8d, 0x14, 0xaf};
uint8_t display_off[] = {0x8d, 0x10, 0xae};

/* 启动向OLED发送数据或者指令 */
void Driver_OLED_Send_START(uint32_t dataType){
    
    /* 发送起始条件 */
    I2C_GenerateSTART(OLED_HARDWARE, ENABLE);
    /* 初始化重试次数 */
    /* 检查当前是否成功占用总线 */
    _TRY(I2C_CheckEvent(OLED_HARDWARE, I2C_EVENT_MASTER_MODE_SELECT) == ERROR);
    /* 发送地址，要往这个地址写数据 */
    I2C_Send7bitAddress(OLED_HARDWARE, 0x78, I2C_Direction_Transmitter);
    /* 检测当前是否是主发送模式 */
    _TRY(I2C_CheckEvent(OLED_HARDWARE, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) == ERROR);
    /* 发送指令：0x00代表指令，0x40代表数据 */
    if(dataType != OLED_COMMAND){
        I2C_SendData(OLED_HARDWARE, 0x40);
    }else{
        I2C_SendData(OLED_HARDWARE, 0x00);
    }
    _TRY(I2C_CheckEvent(OLED_HARDWARE, I2C_EVENT_MASTER_BYTE_TRANSMITTED) == ERROR);
    
}

/* 发送一串数据，数据中不包括地址 */
void Driver_OLED_SendDatas(uint8_t *commands, uint32_t length){
    
    uint32_t i;
    /* 循环发送数据 */
    for(i=0; i<length; i++){
        I2C_SendData(OLED_HARDWARE, commands[i]);
        //I2C_SendData(OLED_HARDWARE, *(commands + i));
        /* 等待数据发送完成 */
        _TRY(I2C_CheckEvent(OLED_HARDWARE, I2C_EVENT_MASTER_BYTE_TRANSMITTING) == ERROR);
    }
    /* 等待最后一个数据发送完 */
    _TRY(I2C_GetFlagStatus(OLED_HARDWARE, I2C_FLAG_BTF) == RESET);
    
}

/* 屏幕输出 false：全黑，true全白，这是命令*/
void Driver_OLED_Fill(uint8_t data){
    uint8_t i,j;
    /* 设置页地址0-7，设置显示位置—列低地址，设置显示位置—列高地址 */
    uint8_t command[] = {0x00, 0x10};
    /* 清理八页 */
    for(i=0;i<8;i++){
        /* 开始发送指令 */
        Driver_OLED_Send_START(OLED_COMMAND);
        /* 设置页地址0-7 */
        I2C_SendData(OLED_HARDWARE, 0xb0+i);
        _TRY(I2C_CheckEvent(OLED_HARDWARE, I2C_EVENT_MASTER_BYTE_TRANSMITTED) == ERROR);
        /* 发送剩下的指令 */
        Driver_OLED_SendDatas(command, sizeof(command));
        /* 开始发送数据 */
        Driver_OLED_Send_START(OLED_DATA);
        for(j=0;j<128;j++){
            /* 全黑 */
            I2C_SendData(OLED_HARDWARE, data);
            /* 等待数据发送完成 */
            _TRY(I2C_CheckEvent(OLED_HARDWARE, I2C_EVENT_MASTER_BYTE_TRANSMITTING) == ERROR);
        }
    }
    /* 等待最后一个数据发送完 */
    _TRY(I2C_GetFlagStatus(OLED_HARDWARE, I2C_FLAG_BTF) == RESET);
    
}

/* 清除一行,行距地址line 0-7 */
void Driver_OLED_ClearRow(uint8_t line){
    uint8_t j;
    /* 设置页地址0-7，设置显示位置—列低地址，设置显示位置—列高地址 */
    uint8_t command[] = {0x00, 0x10};
    /* 开始发送指令 */
    Driver_OLED_Send_START(OLED_COMMAND);
    /* 设置页地址0-7 */
    I2C_SendData(OLED_HARDWARE, 0xb0+line);
    _TRY(I2C_CheckEvent(OLED_HARDWARE, I2C_EVENT_MASTER_BYTE_TRANSMITTED) == ERROR);
    /* 发送剩下的指令 */
    Driver_OLED_SendDatas(command, sizeof(command));
    /* 开始发送数据 */
    Driver_OLED_Send_START(OLED_DATA);
    for(j=0;j<128;j++){
        /* 全黑 */
        I2C_SendData(OLED_HARDWARE, 0x0);
        /* 等待数据发送完成 */
        _TRY(I2C_CheckEvent(OLED_HARDWARE, I2C_EVENT_MASTER_BYTE_TRANSMITTING) == ERROR);
    }
    /* 等待最后一个数据发送完 */
    _TRY(I2C_GetFlagStatus(OLED_HARDWARE, I2C_FLAG_BTF) == RESET);
    
}

/* 设置位置X：0-127，Y：0-7，这是命令*/
void Driver_OLED_Set_Position(uint8_t x, uint8_t y){
    /* 发送列0-7 */
    I2C_SendData(OLED_HARDWARE, 0xb0 + y);
    _TRY(I2C_CheckEvent(OLED_HARDWARE, I2C_EVENT_MASTER_BYTE_TRANSMITTING) == ERROR);
    I2C_SendData(OLED_HARDWARE, ((x&0xf0)>>4)|0x10);
    _TRY(I2C_CheckEvent(OLED_HARDWARE, I2C_EVENT_MASTER_BYTE_TRANSMITTING) == ERROR);
    I2C_SendData(OLED_HARDWARE, ( x & 0x0f));
    _TRY(I2C_CheckEvent(OLED_HARDWARE, I2C_EVENT_MASTER_BYTE_TRANSMITTED) == ERROR);
    
}

/* 显示字符，这是数据 */
void Driver_OLED_Show_Char(uint8_t x, uint8_t y, uint8_t chr, uint8_t charSize, uint8_t rever){
    uint8_t charIndex = 0,i;
    charIndex = chr-0x20;
    /* 转换显示位置 */
    y = (x/128 + y) % 64;
    x = x%128;
    /* 选择显示文字大小 */
    if(charSize == 16){
        /* 发送第一行 */
        Driver_OLED_Send_START(OLED_COMMAND);
           
        Driver_OLED_Set_Position(x,y);
        Driver_OLED_Send_START(OLED_DATA);
        /* 发送字符数据 */
        for(i=0;i<8;i++) {
            
            I2C_SendData(OLED_HARDWARE, rever ? ~F8X16[charIndex*16+i] : F8X16[charIndex*16+i]);
            _TRY(I2C_CheckEvent(OLED_HARDWARE, I2C_EVENT_MASTER_BYTE_TRANSMITTING) == ERROR);
        }
        //发送第二行
        Driver_OLED_Send_START(OLED_COMMAND);
        Driver_OLED_Set_Position(x,y+1);
        Driver_OLED_Send_START(OLED_DATA);
        /* 发送字符数据 */
        for(i=0;i<8;i++){
            I2C_SendData(OLED_HARDWARE, F8X16[charIndex*16+i+8]);
            _TRY(I2C_CheckEvent(OLED_HARDWARE, I2C_EVENT_MASTER_BYTE_TRANSMITTING) == ERROR);
        }
    }else {	
        Driver_OLED_Send_START(OLED_COMMAND);
        Driver_OLED_Set_Position(x,y);
        Driver_OLED_Send_START(OLED_DATA);
        for(i=0;i<6;i++){
            I2C_SendData(OLED_HARDWARE, rever ? ~F6x8[charIndex][i] : F6x8[charIndex][i]);
            _TRY(I2C_CheckEvent(OLED_HARDWARE, I2C_EVENT_MASTER_BYTE_TRANSMITTING) == ERROR);
        }
    }
    /* 等待最后一个数据发送完 */
    _TRY(I2C_GetFlagStatus(OLED_HARDWARE, I2C_FLAG_BTF) == RESET);
    
}

/* 显示字符串 */
void Driver_OLED_ShowString(uint8_t x,uint8_t y,uint8_t *chr,uint8_t Char_Size, uint8_t rever){
    uint8_t j=0;
    while (chr[j] != '\0'){
        Driver_OLED_Show_Char(x,y,chr[j],Char_Size, rever);
        if(Char_Size == 16){
            x+=8;
            if(x>120){x=0;y+=2;}
        }else{
            x+=6;
            if(x>120){x=0;y+=1;}
        }
        j++;
	}
    
}

/* 显示显示BMP图片128×64起始点坐标(x,y),x的范围0～127，y为页的范围0～7 */
void Driver_OLED_DrawBMP(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,const uint8_t *BMP){ 	
    uint32_t j=0;
    uint8_t x,y;
    if(y1%8==0) y=y1/8;      
    else y=y1/8+1;
	for(y=y0;y<y1;y++){
        /* 开始发送命令 */
        Driver_OLED_Send_START(OLED_COMMAND);
		Driver_OLED_Set_Position(x0,y);
        /* 开始发送数据 */
        Driver_OLED_Send_START(OLED_DATA);
        for(x=x0;x<x1;x++){ 
            I2C_SendData(OLED_HARDWARE, BMP[j++]);
            _TRY(I2C_CheckEvent(OLED_HARDWARE, I2C_EVENT_MASTER_BYTE_TRANSMITTING) == ERROR);    	
	    }
        /* 等待最后一个数据发送完 */
        _TRY(I2C_GetFlagStatus(OLED_HARDWARE, I2C_FLAG_BTF) == RESET);
	}
    
}

