/* 
 * File: /home/virusv/TDSAST_IOT/User/initialize.h
 * Project: /home/virusv/TDSAST_IOT
 * Created Date: Monday, September 25th 2017, 8:12:40 pm
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

#ifndef USER_INITIALIZE_H__
#define USER_INITIALIZE_H__

void Init_i2c1_oled(void);
//w5500网卡
void Init_spi1_w5500(void);
// 蓝色灯泡
void Init_led_lock(void);
// 时基
void Init_tim2_ms_base(void);
// 独立看门狗
void Init_iwdg_reset(void);
// 实时时钟初始化
void Init_RTC(void);
#endif
