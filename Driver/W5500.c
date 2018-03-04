#include <stdint.h>
#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "W5500.h"

/***************----- 网络参数变量定义 -----***************/
uint8_t Gateway_IP[4];	//网关IP地址 
uint8_t Sub_Mask[4];	//子网掩码 
uint8_t Phy_Addr[6];	//物理地址(MAC) 
uint8_t IP_Addr[4];		//本机IP地址 

uint8_t UDP_DIPR[4];	//UDP(广播)模式,目的主机IP地址
uint8_t UDP_DPORT[2];	//UDP(广播)模式,目的主机端口号

/***************----- 端口数据缓冲区 -----***************/
uint8_t Rx_Buffer[2048];	//端口接收数据缓冲区 
uint8_t Tx_Buffer[2048];	//端口发送数据缓冲区 

__IO uint8_t W5500_Interrupt;	//W5500中断标志(0:无中断,1:有中断)

extern void delay_ms(uint32_t x);
/*******************************************************************************
* 函数名  : SPI1_Send_Byte
* 描述    : SPI1发送1个字节数据
* 输入    : dat:待发送的数据
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void SPI1_Send_Byte(uint8_t dat){
	SPI_I2S_SendData(SPI1,dat);//写1个字节数据
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);//等待数据寄存器空
}

/*******************************************************************************
* 函数名  : SPI1_Send_Short
* 描述    : SPI1发送2个字节数据(16位)
* 输入    : dat:待发送的16位数据
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void SPI1_Send_Short(uint16_t dat){
    //SPI1_Send_Byte(dat<<8); //写数据高位
	SPI1_Send_Byte(dat>>8); //写数据高位
	SPI1_Send_Byte(dat);	//写数据低位
    
}

/*******************************************************************************
* 函数名  : Write_W5500_1Byte
* 描述    : 通过SPI1向指定地址寄存器写1个字节数据
* 输入    : reg:16位寄存器地址,dat:待写入的数据
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/

void Write_W5500_1Byte(uint16_t reg, uint8_t dat){
	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为低电平

	SPI1_Send_Short(reg);//通过SPI1写16位寄存器地址
	//SPI1_Send_Byte(FDM1|RWB_WRITE|COMMON_R);//通过SPI1写控制字节,1个字节数据长度,写数据,选择通用寄存器
    SPI1_Send_Byte(VDM|RWB_WRITE|COMMON_R);//通过SPI1写控制字节,1个字节数据长度,写数据,选择通用寄存器
	SPI1_Send_Byte(dat);//写1个字节数据

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); //置W5500的SCS为高电平
}

/*******************************************************************************
* 函数名  : Write_W5500_2Byte
* 描述    : 通过SPI1向指定地址寄存器写2个字节数据
* 输入    : reg:16位寄存器地址,dat:16位待写入的数据(2个字节)
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_W5500_2Byte(uint16_t reg, uint16_t dat){
	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为低电平
		
	SPI1_Send_Short(reg);//通过SPI1写16位寄存器地址
	SPI1_Send_Byte(VDM|RWB_WRITE|COMMON_R);//通过SPI1写控制字节,2个字节数据长度,写数据,选择通用寄存器  FDM2
	SPI1_Send_Short(dat);//写16位数据

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); //置W5500的SCS为高电平
}

/*******************************************************************************
* 函数名  : Write_W5500_nByte
* 描述    : 通过SPI1向指定地址寄存器写n个字节数据
* 输入    : reg:16位寄存器地址,*dat_ptr:待写入数据缓冲区指针,size:待写入的数据长度
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_W5500_nByte(uint16_t reg, uint8_t *dat_ptr, uint16_t size){
	uint16_t i;

	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为低电平	

	SPI1_Send_Short(reg);//通过SPI1写16位寄存器地址
	SPI1_Send_Byte(VDM|RWB_WRITE|COMMON_R);//通过SPI1写控制字节,N个字节数据长度,写数据,选择通用寄存器

	for(i=0;i<size;i++){
        //循环将缓冲区的size个字节数据写入W5500
		SPI1_Send_Byte(*dat_ptr++);//写一个字节数据
	}

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); //置W5500的SCS为高电平
}

/*******************************************************************************
* 函数名  : Write_W5500_SOCK_1Byte
* 描述    : 通过SPI1向指定端口寄存器写1个字节数据
* 输入    : s:端口号,reg:16位寄存器地址,dat:待写入的数据
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_W5500_SOCK_1Byte(SOCKET s, uint16_t reg, uint8_t dat){
	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为低电平	
		
	SPI1_Send_Short(reg);//通过SPI1写16位寄存器地址
	SPI1_Send_Byte(VDM|RWB_WRITE|(s*0x20+0x08));//通过SPI1写控制字节,1个字节数据长度,写数据,选择端口s的寄存器 FDM1
	SPI1_Send_Byte(dat);//写1个字节数据

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); //置W5500的SCS为高电平
}

/*******************************************************************************
* 函数名  : Write_W5500_SOCK_2Byte
* 描述    : 通过SPI1向指定端口寄存器写2个字节数据
* 输入    : s:端口号,reg:16位寄存器地址,dat:16位待写入的数据(2个字节)
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_W5500_SOCK_2Byte(SOCKET s, uint16_t reg, uint16_t dat){
	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为低电平
			
	SPI1_Send_Short(reg);//通过SPI1写16位寄存器地址
	SPI1_Send_Byte(VDM|RWB_WRITE|(s*0x20+0x08));//通过SPI1写控制字节,2个字节数据长度,写数据,选择端口s的寄存器 FDM2
	SPI1_Send_Short(dat);//写16位数据

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); //置W5500的SCS为高电平
}

/*******************************************************************************
* 函数名  : Write_W5500_SOCK_4Byte
* 描述    : 通过SPI1向指定端口寄存器写4个字节数据
* 输入    : s:端口号,reg:16位寄存器地址,*dat_ptr:待写入的4个字节缓冲区指针
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_W5500_SOCK_4Byte(SOCKET s, uint16_t reg, uint8_t *dat_ptr){
	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为低电平
			
	SPI1_Send_Short(reg);//通过SPI1写16位寄存器地址
	SPI1_Send_Byte(VDM|RWB_WRITE|(s*0x20+0x08));//通过SPI1写控制字节,4个字节数据长度,写数据,选择端口s的寄存器  FDM4

	SPI1_Send_Byte(*dat_ptr++);//写第1个字节数据
	SPI1_Send_Byte(*dat_ptr++);//写第2个字节数据
	SPI1_Send_Byte(*dat_ptr++);//写第3个字节数据
	SPI1_Send_Byte(*dat_ptr++);//写第4个字节数据

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); //置W5500的SCS为高电平
}

/*******************************************************************************
* 函数名  : Read_W5500_1Byte
* 描述    : 读W5500指定地址寄存器的1个字节数据
* 输入    : reg:16位寄存器地址
* 输出    : 无
* 返回值  : 读取到寄存器的1个字节数据
* 说明    : 无
*******************************************************************************/
uint8_t Read_W5500_1Byte(uint16_t reg){
	uint8_t i;
	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为低电平
			
	SPI1_Send_Short(reg);//通过SPI1写16位寄存器地址
	SPI1_Send_Byte(VDM|RWB_READ|COMMON_R);//通过SPI1写控制字节,1个字节数据长度,读数据,选择通用寄存器 FDM1

	i = SPI_I2S_ReceiveData(SPI1);
	SPI1_Send_Byte(0x00);//发送一个哑数据
	i = SPI_I2S_ReceiveData(SPI1);//读取1个字节数据

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为高电平
	return i;//返回读取到的寄存器数据
}

