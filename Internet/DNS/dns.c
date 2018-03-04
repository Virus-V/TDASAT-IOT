#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dns.h"
#include "socket.h"
#include "dns_parse.h"


/*
********************************************************************************
Define Part
********************************************************************************
*/
//#define _DNS_DEBUG_

/*
********************************************************************************
Local Variable Declaration Section
********************************************************************************
*/
uint8_t * dns_buf;
uint8_t DNS_SOCKET;
uint16_t MSG_ID;

uint32_t dns_time;

/*
********************************************************************************
Function Implementation Part
********************************************************************************
*/
uint8_t * put16(uint8_t * s, uint16_t i);
int16_t dns_makequery(uint16_t op, char * name, uint8_t * buf, uint16_t len);
uint8_t DNS_query(uint8_t * dns_server, uint8_t * name, uint8_t * ip_from_dns);

int8_t check_DNS_timeout(void);


/*
********************************************************************************
*              PUT NETWORK BYTE ORDERED INT.
*
* Description : This function copies uint16_t to the network buffer with network byte order.
* Arguments   : s - is a pointer to the network buffer.
*               i - is a unsigned integer.
* Returns     : a pointer to the buffer.
* Note        : Internal Function
********************************************************************************
*/
uint8_t * put16(uint8_t * s, uint16_t i)
{
	*s++ = i >> 8;
	*s++ = i;

	return s;
}


/*
********************************************************************************
*              MAKE DNS QUERY MESSAGE
*
* Description : This function makes DNS query message.
* Arguments   : op   - Recursion desired
*               name - is a pointer to the domain name.
*               buf  - is a pointer to the buffer for DNS message.
*               len  - is the MAX. size of buffer.
* Returns     : the pointer to the DNS message.
* Note        :
********************************************************************************
*/
int16_t dns_makequery(uint16_t op, char * name, uint8_t * buf, uint16_t len)
{
	uint8_t *cp;
	char *cp1;
	char sname[MAX_DNS_BUF_SIZE];
	char *dname;
	uint16_t p;
	uint16_t dlen;

	cp = buf;

	MSG_ID++;
	cp = put16(cp, MSG_ID);
	p = (op << 11) | 0x0100;			/* Recursion desired */
	cp = put16(cp, p);
	cp = put16(cp, 1);
	cp = put16(cp, 0);
	cp = put16(cp, 0);
	cp = put16(cp, 0);

	strcpy(sname, name);
	dname = sname;
	dlen = strlen(dname);
	for (;;)
	{
		/* Look for next dot */
		cp1 = strchr(dname, '.');

		if (cp1 != NULL) len = cp1 - dname;	/* More to come */
		else len = dlen;			/* Last component */

		*cp++ = len;				/* Write length of component */
		if (len == 0) break;

		/* Copy component up to (but not including) dot */
		strncpy((char *)cp, dname, len);
		cp += len;
		if (cp1 == NULL)
		{
			*cp++ = 0;			/* Last one; write null and finish */
			break;
		}
		dname += len+1;
		dlen -= len+1;
	}

	cp = put16(cp, 0x0001);				/* type */
	cp = put16(cp, 0x0001);				/* class */

	return ((int16_t)((uint32_t)(cp) - (uint32_t)(buf)));
}

/*
********************************************************************************
*              MAKE DNS QUERY AND PARSE THE REPLY
*
* Description : This function makes DNS query message and parses the reply from DNS server.
* Arguments   : name - is a pointer to the domain name.
* Returns     : if succeeds : 1, fails : 0
* Note        :
********************************************************************************
*/
uint8_t DNS_query(uint8_t * dns_server, uint8_t * name, uint8_t * ip_from_dns)
{
	struct dhdr dhp;
	uint8_t ip[4];
	uint16_t len, port;
	int8_t ret_check_timeout;
	uint16_t dns_port;

	len = dns_makequery(0, (char *)name, dns_buf, MAX_DNS_BUF_SIZE);
	sendto(DNS_SOCKET, dns_buf, len, dns_server, IPPORT_DOMAIN);


	while (1)
	{
		if ((len = getSn_RX_RSR(DNS_SOCKET)) > 0)
		{
			if (len > MAX_DNS_BUF_SIZE) len = MAX_DNS_BUF_SIZE;
			len = recvfrom(DNS_SOCKET, dns_buf, len, ip, &port);
			break;
		}

		// Check Timeout
		ret_check_timeout = check_DNS_timeout();

		if (ret_check_timeout < 0) {

#ifdef _DNS_DEBUG_
			printf("> DNS Server is not responding : %d.%d.%d.%d\r\n", dns_server[0], dns_server[1], dns_server[2], dns_server[3]);
#endif
			return 0; // timeout occurred
		}
		else if (ret_check_timeout == 0) {

#ifdef _DNS_DEBUG_
			printf("> Timeout\r\n");
#endif
			// Generate random port number
			//srand(seed); users can use the seed value for make random variable.
			dns_port = rand() % 2000 + 63535; // 63535 ~ 65534

			// Socket open
			socket(DNS_SOCKET, Sn_MR_UDP, dns_port, 0);

			sendto(DNS_SOCKET, dns_buf, len, dns_server, IPPORT_DOMAIN);
		}
	}

	return(parseMSG(&dhp, dns_buf, ip_from_dns));	/* Convert to local format */
}

