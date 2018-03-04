/* 
 * File: /home/virusv/TDSAST_IOT/User/main.c
 * Project: /home/virusv/TDSAST_IOT
 * Created Date: Saturday, September 23rd 2017, 11:15:08 am
 * Author: Virus.V
 * E-mail: virusv@qq.com
 * -----
 * Modified By: Virus.V
 * Last Modified: Fri Oct 06 2017
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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "stm32f10x.h"
#include "initialize.h"
#include "OLED.h"
#include "socket.h"
#include "DNS/dns.h"
#include "DHCP/dhcp.h"
#include "libemqtt.h"

#define SOCK_MQTT       0
#define SOCK_TCPS       1
#define SOCK_UDPS       2
#define SOCK_DNS		6
#define SOCK_DHCP		7
__IO uint32_t Interval_counter = 0; // MQTT发送心跳包的延时
__IO uint32_t Timer2_Counter = 0; //Timer2定时器计数变量(ms)
#define DATA_BUF_SIZE   2048
#define WAIT_PACKET_TIME 500

#define MQTT_RCVBUFSIZE 1024
/* MQTT接收缓冲区 */
uint8_t packet_buffer[MQTT_RCVBUFSIZE];
/* Socket缓冲区 */
uint8_t gDATABUF[DATA_BUF_SIZE];
/* 默认网络配置参数 */
wiz_NetInfo gWIZNETINFO = { {0x00, 0x08, 0xdc,0x00, 0xab, 0xcd}, //MAC
                            {10, 0, 0, 188}, //IP 
                            {255, 0, 0, 0}, //Subnet Mask
                            {10, 0, 0, 1}, //Gateway
                            {8, 8, 8, 8}, // Google public DNS Server (Primary)
                            NETINFO_DHCP }; //NETINFO_STATIC
/* 备用DNS服务器 */
uint8_t secondary_dns_server[4] = {8, 8, 4, 4}; // Google public DNS Server (Secondary)
//uint8_t ip_from_dns[4]; // 从dns获得的ip地址
/* 获得IP标志位 */
uint8_t ip_configed = 0,lock_on = 0;
/* MQTT代理对象 */
mqtt_broker_handle_t broker;

//W5500的片选信号 回调
static void wizchip_select(void){
    GPIO_ResetBits(GPIOA, GPIO_Pin_4);//置W5500的SCS为低电平
}
//W5500的片选信号 回调
static void wizchip_deselect(void){
    GPIO_SetBits(GPIOA, GPIO_Pin_4);//置W5500的SCS为高电平
}

//向w5500 写一个字节，回调函数
static void wizchip_write(uint8_t wb){
    //printf("W%02x ", wb);
    SPI_I2S_SendData(SPI1, wb);//写1个字节数据
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);//等待数据寄存器空
}

//从w5500读一个字节，回调函数
static uint8_t wizchip_read(void){
    uint8_t i;
    i = SPI_I2S_ReceiveData(SPI1);//读取1个字节数据
    wizchip_write(0x00);//发送一个哑数据 !!!
    i = SPI_I2S_ReceiveData(SPI1);//读取1个字节数据
    //printf("R%02x ", i);
    return i;//读取1个字节数据
}

/* 采用TIM2产生1ms时基延时 */
void delay_ms(uint32_t x){
	Timer2_Counter = 0; 
	while(Timer2_Counter < x){
        /* 喂狗 */
        IWDG_ReloadCounter();
    }
}

static void display_netinfo(void){
    char tmpstr[6],disp[22];
    wiz_NetInfo netinfo;
    ctlnetwork(CN_GET_NETINFO, (void*)&netinfo);
    // 显示网络信息
    ctlwizchip(CW_GET_ID,(void*)tmpstr);
    snprintf(disp, sizeof(disp), "%s:%s", tmpstr, (netinfo.dhcp == NETINFO_DHCP ? "DHCP" : "STATIC"));
    Driver_OLED_ShowString(0, 1, disp, 8, 0);

    snprintf(disp, sizeof(disp), "MAC:%02X:%02X:%02X:%02X:%02X:%02X",netinfo.mac[0],netinfo.mac[1],netinfo.mac[2],
            netinfo.mac[3],netinfo.mac[4],netinfo.mac[5]);
    Driver_OLED_ShowString(0, 2, disp, 8, 0);
    snprintf(disp, sizeof(disp), "SIP:%d.%d.%d.%d", netinfo.ip[0],netinfo.ip[1],netinfo.ip[2],netinfo.ip[3]);
    Driver_OLED_ShowString(0, 3, disp, 8, 0);
    snprintf(disp, sizeof(disp), "GAR:%d.%d.%d.%d", netinfo.gw[0],netinfo.gw[1],netinfo.gw[2],netinfo.gw[3]);
    Driver_OLED_ShowString(0, 4, disp, 8, 0);
    snprintf(disp, sizeof(disp), "SUB:%d.%d.%d.%d", netinfo.sn[0],netinfo.sn[1],netinfo.sn[2],netinfo.sn[3]);
    Driver_OLED_ShowString(0, 5, disp, 8, 0);
    snprintf(disp, sizeof(disp), "DNS:%d.%d.%d.%d", netinfo.dns[0],netinfo.dns[1],netinfo.dns[2],netinfo.dns[3]);
    Driver_OLED_ShowString(0, 6, disp, 8, 0);
}

