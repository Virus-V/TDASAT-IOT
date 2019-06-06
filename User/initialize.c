/* 
 * File: /home/virusv/TDSAST_IOT/User/initialize.c
 * Project: /home/virusv/TDSAST_IOT
 * Created Date: Monday, September 25th 2017, 8:49:34 pm
 * Author: Virus.V
 * E-mail: virusv@qq.com
 * -----
 * Modified By: Virus.V
 * Last Modified: Sun Oct 01 2017
 * -----
 * Copyright (c) 2017 TD-SAST
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *  
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *  
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stm32f10x.h"
#include "initialize.h"

void Init_i2c1_oled(void){
    GPIO_InitTypeDef OLED_GPIO_InitStructure;
    I2C_InitTypeDef OLED_I2C_InitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    OLED_GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    OLED_GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    OLED_GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(GPIOB, &OLED_GPIO_InitStructure);

    I2C_DeInit(I2C2); //这句一定要加
    OLED_I2C_InitStructure.I2C_ClockSpeed = 270000;
    OLED_I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    OLED_I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_16_9;
    OLED_I2C_InitStructure.I2C_OwnAddress1 = 0x45;
    OLED_I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    OLED_I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; //响应七位地址
    I2C_Init(I2C2, &OLED_I2C_InitStructure);
    /* 启动时钟拉长 */
    I2C_StretchClockCmd(I2C2, ENABLE);
    I2C_Cmd(I2C2, ENABLE);
}

// 蓝色灯泡引脚和锁控制引脚
void Init_led_lock(void){
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  	
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    // 熄灭灯
    GPIO_SetBits(GPIOB, GPIO_Pin_9);
    GPIO_ResetBits(GPIOB, GPIO_Pin_8);  //开锁
}

//w5500网卡
void Init_spi1_w5500(void){
    SPI_InitTypeDef SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    /* 打开SPI1外设和GPIOA的时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOA , ENABLE);
    /*****GPIOA******
     3.PA5=>SPI1_SCK,Master,Alternate function push-pull
     4.PA6=>SPI1_MISO,Master,Input floating / Input pull-up
     5.PA7=>SPI1_MOSI,Master,Alternate function push-pull
     6.PA3=>W5500_RST,Output Push-pull
     7.PA11=>W5500_INT,上拉输入
     8.PA4=>W5500_SCS,推挽输出
    *****************/
    /* SPI1配置 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* W5500_RST引脚初始化配置(PA3) */
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOA, GPIO_Pin_3);//复位引脚拉低,复位

    /* W5500_INT引脚初始化配置(PA11) */	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* SPI1配置 */
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //双线全双工
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master; //作为主机
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; //传输数据为8位
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High; //时钟初始极性为高
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge; //在第二个跳变同步，W5500总是在上升沿同步数据
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; //软件管理NSS
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;//SPI_BaudRatePrescaler_2;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7; //CRC
    SPI_Init(SPI1, &SPI_InitStructure);
    /* 启动SPI1 */
    SPI_Cmd(SPI1, ENABLE);
    GPIO_SetBits(GPIOA, GPIO_Pin_3);//复位引脚拉高,取消复位
}

// TIM2提供1ms时基
void Init_tim2_ms_base(void){
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    /* 初始化TIM */
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 9;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    //使能Timer2时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_TimeBaseStructure.TIM_Period = 9;						//设置在下一个更新事件装入活动的自动重装载寄存器周期的值(计数到10为1ms)
	TIM_TimeBaseStructure.TIM_Prescaler = 7199;					//设置用来作为TIMx时钟频率除数的预分频值(10KHz的计数频率)
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//设置时钟分割:TDTS = TIM_CKD_DIV1
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);				//根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); //使能TIM2指定的中断
	TIM_Cmd(TIM2, ENABLE); //使能TIMx外设
}

// 独立看门狗，防止服务器连接意外断开
void Init_iwdg_reset(void){
    // 使能独立看门狗使用的LSI时钟
    RCC_LSICmd(ENABLE);
    // 等待时钟稳定
    while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);
    // 在调试的时候暂停看门狗
    DBGMCU_Config(DBGMCU_IWDG_STOP, ENABLE);
    // 开启独立看门狗，使能访问
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    /**
     * 设置预分频
     * 独立看门狗是内部一个独立的40KHz的震荡源提供频率
     * 记录时间为 = (时钟频率(40KHz) / 分频数)* IWDG_SetReload(t)
     * 最大记录时间 = (1/40K) * 256 * 0xFFF = 26.208秒
     */
    IWDG_SetPrescaler(IWDG_Prescaler_256);
    // 0x30D 等于 4.9984 秒，约为5秒
    IWDG_SetReload(0xFFF);
    // 重置计数器
    IWDG_ReloadCounter();
    // 启动看门狗
    IWDG_Enable();
}