/*******************************************************************************
* 函数名  : Read_W5500_SOCK_1Byte
* 描述    : 读W5500指定端口寄存器的1个字节数据
* 输入    : s:端口号,reg:16位寄存器地址
* 输出    : 无
* 返回值  : 读取到寄存器的1个字节数据
* 说明    : 无
*******************************************************************************/
uint8_t Read_W5500_SOCK_1Byte(SOCKET s, uint16_t reg){
	uint8_t i;
	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为低电平
			
	SPI1_Send_Short(reg);//通过SPI1写16位寄存器地址
	SPI1_Send_Byte(VDM|RWB_READ|(s*0x20+0x08));//通过SPI1写控制字节,1个字节数据长度,读数据,选择端口s的寄存器 FDM1

	i = SPI_I2S_ReceiveData(SPI1);
	SPI1_Send_Byte(0x00);//发送一个哑数据
	i = SPI_I2S_ReceiveData(SPI1);//读取1个字节数据

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为高电平
	return i;//返回读取到的寄存器数据
}

/*******************************************************************************
* 函数名  : Read_W5500_SOCK_2Byte
* 描述    : 读W5500指定端口寄存器的2个字节数据
* 输入    : s:端口号,reg:16位寄存器地址
* 输出    : 无
* 返回值  : 读取到寄存器的2个字节数据(16位)
* 说明    : 无
*******************************************************************************/
uint16_t Read_W5500_SOCK_2Byte(SOCKET s, uint16_t reg){
	uint16_t i;

	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为低电平
			
	SPI1_Send_Short(reg);//通过SPI1写16位寄存器地址
	SPI1_Send_Byte(VDM|RWB_READ|(s*0x20+0x08));//通过SPI1写控制字节,2个字节数据长度,读数据,选择端口s的寄存器  FDM2
    //TODO:这儿有问题
	i = SPI_I2S_ReceiveData(SPI1);
	SPI1_Send_Byte(0x00);//发送一个哑数据
	i = SPI_I2S_ReceiveData(SPI1);//读取高位数据
	SPI1_Send_Byte(0x00);//发送一个哑数据
	i = (i << 8) | SPI_I2S_ReceiveData(SPI1);  

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为高电平
	return i;//返回读取到的寄存器数据
}

