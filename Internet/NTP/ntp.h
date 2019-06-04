/*
 * ntp.h
 *
 *  Created on: 2019-6-3
 *      Author: virusv
 */

#ifndef INTERNET_NTP_NTP_H_
#define INTERNET_NTP_NTP_H_

int GetNTPTime(int fd, uint8_t *ip, uint16_t port, uint64_t *time);

#endif /* INTERNET_NTP_NTP_H_ */
