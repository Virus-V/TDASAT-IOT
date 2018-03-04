#ifndef	_W5500_H_
#define	_W5500_H_
#include <stdint.h>
/***************** Common Register *****************/
#define MR		0x0000
	#define RST		0x80 //复位
	#define WOL		0x20 //Weak on Lan
	#define PB		0x10 //Ping Block Mode
	#define PPP		0x08 //PPPoE Mode
	#define FARP	0x02 //Force ARP

#define GAR		0x0001 //Gateway Address Register
#define SUBR	0x0005 //Subnet Mask Address Register
#define SHAR	0x0009 //Source Hardware Address
#define SIPR	0x000f //Source IP Address

#define INTLEVEL	0x0013 //Interrupt Low Level Timer 两次中断请求的间隔时间
#define IR		0x0015 //Interrupt Register 
	#define CONFLICT	0x80 //IP地址冲突
	#define UNREACH		0x40 //目标不可达
	#define PPPOE		0x20 //PPPoE Connection Close
	#define MP			0x10 //Magic Packet
                             //When WOL mode is enabled and receives the magic packet over UDP,
                             //this bit is set.

#define IMR		0x0016 //Interrupt Mask
	#define IM_IR7		0x80  //对应前四个
	#define IM_IR6		0x40
	#define IM_IR5		0x20
	#define IM_IR4		0x10

#define SIR		0x0017 //Socket Interrupt
	#define S7_INT		0x80
	#define S6_INT		0x40
	#define S5_INT		0x20
	#define S4_INT		0x10
	#define S3_INT		0x08
	#define S2_INT		0x04
	#define S1_INT		0x02
	#define S0_INT		0x01

#define SIMR	0x0018 //Socket Interrupt Mask
	#define S7_IMR		0x80
	#define S6_IMR		0x40
	#define S5_IMR		0x20
	#define S4_IMR		0x10
	#define S3_IMR		0x08
	#define S2_IMR		0x04
	#define S1_IMR		0x02
	#define S0_IMR		0x01

#define RTR		0x0019
#define RCR		0x001b

#define PTIMER	0x001c
#define PMAGIC	0x001d
#define PHA		0x001e
#define PSID	0x0024
#define PMRU	0x0026

#define UIPR	0x0028
#define UPORT	0x002c

#define PHYCFGR	0x002e
	#define RST_PHY		0x80
	#define OPMODE		0x40
	#define DPX			0x04
	#define SPD			0x02
	#define LINK		0x01

#define VERR	0x0039

/********************* Socket Register *******************/
#define Sn_MR		0x0000
	#define MULTI_MFEN		0x80
	#define BCASTB			0x40
	#define	ND_MC_MMB		0x20
	#define UCASTB_MIP6B	0x10
	#define MR_CLOSE		0x00
	#define MR_TCP		    0x01
	#define MR_UDP	    	0x02
	#define MR_MACRAW		0x04

#define Sn_CR		0x0001 
	#define OPEN		0x01
	#define LISTEN		0x02
	#define CONNECT		0x04
	#define DISCON		0x08
	#define CLOSE		0x10
	#define SEND		0x20
	#define SEND_MAC	0x21
	#define SEND_KEEP	0x22
	#define RECV		0x40

#define Sn_IR		0x0002
	#define IR_SEND_OK		0x10
	#define IR_TIMEOUT		0x08
	#define IR_RECV			0x04
	#define IR_DISCON		0x02
	#define IR_CON			0x01

#define Sn_IMR		0x002c
	#define IMR_SENDOK	    0x10
	#define IMR_TIMEOUT 	0x08
	#define IMR_RECV	    0x04
	#define IMR_DISCON  	0x02
	#define IMR_CON		    0x01

#define Sn_SR		0x0003
	#define SOCK_CLOSED		0x00
	#define SOCK_INIT		0x13
	#define SOCK_LISTEN		0x14
	#define SOCK_ESTABLISHED	0x17
	#define SOCK_CLOSE_WAIT		0x1c
	#define SOCK_UDP		0x22
	#define SOCK_MACRAW		0x02
    /* 临时状态 */
	#define SOCK_SYNSEND	0x15
	#define SOCK_SYNRECV	0x16
	#define SOCK_FIN_WAI	0x18
	#define SOCK_CLOSING	0x1a
	#define SOCK_TIME_WAIT	0x1b
	#define SOCK_LAST_ACK	0x1d

#define Sn_PORT		0x0004
#define Sn_DHAR	   	0x0006
#define Sn_DIPR		0x000c
#define Sn_DPORTR	0x0010

#define Sn_MSSR		0x0012
#define Sn_TOS		0x0015
#define Sn_TTL		0x0016

#define Sn_RXBUF_SIZE	0x001e
#define Sn_TXBUF_SIZE	0x001f
#define Sn_TX_FSR	    0x0020
#define Sn_TX_RD	    0x0022
#define Sn_TX_WR	    0x0024
#define Sn_RX_RSR   	0x0026
#define Sn_RX_RD	    0x0028
#define Sn_RX_WR	    0x002a