//DHCP客户端
void DHCPClient(void){
    uint8_t dhcp_ret = 0; //ip是否被配置
    /* DHCP IP allocation and check the DHCP lease time (for IP renewal) */
    if(gWIZNETINFO.dhcp == NETINFO_DHCP) {

        dhcp_ret = DHCP_run();

        if((dhcp_ret == DHCP_SUCCESS) || (dhcp_ret == DHCP_IP_CHANGED)) {
            getIPfromDHCP(gWIZNETINFO.ip);
            getGWfromDHCP(gWIZNETINFO.gw);
            getSNfromDHCP(gWIZNETINFO.sn);
            getDNSfromDHCP(gWIZNETINFO.dns);
            gWIZNETINFO.dhcp = NETINFO_DHCP;
            ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);
            //ip已被成功配置
            ip_configed = 1;
            // 显示ip租约时间
            //printf("DHCP IP Leased Time : %ld Sec\r\n", getDHCPLeasetime());
            display_netinfo();
            /* 喂狗 */
            IWDG_ReloadCounter();
        }
        if(dhcp_ret == DHCP_FAILED)	{
            //printf(">> DHCP Failed\r\n");
            Driver_OLED_ShowString(0, 1, ">> DHCP Failed", 8, 0);
            /* 设置IP模式为静态 */
            gWIZNETINFO.dhcp = NETINFO_STATIC;
            /* 应用网络配置 */
            ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);
            // 显示ip内容
            display_netinfo();
            /* 喂狗 */
            IWDG_ReloadCounter();
            //ip已被成功配置
            ip_configed = 1;
        }
    }
}

//DNS 客户端，参数是域名，返回ip地址，失败返回0
uint8_t DNSClient(uint8_t *hostname, uint8_t ip_from_dns[]){
    uint8_t tmp = 0;
    /* DNS client initialization */
    DNS_init(SOCK_DNS, gDATABUF);

    /* DNS query & response */
    tmp = DNS_run(gWIZNETINFO.dns, secondary_dns_server, hostname, ip_from_dns);
    //tmp = DNS_run(gWIZNETINFO.dns, 0, hostname, ip_from_dns); // if you want to use only primary DNS server, second parameter set to 0
    return tmp;
}

static int send_packet(void* socket_info, const void* buf, unsigned int count) {
    int fd = (int)socket_info;
    return send(fd, (uint8_t *)buf, count);
}

int read_packet(void){
    int total_bytes = 0, bytes_rcvd, packet_length;
    uint8_t rem_len_bytes;  // 1~4
    uint16_t rem_len;   // 数据包中数据长度

	memset(packet_buffer, 0, sizeof(packet_buffer));
	
	if((bytes_rcvd = recv(SOCK_MQTT, packet_buffer, MQTT_RCVBUFSIZE)) <= 0) {
		return -1;
    }
    
	total_bytes += bytes_rcvd;
	if (total_bytes < 2) return -1;
    
    /* 获得帧中保存载荷数据长度的可变数据长度 */
    rem_len_bytes = mqtt_num_rem_len_bytes(packet_buffer);
	/* 获得载荷数据长度 */
    rem_len = mqtt_parse_rem_len(packet_buffer);
    
	// 数据包总长度 = 载荷长度 + 1byte帧头 + 帧中表示载荷长度的可变数据长度
    packet_length = rem_len + rem_len_bytes + 1;
    // 判断数据长度是否大于缓冲区大小
    if(packet_length >= MQTT_RCVBUFSIZE) return -1;
    
	while(total_bytes < packet_length){ //接收剩下的数据
		if((bytes_rcvd = recv(SOCK_MQTT, (packet_buffer+total_bytes), (MQTT_RCVBUFSIZE - total_bytes))) <= 0) return -1;
		total_bytes += bytes_rcvd;
	}

	return packet_length;
}