/*******************************************************************************
* 函数名  : Read_SOCK_Data_Buffer
* 描述    : 从W5500接收数据缓冲区中读取数据
* 输入    : s:端口号,*dat_ptr:数据保存缓冲区指针
* 输出    : 无
* 返回值  : 读取到的数据长度,rx_size个字节
* 说明    : 无
*******************************************************************************/
uint16_t Read_SOCK_Data_Buffer(SOCKET s, uint8_t *dat_ptr){
	uint16_t rx_size, offset, offset1, i;
	uint8_t j;

	rx_size = Read_W5500_SOCK_2Byte(s,Sn_RX_RSR);
	if(rx_size == 0) return 0;//没接收到数据则返回
	if(rx_size > 1460) rx_size = 1460;

	offset = Read_W5500_SOCK_2Byte(s,Sn_RX_RD);
	offset1 = offset;
	offset &= (S_RX_SIZE - 1);  //计算实际的物理地址

	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);  //置W5500的SCS为低电平

	SPI1_Send_Short(offset);  //写16位地址
	SPI1_Send_Byte(VDM|RWB_READ|(s*0x20+0x18));  //写控制字节,N个字节数据长度,读数据,选择端口s的寄存器
	j = SPI_I2S_ReceiveData(SPI1);
	
	if((offset+rx_size) < S_RX_SIZE){
        //如果最大地址未超过W5500接收缓冲区寄存器的最大地址
		for(i = 0;i < rx_size; i++){
            //循环读取rx_size个字节数据
			SPI1_Send_Byte(0x00);//发送一个哑数据
			j=SPI_I2S_ReceiveData(SPI1);//读取1个字节数据
			*dat_ptr++=j;//将读取到的数据保存到数据保存缓冲区
			//dat_ptr++;//数据保存缓冲区指针地址自增1
		}
	}else{
        //如果最大地址超过W5500接收缓冲区寄存器的最大地址
		offset = S_RX_SIZE-offset;
		for(i=0; i < offset; i++){
            //循环读取出前offset个字节数据
			SPI1_Send_Byte(0x00);//发送一个哑数据
			j = SPI_I2S_ReceiveData(SPI1);//读取1个字节数据
            //TODO:这儿改了
			*dat_ptr++ = j;//将读取到的数据保存到数据保存缓冲区
			//dat_ptr++;//数据保存缓冲区指针地址自增1
		}
		GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); //置W5500的SCS为高电平

		GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为低电平

		SPI1_Send_Short(0x00);//写16位地址
		SPI1_Send_Byte(VDM|RWB_READ|(s*0x20+0x18));//写控制字节,N个字节数据长度,读数据,选择端口s的寄存器
		j=SPI_I2S_ReceiveData(SPI1);

		for(;i<rx_size;i++){
            //循环读取后rx_size-offset个字节数据
			SPI1_Send_Byte(0x00);//发送一个哑数据
			j=SPI_I2S_ReceiveData(SPI1);//读取1个字节数据
            //TODO:这个修改了
			*dat_ptr++=j;//将读取到的数据保存到数据保存缓冲区
			//dat_ptr++;//数据保存缓冲区指针地址自增1
		}
	}
	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); //置W5500的SCS为高电平

	offset1 += rx_size;//更新实际物理地址,即下次读取接收到的数据的起始地址
	Write_W5500_SOCK_2Byte(s, Sn_RX_RD, offset1);
	Write_W5500_SOCK_1Byte(s, Sn_CR, RECV);//发送启动接收命令
	return rx_size;//返回接收到数据的长度
}

