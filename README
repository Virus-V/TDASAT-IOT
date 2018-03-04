##介绍：
这是使用STM32F103C8T6+W5500设计的物联网控制电磁锁的系统。  
STM32通过SPI接口与W5500通讯，然后移植socket层，在编程过程中程序与外部socket通讯
就像在linux下面调用socket API几乎一样。  

系统里集成了libeMQTT库，是一个轻量的MQTT协议实现，这个库只实现了MQTT协议的publish QoS 0消息和
subscribe QoS 0的消息，没有实现其他级别的QoS消息订阅和推送，这是它不足的地方。  

##使用硬件：  

* [1、0.96寸OLED显示屏Arduino 12864液晶屏模块IIC接口 提供原理图](https://detail.tmall.com/item.htm?spm=a230r.1.14.13.5913092etxEcFs&id=531082889483&cm_id=140105335569ed55e27b&abbucket=7)  
* [2、大功率MOS管场效应管 触发开关驱动板 PWM调节电子开关控制板模块](https://detail.tmall.com/item.htm?id=535354446438&_u=t2dmg8j26111)
* [3、LM2596多路开关电源 3.3V/5V/12V/ADJ可调电压输出 电源模块](https://item.taobao.com/item.htm?id=525275474884&_u=t2dmg8j26111)
* [4、EU-STM32F103C8开发板STM32小系统核心板STM32单片机学习评估板](https://item.taobao.com/item.htm?spm=a1z10.5-c.w4002-4286033451.20.21490a4fih9UQ9&id=39907173191)
* [5、ZUCON磁力锁反馈电磁锁280kg公斤明装磁力锁楼宇电锁带信号门锁](https://item.taobao.com/item.htm?id=19933084848&_u=t2dmg8j26111)
* [6、艾克姆 W5500以太网模块 SPI接口 TCP/IP STC15/STM32单片机](https://item.taobao.com/item.htm?id=535409082707&_u=t2dmg8j26111)

服务器：CentOS 7.0 + mosquitto（MQTT broker）  
嵌入式模块：libemqtt  
微信：VPF，phpWechat  

##TODO：  
1、移植SSL/TLS层，完善物联网设备安全层

###v1.2.1  
1、修复断网无法自动重新连接服务器

###v1.2.0   
1、增加了断网开锁功能  
2、移植DNS功能，可以根据域名找到MQTT broker服务器，sdc.tdsast.cn

###v1.0.0  
1、移植socket层  
2、移植DHCP服务，使其在局域网内自动获得ip地址  
3、加入libemqtt，嵌入式MQTT Client库  


### 联系我
如果在使用过程中遇到任何问题，请联系我Email: virusv@qq.com