/*
********************************************************************************
*              DNS CLIENT INIT
*
* Description : This function initialize DNS client
* Arguments   : s - is a DNS socket number, buf - is a pointer of user's buffer
* Returns     : None.
* Note        :
********************************************************************************
*/

void DNS_init(uint8_t s, uint8_t * buf)
{
	uint16_t dns_port = 3000;

	DNS_SOCKET = s; // SOCK_DNS
	dns_buf = buf; // User's shared buffer
	MSG_ID = 0x1122;

	// Generate random port number
	//srand(seed); users can use the seed value for make random variable.
	dns_port = rand() % 2000 + 63535; // 63535 ~ 65534
	// Socket open
	socket(DNS_SOCKET, Sn_MR_UDP, dns_port, 0);
}

/*
********************************************************************************
*              DNS CLIENT RUN
*
* Description : This function get the IP address from DNS
* Arguments   : dns_server_1st - IP address of primary DNS server
* 				dns_server_2nd - IP address of secondary DNS server
* 				name - is a pointer to the domain name.
* 				ip_from_dns - IP address of domain from DNS server
* Returns     : 0 - both failed / 1 - primary success / 2 - secondary success (primary failed)
* Note        :
********************************************************************************
*/

uint8_t DNS_run(uint8_t * dns_server_1st, uint8_t * dns_server_2nd, uint8_t * name, uint8_t * ip_from_dns)
{
	uint8_t ret;

#ifdef _DNS_DEBUG_
	printf("> DNS Query to Primary DNS Server : %d.%d.%d.%d\r\n", dns_server_1st[0], dns_server_1st[1], dns_server_1st[2], dns_server_1st[3]);
#endif

	ret = DNS_query(dns_server_1st, name, ip_from_dns);

	if(!ret) {
		if(dns_server_2nd != 0) {

#ifdef _DNS_DEBUG_
			printf("> DNS Query to Secondary DNS Server : %d.%d.%d.%d\r\n", dns_server_2nd[0], dns_server_2nd[1], dns_server_2nd[2], dns_server_2nd[3]);
#endif
			ret = DNS_query(dns_server_2nd, name, ip_from_dns);
			if(ret == 1) ret++;
		}
	}

	close(DNS_SOCKET);
	// Return value
	// 0 - both failed / 1 - primary success / 2 - secondary success (primary failed)
	return ret;
}

/*
********************************************************************************
*              CHECK DNS TIMEOUT
*
* Description : This function check the DNS timeout
* Arguments   : None.
* Returns     : -1 - timeout occurred, 0 - timer over, but no timeout, 1 - no timer over, no timeout occur
* Note        : timeout : retry count and timer both over.
********************************************************************************
*/

int8_t check_DNS_timeout(void)
{
	static uint8_t retry_count;

	if(dns_time >= DNS_WAIT_TIME)
	{
		dns_time = 0;
		if(retry_count >= MAX_DNS_RETRY) {
			retry_count = 0;
			return -1; // timeout occurred
		}
		retry_count++;
		return 0; // timer over, but no timeout
	}

	return 1; // no timer over, no timeout occur
}

/*
*********************************************************************************************************
*				DNS TIMER HANDLER
*
* Description: This function must be called once per second in the timer interrupt handler.
* Arguments  : None.
* Returns    : None.
* Note       :
*********************************************************************************************************
*/
void DNS_timerHandler(void)
{
	dns_time++;
}