/*******************************************************************************
* 函数名  : Write_SOCK_Data_Buffer
* 描述    : 将数据写入W5500的数据发送缓冲区
* 输入    : s:端口号,*dat_ptr:数据保存缓冲区指针,size:待写入数据的长度
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_SOCK_Data_Buffer(SOCKET s, uint8_t *dat_ptr, uint16_t size){
	uint16_t offset, offset1, i;

	//如果是UDP模式,可以在此设置目的主机的IP和端口号
	if((Read_W5500_SOCK_1Byte(s,Sn_MR)&0x0f) != SOCK_UDP){
        //如果Socket打开失败
		Write_W5500_SOCK_4Byte(s, Sn_DIPR, UDP_DIPR);//设置目的主机IP  		
		Write_W5500_SOCK_2Byte(s, Sn_DPORTR, UDP_DPORT[0]*256 + UDP_DPORT[1]);//设置目的主机端口号				
	}

	offset = Read_W5500_SOCK_2Byte(s,Sn_TX_WR);
	offset1 = offset;
	offset &= (S_TX_SIZE-1); //计算实际的物理地址

	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为低电平

	SPI1_Send_Short(offset);//写16位地址
	SPI1_Send_Byte(VDM|RWB_WRITE|(s*0x20+0x10));//写控制字节,N个字节数据长度,写数据,选择端口s的寄存器

	if((offset+size) < S_TX_SIZE){
        //如果最大地址未超过W5500发送缓冲区寄存器的最大地址
		for(i=0;i<size;i++){
            //循环写入size个字节数据
			SPI1_Send_Byte(*dat_ptr++);//写入一个字节的数据		
		}
	}else{
        //如果最大地址超过W5500发送缓冲区寄存器的最大地址
		offset = S_TX_SIZE-offset;
		for(i=0; i < offset; i++){
            //循环写入前offset个字节数据
			SPI1_Send_Byte(*dat_ptr++);//写入一个字节的数据
		}
		GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); //置W5500的SCS为高电平

		GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为低电平

		SPI1_Send_Short(0x00);//写16位地址
		SPI1_Send_Byte(VDM|RWB_WRITE|(s*0x20+0x10));//写控制字节,N个字节数据长度,写数据,选择端口s的寄存器

		for(;i<size;i++){
            //循环写入size-offset个字节数据
			SPI1_Send_Byte(*dat_ptr++);//写入一个字节的数据
		}
	}
	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); //置W5500的SCS为高电平

	offset1 += size;//更新实际物理地址,即下次写待发送数据到发送数据缓冲区的起始地址
	Write_W5500_SOCK_2Byte(s, Sn_TX_WR, offset1);
	Write_W5500_SOCK_1Byte(s, Sn_CR, SEND);//发送启动发送命令				
}

/*******************************************************************************
* 函数名  : W5500_Hardware_Reset
* 描述    : 硬件复位W5500
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : W5500的复位引脚保持低电平至少500us以上,才能重围W5500
*******************************************************************************/
void W5500_Hardware_Reset(void){
    //printf("HR start\n");
	GPIO_ResetBits(W5500_RST_PORT, W5500_RST);//复位引脚拉低
	delay_ms(50);
	GPIO_SetBits(W5500_RST_PORT, W5500_RST);//复位引脚拉高
	delay_ms(200);
	while((Read_W5500_1Byte(PHYCFGR)&LINK)==0);//等待以太网连接完成
}