#define Sn_FRAG	    	0x002d
#define Sn_KPALVTR  	0x002f

/*******************************************************************/
/************************ SPI Control Byte *************************/
/*******************************************************************/
/* Operation mode bits */
#define VDM		0x00  //可变长的数据
#define FDM1	0x01  //固定数据长度模式，1字节
#define	FDM2	0x02  //固定数据长度模式，2字节
#define FDM4	0x03  //固定数据长度模式，4字节

/* Read_Write control bit */
#define RWB_READ	0x00
#define RWB_WRITE	0x04

/* Block select bits */
#define COMMON_R	0x00

/* Socket 0 */
#define S0_REG		0x08
#define S0_TX_BUF	0x10
#define S0_RX_BUF	0x18

/* Socket 1 */
#define S1_REG		0x28
#define S1_TX_BUF	0x30
#define S1_RX_BUF	0x38

/* Socket 2 */
#define S2_REG		0x48
#define S2_TX_BUF	0x50
#define S2_RX_BUF	0x58

/* Socket 3 */
#define S3_REG		0x68
#define S3_TX_BUF	0x70
#define S3_RX_BUF	0x78

/* Socket 4 */
#define S4_REG		0x88
#define S4_TX_BUF	0x90
#define S4_RX_BUF	0x98

/* Socket 5 */
#define S5_REG		0xa8
#define S5_TX_BUF	0xb0
#define S5_RX_BUF	0xb8

/* Socket 6 */
#define S6_REG		0xc8
#define S6_TX_BUF	0xd0
#define S6_RX_BUF	0xd8

/* Socket 7 */
#define S7_REG		0xe8
#define S7_TX_BUF	0xf0
#define S7_RX_BUF	0xf8

#define S_RX_SIZE	2048	/*定义Socket接收缓冲区的大小，可以根据W5500_RMSR的设置修改 */
#define S_TX_SIZE	2048  	/*定义Socket发送缓冲区的大小，可以根据W5500_TMSR的设置修改 */


/***************----- W5500 GPIO定义 -----***************/
#define W5500_SCS		GPIO_Pin_4	//定义W5500的CS引脚	 
#define W5500_SCS_PORT	GPIOA
	
#define W5500_RST		GPIO_Pin_3	//定义W5500的RST引脚
#define W5500_RST_PORT	GPIOA

#define W5500_INT		GPIO_Pin_2	//定义W5500的INT引脚
#define W5500_INT_PORT	GPIOA

/***************----- 网络参数变量定义 -----***************/
extern uint8_t Gateway_IP[4];	//网关IP地址 
extern uint8_t Sub_Mask[4];	//子网掩码 
extern uint8_t Phy_Addr[6];	//物理地址(MAC) 
extern uint8_t IP_Addr[4];	//本机IP地址 

extern uint8_t UDP_DIPR[4];	//UDP(广播)模式,目的主机IP地址
extern uint8_t UDP_DPORT[2];	//UDP(广播)模式,目的主机端口号

/***************----- 端口数据缓冲区 -----***************/
extern uint8_t Rx_Buffer[2048];	//端口接收数据缓冲区 
extern uint8_t Tx_Buffer[2048];	//端口发送数据缓冲区 

extern __IO uint8_t W5500_Interrupt;	//W5500中断标志(0:无中断,1:有中断)
typedef uint8_t SOCKET;			//自定义端口号数据类型

void Write_W5500_1Byte(uint16_t reg, uint8_t dat);
void Write_W5500_2Byte(uint16_t reg, uint16_t dat);

void Write_W5500_nByte(uint16_t reg, uint8_t *dat_ptr, uint16_t size);

uint8_t Read_W5500_1Byte(uint16_t reg);
uint8_t Read_W5500_SOCK_1Byte(SOCKET s, uint16_t reg);
uint16_t Read_W5500_SOCK_2Byte(SOCKET s, uint16_t reg);
uint16_t Read_SOCK_Data_Buffer(SOCKET s, uint8_t *dat_ptr);
void Write_W5500_SOCK_1Byte(SOCKET s, uint16_t reg, uint8_t dat);
void Write_W5500_SOCK_2Byte(SOCKET s, uint16_t reg, uint16_t dat);
void Write_W5500_SOCK_4Byte(SOCKET s, uint16_t reg, uint8_t *dat_ptr);

void Write_SOCK_Data_Buffer(SOCKET s, uint8_t *dat_ptr, uint16_t size);
void W5500_Hardware_Reset(void);
void W5500_Init(void);
uint8_t Detect_Gateway(void);
uint8_t Socket_Connect(SOCKET s);
uint8_t Socket_Listen(SOCKET s);
uint8_t Socket_UDP(SOCKET s);
#endif