int main(void){
    uint8_t tmp = 0;
    uint8_t const memsize[2][8] = {{2,2,2,2,2,2,2,2},{2,2,2,2,2,2,2,2}};
    /* MQTT代理服务器地址 */
    uint8_t server_ip[4] = {139,129,19,29};
    /* 配置远程服务器端口 */
    uint16_t server_port = 1883;    // 默认是1883
    uint16_t local_port = 1123; //本地端口,可以随机值

    int packet_length;  // 包长度
	uint16_t msg_id, msg_id_rcv;

    /* LED和锁 */
    Init_led_lock();
    /* 产生时基 */
    Init_tim2_ms_base();
    /* OLED硬件初始化 */
    Init_i2c1_oled();
    /* 初始化W5500 */
    Init_spi1_w5500();
    // 初始化独立看门狗
    Init_iwdg_reset();

    /* 发送初始化指令序列 */
    Driver_OLED_Send_START(OLED_COMMAND);
    Driver_OLED_SendDatas(oled_init, sizeof(oled_init));
    /* 清空屏幕 */
    Driver_OLED_Fill(0x0);
    /* 打印版本信息 */
    Driver_OLED_ShowString(0, 0, "TDSAST-IOT V1.2.4", 8, 0);


    /* 片选回调函数 */
#if _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_VDM_
    reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
#elif _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_FDM_
    reg_wizchip_cs_cbfunc(wizchip_select, wizchip_select);  // CS must be tried with LOW.
#else
    #if (_WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SIP_) != _WIZCHIP_IO_MODE_SIP_
        #error "Unknown _WIZCHIP_IO_MODE_"
    #else
        reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
    #endif
#endif
    /* SPI读写回调函数 */
    reg_wizchip_spi_cbfunc(wizchip_read, wizchip_write);

    /* 喂狗 */
    IWDG_ReloadCounter();

    /* PHY 连接状态检查 */
    do{
        if(ctlwizchip(CW_GET_PHYLINK, (void*)&tmp) == -1)
            Driver_OLED_ShowString(0, 1, "Unknow PHY Link stat.", 8, 0);
    }while(tmp == PHY_LINK_OFF);

    /* 喂狗 */
    IWDG_ReloadCounter();
    
    /* 初始化缓冲区 */
    if(ctlwizchip(CW_INIT_WIZCHIP,(void*)memsize) == -1){
        Driver_OLED_ShowString(0, 1, "NET Initialized fail.", 8, 0);
        while(1);
    }

    /* 喂狗 */
    IWDG_ReloadCounter();
    
    /* 初始化网络 */
    ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);
    
    /* 初始化IP配置 */
    if(gWIZNETINFO.dhcp == NETINFO_DHCP){
        /* >> DHCP 客户端 << */
        DHCP_init(SOCK_DHCP, gDATABUF);
    }else if(gWIZNETINFO.dhcp == NETINFO_STATIC){
        display_netinfo();
    }else{
        Driver_OLED_ShowString(0, 1, "IP Mode Incorrect!", 8, 0);
        while(1);
    }

    /* 喂狗 */
    IWDG_ReloadCounter();

    /* 打印当前网络信息 */
    display_netinfo();
    
    while(1){
        /**
         * 自动获得IP地址,然后检查该ip地址是否到期,
         * 如果到期则再次自动获取,反之直接返回,
         * 保持该函数在大循环里
         */
        DHCPClient();   //自动获得ip地址

        if(!ip_configed) continue;
    
        /* 从DNS获得IP地址 */
        DNSClient("sdc.tdsast.cn", server_ip);
        // do{
        //     char disp[24];
        //     snprintf(disp, sizeof(disp), "IP:%d.%d.%d.%d", server_ip[0],server_ip[1],server_ip[2],server_ip[3]);
        //     Driver_OLED_ShowString(0, 7, disp, 8, 0);
        // }while(0);
        // delay_ms(10000);
        // continue;
        
        /* 喂狗 */
        IWDG_ReloadCounter();

        /* 先关闭该socket */
        close(SOCK_MQTT);
        /* 初始化MQTT客户端 */
        mqtt_init(&broker, "tdsast-iot-ext-door");
        /* 登录MQTT服务器 */
        mqtt_init_auth(&broker, "extdoor", "extdoor12345");
        
        /* 喂狗 */
        IWDG_ReloadCounter();

        /* 打开MQTT的socket */
        socket(SOCK_MQTT, Sn_MR_TCP, local_port, Sn_MR_ND | SOCK_IO_BLOCK);
        /* 连接MQTT服务器 */
        connect(SOCK_MQTT, server_ip, server_port);
        /* MQTT stuffs */
        mqtt_set_alive(&broker, 30);    // 心跳超时30秒,默认是300秒
        broker.socket_info = (void*)SOCK_MQTT; //这个只会传给send对应的函数的第一个参数
        broker.send = send_packet;
        
        /**
         * 连接MQTT服务器
         */
        /* 喂狗 */
        IWDG_ReloadCounter();
        // >>>>> CONNECT
        mqtt_connect(&broker);
        // <<<<< CONNACK
        packet_length = read_packet();

        if(packet_length < 0){
            Driver_OLED_ShowString(0, 7, "Read CONN pack error.", 8, 0); // 显示在最后一行
            delay_ms(1000);
            continue;
        }

        // 检查回包类型
        if(MQTTParseMessageType(packet_buffer) != MQTT_MSG_CONNACK){
            Driver_OLED_ShowString(0, 7, "CONNACK expected!", 8, 0); // 显示在最后一行
            delay_ms(1000);
            continue;
        }

        // 检查是否成功
        if(packet_buffer[3] != 0x00){
            Driver_OLED_ShowString(0, 7, "CONNACK failed!", 8, 0); // 显示在最后一行
            delay_ms(1000);
            continue;
        }

        /**
         * MQTT 订阅Topic
         */
        /* 喂狗 */
        IWDG_ReloadCounter();
        // >>>>> SUBSCRIBE
        mqtt_subscribe(&broker, "td_cloud/tdsast/ext_door", &msg_id);
        // <<<<< SUBACK
        packet_length = read_packet();
        if(packet_length < 0){
            Driver_OLED_ShowString(0, 7, "Read SUBS pack error.", 8, 0); // 显示在最后一行
            delay_ms(1000);
            continue;
        }
        // 检查接收数据包类型
        if(MQTTParseMessageType(packet_buffer) != MQTT_MSG_SUBACK){
            Driver_OLED_ShowString(0, 7, "SUBACK expected!", 8, 0); // 显示在最后一行
            delay_ms(1000);
            continue;
        }
        // 检查接收数据包id
        msg_id_rcv = mqtt_parse_msg_id(packet_buffer);
        if(msg_id != msg_id_rcv){
            // 发送包和接收包的msg id不匹配
            delay_ms(1000);
            continue;
        }

        // 设置socket为非阻塞模式
        tmp = SOCK_IO_NONBLOCK;
        if(ctlsocket(SOCK_MQTT, CS_SET_IOMODE, (void *)&tmp) != SOCK_OK){
            delay_ms(1000);
            continue;
        }
        //__BKPT(1);
        GPIO_ResetBits(GPIOB, GPIO_Pin_9);  // 点亮灯
        GPIO_SetBits(GPIOB, GPIO_Pin_8);  //上锁
        
        while(getSn_SR(SOCK_MQTT) == SOCK_ESTABLISHED){
            /* 喂狗 */
            IWDG_ReloadCounter();
            
            // 发送心跳包
            if(Interval_counter > 25000){
                Interval_counter = 0;
                mqtt_ping(&broker);
            }

            if(lock_on && Timer2_Counter > 3000u){ // 开锁3秒
                lock_on = 0;
                // 上锁
                GPIO_SetBits(GPIOB, GPIO_Pin_8);
            }
            // <<<<<
            packet_length = read_packet();
            if(packet_length <= 0){
                continue;
            }
            
            if(MQTTParseMessageType(packet_buffer) == MQTT_MSG_PUBLISH) {
                uint8_t msg[128];
                //uint8_t topic[32];
                uint16_t len;
                //len = (len = mqtt_parse_pub_topic(packet_buffer, topic)) >= sizeof(topic) ? sizeof(topic) - 1 : len;
                //topic[len] = '\0';
                len = (len = mqtt_parse_publish_msg(packet_buffer, msg)) >= sizeof(msg) ? sizeof(msg) - 1 : len;
                msg[len] = '\0';
                //printf("%s %s\n", topic, msg);
                //Driver_OLED_ShowString(0, 7, msg, 8, 0); // 显示在最后一行
                if(!lock_on && strncmp("Lock ON", msg, 7) == 0){  //判断是不是开锁指令
                    // 开锁
                    lock_on = 1;
                    Timer2_Counter = 0;
                    GPIO_ResetBits(GPIOB, GPIO_Pin_8);  //开锁
                }
            }
            
        }
        GPIO_SetBits(GPIOB, GPIO_Pin_9); // 熄灭灯
        GPIO_ResetBits(GPIOB, GPIO_Pin_8);  //开锁
    }
    return 0;
}