/*******************************************************************************
* 函数名  : W5500_Init
* 描述    : 初始化W5500寄存器函数
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 在使用W5500之前，先对W5500初始化
*******************************************************************************/
void W5500_Init(void){
	uint8_t i=0;

	Write_W5500_1Byte(MR, RST);//软件复位W5500,置1有效,复位后自动清0
	delay_ms(10);//延时10ms,自己定义该函数

	//设置网关(Gateway)的IP地址,Gateway_IP为4字节unsigned char数组,自己定义 
	//使用网关可以使通信突破子网的局限，通过网关可以访问到其它子网或进入Internet
	Write_W5500_nByte(GAR, Gateway_IP, 4);

	//设置子网掩码(MASK)值,SUB_MASK为4字节unsigned char数组,自己定义
	//子网掩码用于子网运算
	Write_W5500_nByte(SUBR, Sub_Mask, 4);
	
	//设置物理地址,PHY_ADDR为6字节unsigned char数组,自己定义,用于唯一标识网络设备的物理地址值
	//该地址值需要到IEEE申请，按照OUI的规定，前3个字节为厂商代码，后三个字节为产品序号
	//如果自己定义物理地址，注意第一个字节必须为偶数
	Write_W5500_nByte(SHAR, Phy_Addr, 6);

	//设置本机的IP地址,IP_ADDR为4字节unsigned char数组,自己定义
	//注意，网关IP必须与本机IP属于同一个子网，否则本机将无法找到网关
	Write_W5500_nByte(SIPR, IP_Addr, 4);
	
	//设置发送缓冲区和接收缓冲区的大小，参考W5500数据手册
	for(i=0;i<8;i++){
		Write_W5500_SOCK_1Byte(i, Sn_RXBUF_SIZE, 0x02); //Socket Rx memory size=2k
		Write_W5500_SOCK_1Byte(i, Sn_TXBUF_SIZE, 0x02); //Socket Tx mempry size=2k
	}

	//设置重试时间，默认为2000(200ms) 
	//每一单位数值为100微秒,初始化时值设为2000(0x07D0),等于200毫秒
	Write_W5500_2Byte(RTR, 0x7d0);

	//设置重试次数，默认为8次 
	//如果重发的次数超过设定值,则产生超时中断(相关的端口中断寄存器中的Sn_IR 超时位(TIMEOUT)置“1”)
	Write_W5500_1Byte(RCR,8);

	//启动中断，参考W5500数据手册确定自己需要的中断类型
	//IMR_CONFLICT是IP地址冲突异常中断,IMR_UNREACH是UDP通信时，地址无法到达的异常中断
	//其它是Socket事件中断，根据需要添加
	Write_W5500_1Byte(IMR, IM_IR7 | IM_IR6);
	Write_W5500_1Byte(SIMR, S0_IMR); //这儿只打开了Socket0的中断
	Write_W5500_SOCK_1Byte(0, Sn_IMR, IMR_SENDOK | IMR_TIMEOUT | IMR_RECV | IMR_DISCON | IMR_CON);
}

/*******************************************************************************
* 函数名  : Detect_Gateway
* 描述    : 检查网关服务器
* 输入    : 无
* 输出    : 无
* 返回值  : 成功返回TRUE(0xFF),失败返回FALSE(0x00)
* 说明    : 无
*******************************************************************************/
uint8_t Detect_Gateway(void){
	uint8_t ip_adde[4];
    uint8_t j=0;
    /* 获得一个与当前ip不同网段的ip，其实就是全部字段+1 */
	ip_adde[0]=IP_Addr[0]+1;
	ip_adde[1]=IP_Addr[1]+1;
	ip_adde[2]=IP_Addr[2]+1;
	ip_adde[3]=IP_Addr[3]+1;

	//检查网关及获取网关的物理地址
	Write_W5500_SOCK_4Byte(0, Sn_DIPR, ip_adde);  //向目的地址寄存器写入与本机IP不同的IP值
	Write_W5500_SOCK_1Byte(0, Sn_MR, MR_TCP);  //设置socket为TCP模式
	Write_W5500_SOCK_1Byte(0, Sn_CR, OPEN);  //打开Socket	
	delay_ms(5);  //延时5ms
	
	if(Read_W5500_SOCK_1Byte(0, Sn_SR) != SOCK_INIT){
        //如果socket打开失败
		Write_W5500_SOCK_1Byte(0, Sn_CR, CLOSE);  //打开不成功,关闭Socket
		return 0;//返回FALSE(0x00)
	}
    //设置Socket为Connect模式
	Write_W5500_SOCK_1Byte(0, Sn_CR, CONNECT);
	do{
		j = Read_W5500_SOCK_1Byte(0, Sn_IR); //读取Socket0中断标志寄存器
		if(j != 0) Write_W5500_SOCK_1Byte(0, Sn_IR, j);
		delay_ms(5);//延时5ms 
		if((j & IR_TIMEOUT) == IR_TIMEOUT){ 
            //连接超时
			return 0;	
		}else if(Read_W5500_SOCK_1Byte(0, Sn_DHAR) != 0xff){
            //获得到了网关的ip地址
			Write_W5500_SOCK_1Byte(0, Sn_CR, CLOSE);//关闭Socket
			return 1;							
		}
	}while(1);
}
/*******************************************************************************
* 函数名  : Socket_Connect
* 描述    : 设置指定Socket(0~7)为客户端与远程服务器连接
* 输入    : s:待设定的端口
* 输出    : 无
* 返回值  : 成功返回TRUE(0xFF),失败返回FALSE(0x00)
* 说明    : 当本机Socket工作在客户端模式时,引用该程序,与远程服务器建立连接
*			如果启动连接后出现超时中断，则与服务器连接失败,需要重新调用该程序连接
*			该程序每调用一次,就与服务器产生一次连接
*******************************************************************************/
uint8_t Socket_Connect(SOCKET s){
	Write_W5500_SOCK_1Byte(s, Sn_MR, MR_TCP);//设置socket为TCP模式
	Write_W5500_SOCK_1Byte(s, Sn_CR, OPEN);//打开Socket
	delay_ms(5);//延时5ms
	if(Read_W5500_SOCK_1Byte(s, Sn_SR)!=SOCK_INIT){
        //如果socket打开失败
		Write_W5500_SOCK_1Byte(s, Sn_CR, CLOSE);//打开不成功,关闭Socket
		return 0;//返回FALSE(0x00)
	}
	Write_W5500_SOCK_1Byte(s, Sn_CR, CONNECT);//设置Socket为Connect模式
	return 1;//返回TRUE,设置成功
}

/*******************************************************************************
* 函数名  : Socket_Listen
* 描述    : 设置指定Socket(0~7)作为服务器等待远程主机的连接
* 输入    : s:待设定的端口
* 输出    : 无
* 返回值  : 成功返回TRUE(0xFF),失败返回FALSE(0x00)
* 说明    : 当本机Socket工作在服务器模式时,引用该程序,等等远程主机的连接
*			该程序只调用一次,就使W5500设置为服务器模式
*******************************************************************************/
uint8_t Socket_Listen(SOCKET s){
	Write_W5500_SOCK_1Byte(s, Sn_MR, MR_TCP);//设置socket为TCP模式 
	Write_W5500_SOCK_1Byte(s, Sn_CR, OPEN);//打开Socket	
	delay_ms(5);//延时5ms
	if(Read_W5500_SOCK_1Byte(s, Sn_SR)!=SOCK_INIT){
        //如果socket打开失败
		Write_W5500_SOCK_1Byte(s, Sn_CR, CLOSE);//打开不成功,关闭Socket
		return 0;//返回FALSE(0x00)
	}	
	Write_W5500_SOCK_1Byte(s, Sn_CR, LISTEN);//设置Socket为侦听模式	
	delay_ms(5);//延时5ms 
	if(Read_W5500_SOCK_1Byte(s, Sn_SR) != SOCK_LISTEN){
        //如果socket设置失败
		Write_W5500_SOCK_1Byte(s, Sn_CR, CLOSE);//设置不成功,关闭Socket
		return 0;//返回FALSE(0x00)
	}

	return 1;

	//至此完成了Socket的打开和设置侦听工作,至于远程客户端是否与它建立连接,则需要等待Socket中断，
	//以判断Socket的连接是否成功。参考W5500数据手册的Socket中断状态
	//在服务器侦听模式不需要设置目的IP和目的端口号
}

/*******************************************************************************
* 函数名  : Socket_UDP
* 描述    : 设置指定Socket(0~7)为UDP模式
* 输入    : s:待设定的端口
* 输出    : 无
* 返回值  : 成功返回TRUE(0xFF),失败返回FALSE(0x00)
* 说明    : 如果Socket工作在UDP模式,引用该程序,在UDP模式下,Socket通信不需要建立连接
*			该程序只调用一次，就使W5500设置为UDP模式
*******************************************************************************/
uint8_t Socket_UDP(SOCKET s){
	Write_W5500_SOCK_1Byte(s, Sn_MR, MR_UDP);//设置Socket为UDP模式*/
	Write_W5500_SOCK_1Byte(s, Sn_CR, OPEN);//打开Socket*/
	delay_ms(5);//延时5ms
	if(Read_W5500_SOCK_1Byte(s, Sn_SR)!=SOCK_UDP){
        //如果Socket打开失败
		Write_W5500_SOCK_1Byte(s, Sn_CR, CLOSE);//打开不成功,关闭Socket
		return 0;//返回FALSE(0x00)
	}
	else
		return 1;

	//至此完成了Socket的打开和UDP模式设置,在这种模式下它不需要与远程主机建立连接
	//因为Socket不需要建立连接,所以在发送数据前都可以设置目的主机IP和目的Socket的端口号
	//如果目的主机IP和目的Socket的端口号是固定的,在运行过程中没有改变,那么也可以在这里设置
}