/*
********************************************************************************
File Include Section
********************************************************************************
*/
#include <stdio.h>
#include <string.h>
#include "socket.h"
#include "dhcp.h"

/*
********************************************************************************
Define Part
********************************************************************************
*/
//#define _DHCP_DEBUG_

/*
********************************************************************************
Function declaration Part
********************************************************************************
*/
void send_DHCP_DISCOVER(void);
void send_DHCP_REQUEST(uint8_t *Cip, uint8_t *d_addr);
void send_DHCP_REREQUEST(void);
void send_DHCP_RELEASE(void);
void send_DHCP_DECLINE(void);

uint8_t check_DHCP_state(void);
int8_t check_DHCP_leasedIP(void);

uint8_t check_DHCP_timeout(void);
void reset_DHCP_timeout(void);
void DHCP_timerHandler(void);

int8_t parseDHCPMSG(uint16_t length);

/*
********************************************************************************
Local Variable Declaration Section
********************************************************************************
*/

uint8_t DHCP_SIP[4];
uint8_t DHCP_SHA[6];
uint8_t OLD_SIP[4];

// Network information from DHCP Server
uint8_t DHCP_allocated_ip[4] = {0, };
uint8_t DHCP_allocated_gw[4] = {0, };
uint8_t DHCP_allocated_sn[4] = {0, };
uint8_t DHCP_allocated_dns[4] = {0, };

uint8_t DHCP_SOCKET;

// Get wizchip MAC address
uint8_t wizchip_mac[6] = {0, };

int8_t dhcp_state;
int8_t retry_count;

un_l2cval lease_time;
uint32_t my_time, next_time;

uint32_t DHCP_XID;

RIP_MSG*  MSG;

uint8_t HOST_NAME[] = "TDSAST";
uint8_t Cip[4] = {0,0,0,0};

/*
********************************************************************************
User's Function Implementation Part
********************************************************************************
*/

static void ip_assign(void) { }

static void ip_update(void)
{
	/* WIZchip Software Reset */
	setMR(MR_RST);
	getMR(); // for delay
	setSHAR(wizchip_mac);
}

static void ip_conflict(void)
{
	// WIZchip Software Reset
	setMR(MR_RST);
	getMR(); // for delay
	setSHAR(wizchip_mac);

	socket(DHCP_SOCKET, Sn_MR_UDP, DHCP_CLIENT_PORT, 0x00);
	send_DHCP_DISCOVER();
	dhcp_state = STATE_DHCP_DISCOVER;
}

void (*dhcp_ip_assign)(void) = ip_assign;		/**< handler to be called when the IP address from DHCP server is first assigned */
void (*dhcp_ip_update)(void) = ip_update;		/**< handler to be called when the IP address from DHCP server is updated */
void (*dhcp_ip_conflict)(void) = ip_conflict;		/**< handler to be called when the IP address from DHCP server is conflict */


/*
********************************************************************************
Function Implementation Part
********************************************************************************
*/

/*
*********************************************************************************************************
*              SEND DHCP DISCOVER
*
* Description: This function sends DHCP DISCOVER message to DHCP server.
* Arguments  : None.
* Returns    : None.
* Note       : 
*********************************************************************************************************
*/
void send_DHCP_DISCOVER(void)
{
	uint16_t i;
	uint8_t ip[4];
	uint16_t k = 0;

	MSG->op = DHCP_BOOTREQUEST;
	MSG->htype = DHCP_HTYPE10MB;
	MSG->hlen = DHCP_HLENETHERNET;
	MSG->hops = DHCP_HOPS;
	MSG->xid = DHCP_XID;
	MSG->secs = DHCP_SECS;
	MSG->flags = DHCP_FLAGSBROADCAST;

	MSG->ciaddr[0] = 0;
	MSG->ciaddr[1] = 0;
	MSG->ciaddr[2] = 0;
	MSG->ciaddr[3] = 0;

	MSG->yiaddr[0] = 0;
	MSG->yiaddr[1] = 0;
	MSG->yiaddr[2] = 0;
	MSG->yiaddr[3] = 0;

	MSG->siaddr[0] = 0;
	MSG->siaddr[1] = 0;
	MSG->siaddr[2] = 0;
	MSG->siaddr[3] = 0;

	MSG->giaddr[0] = 0;
	MSG->giaddr[1] = 0;
	MSG->giaddr[2] = 0;
	MSG->giaddr[3] = 0;

	MSG->chaddr[0] = wizchip_mac[0];
	MSG->chaddr[1] = wizchip_mac[1];
	MSG->chaddr[2] = wizchip_mac[2];
	MSG->chaddr[3] = wizchip_mac[3];
	MSG->chaddr[4] = wizchip_mac[4];
	MSG->chaddr[5] = wizchip_mac[5];

	for (i = 6; i < 16; i++) MSG->chaddr[i] = 0;
	for (i = 0; i < 64; i++) MSG->sname[i] = 0;
	for (i = 0; i < 128; i++) MSG->file[i] = 0;

	// MAGIC_COOKIE
	MSG->OPT[k++] = 0x63;
	MSG->OPT[k++] = 0x82;
	MSG->OPT[k++] = 0x53;
	MSG->OPT[k++] = 0x63;

	// Option Request Param
	MSG->OPT[k++] = dhcpMessageType;
	MSG->OPT[k++] = 0x01;
	MSG->OPT[k++] = DHCP_DISCOVER;
	
	// Client identifier
	MSG->OPT[k++] = dhcpClientIdentifier;
	MSG->OPT[k++] = 0x07;
	MSG->OPT[k++] = 0x01;
	MSG->OPT[k++] = wizchip_mac[0];
	MSG->OPT[k++] = wizchip_mac[1];
	MSG->OPT[k++] = wizchip_mac[2];
	MSG->OPT[k++] = wizchip_mac[3];
	MSG->OPT[k++] = wizchip_mac[4];
	MSG->OPT[k++] = wizchip_mac[5];
	
	// host name
	MSG->OPT[k++] = hostName;
	MSG->OPT[k++] = 9; // length of hostname
	MSG->OPT[k++] = HOST_NAME[0];
	MSG->OPT[k++] = HOST_NAME[1];
	MSG->OPT[k++] = HOST_NAME[2];
	MSG->OPT[k++] = HOST_NAME[3];
	MSG->OPT[k++] = HOST_NAME[4];
	MSG->OPT[k++] = HOST_NAME[5];
	MSG->OPT[k++] = wizchip_mac[3];
	MSG->OPT[k++] = wizchip_mac[4];
	MSG->OPT[k++] = wizchip_mac[5];


	MSG->OPT[k++] = dhcpParamRequest;
	MSG->OPT[k++] = 0x06;	// length of request
	MSG->OPT[k++] = subnetMask;
	MSG->OPT[k++] = routersOnSubnet;
	MSG->OPT[k++] = dns;
	MSG->OPT[k++] = domainName;
	MSG->OPT[k++] = dhcpT1value;
	MSG->OPT[k++] = dhcpT2value;
	MSG->OPT[k++] = endOption;

	for (i = k; i < OPT_SIZE; i++) MSG->OPT[i] = 0;

	// send broadcasting packet
	for (i = 0; i < 4; i++) ip[i] = 255;

#ifdef _DHCP_DEBUG_
	printf("> Send DHCP_DISCOVER\r\n");
#endif

	sendto(DHCP_SOCKET, (uint8_t *)MSG, RIP_MSG_SIZE, ip, DHCP_SERVER_PORT);
}

/*
*********************************************************************************************************
*              SEND DHCP REQUEST
*
* Description: This function sends DHCP REQUEST message to DHCP server.
* Arguments  : Cip - Initial IP address for DHCP (0.0.0.0) or allocated IP address from DHCP (for IP renewal)
* 			   d_addr - Destination IP address
* Returns    : None.
* Note       : 
*********************************************************************************************************
*/
void send_DHCP_REQUEST(uint8_t *Cip, uint8_t *d_addr)
{
	int i;
	uint8_t ip[4];
	uint16_t k = 0;

	MSG->op = DHCP_BOOTREQUEST;
	MSG->htype = DHCP_HTYPE10MB;
	MSG->hlen = DHCP_HLENETHERNET;
	MSG->hops = DHCP_HOPS;
	MSG->xid = DHCP_XID;
	MSG->secs = DHCP_SECS;
	//MSG->flags = DHCP_FLAGSBROADCAST;
	if (d_addr[0] == 0xff) 	MSG->flags = DHCP_FLAGSBROADCAST;
	else MSG->flags = 0;
	
	MSG->ciaddr[0] = Cip[0];
	MSG->ciaddr[1] = Cip[1];
	MSG->ciaddr[2] = Cip[2];
	MSG->ciaddr[3] = Cip[3];

	MSG->yiaddr[0] = 0;
	MSG->yiaddr[1] = 0;
	MSG->yiaddr[2] = 0;
	MSG->yiaddr[3] = 0;

	MSG->siaddr[0] = 0;
	MSG->siaddr[1] = 0;
	MSG->siaddr[2] = 0;
	MSG->siaddr[3] = 0;

	MSG->giaddr[0] = 0;
	MSG->giaddr[1] = 0;
	MSG->giaddr[2] = 0;
	MSG->giaddr[3] = 0;

	MSG->chaddr[0] = wizchip_mac[0];
	MSG->chaddr[1] = wizchip_mac[1];
	MSG->chaddr[2] = wizchip_mac[2];
	MSG->chaddr[3] = wizchip_mac[3];
	MSG->chaddr[4] = wizchip_mac[4];
	MSG->chaddr[5] = wizchip_mac[5];

	for (i = 6; i < 16; i++) MSG->chaddr[i] = 0;
	for (i = 0; i < 64; i++) MSG->sname[i] = 0;
	for (i = 0; i < 128; i++) MSG->file[i] = 0;

	// MAGIC_COOKIE 
	MSG->OPT[k++] = 0x63;
	MSG->OPT[k++] = 0x82;
	MSG->OPT[k++] = 0x53;
	MSG->OPT[k++] = 0x63;

	// Option Request Param.
	MSG->OPT[k++] = dhcpMessageType;
	MSG->OPT[k++] = 0x01;
	MSG->OPT[k++] = DHCP_REQUEST;

	MSG->OPT[k++] = dhcpClientIdentifier;
	MSG->OPT[k++] = 0x07;
	MSG->OPT[k++] = 0x01;
	MSG->OPT[k++] = wizchip_mac[0];
	MSG->OPT[k++] = wizchip_mac[1];
	MSG->OPT[k++] = wizchip_mac[2];
	MSG->OPT[k++] = wizchip_mac[3];
	MSG->OPT[k++] = wizchip_mac[4];
	MSG->OPT[k++] = wizchip_mac[5];

	if (d_addr[0] == 0xff) {
		MSG->OPT[k++] = dhcpRequestedIPaddr;
		MSG->OPT[k++] = 0x04;
		MSG->OPT[k++] = DHCP_allocated_ip[0];
		MSG->OPT[k++] = DHCP_allocated_ip[1];
		MSG->OPT[k++] = DHCP_allocated_ip[2];
		MSG->OPT[k++] = DHCP_allocated_ip[3];
	
		MSG->OPT[k++] = dhcpServerIdentifier;
		MSG->OPT[k++] = 0x04;
		MSG->OPT[k++] = DHCP_SIP[0];
		MSG->OPT[k++] = DHCP_SIP[1];
		MSG->OPT[k++] = DHCP_SIP[2];
		MSG->OPT[k++] = DHCP_SIP[3];
	}

	// host name
	MSG->OPT[k++] = hostName;
	MSG->OPT[k++] = 9; // length of hostname
	MSG->OPT[k++] = HOST_NAME[0];
	MSG->OPT[k++] = HOST_NAME[1];
	MSG->OPT[k++] = HOST_NAME[2];
	MSG->OPT[k++] = HOST_NAME[3];
	MSG->OPT[k++] = HOST_NAME[4];
	MSG->OPT[k++] = HOST_NAME[5];
	MSG->OPT[k++] = wizchip_mac[3];
	MSG->OPT[k++] = wizchip_mac[4];
	MSG->OPT[k++] = wizchip_mac[5];
	
	MSG->OPT[k++] = dhcpParamRequest;
	MSG->OPT[k++] = 0x08;
	MSG->OPT[k++] = subnetMask;
	MSG->OPT[k++] = routersOnSubnet;
	MSG->OPT[k++] = dns;
	MSG->OPT[k++] = domainName;
	MSG->OPT[k++] = dhcpT1value;
	MSG->OPT[k++] = dhcpT2value;
	MSG->OPT[k++] = performRouterDiscovery;
	MSG->OPT[k++] = staticRoute;
	MSG->OPT[k++] = endOption;

	for (i = k; i < OPT_SIZE; i++) MSG->OPT[i] = 0;

	// send broadcasting packet
	for (i = 0; i < 4; i++) ip[i] = d_addr[i];

#ifdef _DHCP_DEBUG_
	printf("> Send DHCP_REQUEST\r\n");
#endif
	
	sendto(DHCP_SOCKET, (uint8_t *)MSG, RIP_MSG_SIZE, ip, DHCP_SERVER_PORT);

}

/*
*********************************************************************************************************
*              SEND DHCP DHCPDECLINE
*
* Description: This function sends DHCP RELEASE message to DHCP server.
* Arguments  : s - is a socket number.
* Returns    : None.
* Note       : 
*********************************************************************************************************
*/
void send_DHCP_DECLINE(void)
{
	int i;
	uint8_t ip[4];
	uint16_t k = 0;

	MSG->op = DHCP_BOOTREQUEST;
	MSG->htype = DHCP_HTYPE10MB;
	MSG->hlen = DHCP_HLENETHERNET;
	MSG->hops = DHCP_HOPS;
	MSG->xid = DHCP_XID;
	MSG->secs = DHCP_SECS;
	MSG->flags = 0;

	MSG->ciaddr[0] = 0;
	MSG->ciaddr[1] = 0;
	MSG->ciaddr[2] = 0;
	MSG->ciaddr[3] = 0;

	MSG->yiaddr[0] = 0;
	MSG->yiaddr[1] = 0;
	MSG->yiaddr[2] = 0;
	MSG->yiaddr[3] = 0;

	MSG->siaddr[0] = 0;
	MSG->siaddr[1] = 0;
	MSG->siaddr[2] = 0;
	MSG->siaddr[3] = 0;

	MSG->giaddr[0] = 0;
	MSG->giaddr[1] = 0;
	MSG->giaddr[2] = 0;
	MSG->giaddr[3] = 0;

	MSG->chaddr[0] = wizchip_mac[0];
	MSG->chaddr[1] = wizchip_mac[1];
	MSG->chaddr[2] = wizchip_mac[2];
	MSG->chaddr[3] = wizchip_mac[3];
	MSG->chaddr[4] = wizchip_mac[4];
	MSG->chaddr[5] = wizchip_mac[5];

	for (i = 6; i < 16; i++) MSG->chaddr[i] = 0;
	for (i = 0; i < 64; i++) MSG->sname[i] = 0;
	for (i = 0; i < 128; i++) MSG->file[i] = 0;

	// MAGIC_COOKIE
	MSG->OPT[k++] = 0x63;
	MSG->OPT[k++] = 0x82;
	MSG->OPT[k++] = 0x53;
	MSG->OPT[k++] = 0x63;

	// Option Request Param.
	MSG->OPT[k++] = dhcpMessageType;
	MSG->OPT[k++] = 0x01;
	MSG->OPT[k++] = DHCP_DECLINE;

	MSG->OPT[k++] = dhcpClientIdentifier;
	MSG->OPT[k++] = 0x07;
	MSG->OPT[k++] = 0x01;
	MSG->OPT[k++] = wizchip_mac[0];
	MSG->OPT[k++] = wizchip_mac[1];
	MSG->OPT[k++] = wizchip_mac[2];
	MSG->OPT[k++] = wizchip_mac[3];
	MSG->OPT[k++] = wizchip_mac[4];
	MSG->OPT[k++] = wizchip_mac[5];

	MSG->OPT[k++] = dhcpRequestedIPaddr;
	MSG->OPT[k++] = 0x04;
	MSG->OPT[k++] = DHCP_allocated_ip[0];
	MSG->OPT[k++] = DHCP_allocated_ip[1];
	MSG->OPT[k++] = DHCP_allocated_ip[2];
	MSG->OPT[k++] = DHCP_allocated_ip[3];

	MSG->OPT[k++] = dhcpServerIdentifier;
	MSG->OPT[k++] = 0x04;
	MSG->OPT[k++] = DHCP_SIP[0];
	MSG->OPT[k++] = DHCP_SIP[1];
	MSG->OPT[k++] = DHCP_SIP[2];
	MSG->OPT[k++] = DHCP_SIP[3];

	MSG->OPT[k++] = endOption;

	for (i = k; i < OPT_SIZE; i++) MSG->OPT[i] = 0;

	//send broadcasting packet
	ip[0] = 0xFF;
	ip[1] = 0xFF;
	ip[2] = 0xFF;
	ip[3] = 0xFF;

#ifdef _DHCP_DEBUG_
	printf("\r\n> Send DHCP_DECLINE\r\n");
#endif

	sendto(DHCP_SOCKET, (uint8_t *)MSG, RIP_MSG_SIZE, ip, DHCP_SERVER_PORT);

}

/*
*********************************************************************************************************
*              PARSE REPLY MSG
*
* Description: This function parses the reply message from DHCP server.
* Arguments  : s      - is a socket number.
*              length - is a size data to receive.
* Returns    : None.
* Note       : 
*********************************************************************************************************
*/
int8_t parseDHCPMSG(uint16_t length)
{
	uint8_t svr_addr[6];
	uint16_t  svr_port;

	uint16_t i, len;
	uint8_t * p;
	uint8_t * e;
	uint8_t type = 0;
	uint8_t opt_len;

	len = recvfrom(DHCP_SOCKET, (uint8_t *)MSG, length, svr_addr, &svr_port);

	if (svr_port == DHCP_SERVER_PORT) {

		for (i = 0; i < 6; i++)
			if (MSG->chaddr[i] != wizchip_mac[i]) {
				type = 0;
				goto PARSE_END;
			}

		for (i = 0; i < 4; i++) {
			DHCP_allocated_ip[i] = MSG->yiaddr[i];
		}
		
		type = 0;
		p = (uint8_t *)(&MSG->op);
		p = p + 240;
		e = p + (len - 240);

		while ( p < e ) {

			switch ( *p ) {

			case endOption :
				goto PARSE_END;
       			case padOption :
				p++;
				break;
			case dhcpMessageType :
				p++;
				p++;
				type = *p++;
				break;
			case subnetMask :
				p++;
				p++;
				for (i = 0; i < 4; i++)	 DHCP_allocated_sn[i] = *p++;
				break;
			case routersOnSubnet :
				p++;
				opt_len = *p++;       
				for (i = 0; i < 4; i++)	DHCP_allocated_gw[i] = *p++;
				for (i = 0; i < (opt_len-4); i++) p++;
				break;
			
			case dns :
				p++;                  
				opt_len = *p++;       
				for (i = 0; i < 4; i++)	DHCP_allocated_dns[i] = *p++;
				for (i = 0; i < (opt_len-4); i++) p++;
				break;
				
				
			case dhcpIPaddrLeaseTime :
				p++;
				opt_len = *p++;
				lease_time.cVal[3] = *p++;
				lease_time.cVal[2] = *p++;
				lease_time.cVal[1] = *p++;
				lease_time.cVal[0] = *p++;
				break;

			case dhcpServerIdentifier :
				p++;
				opt_len = *p++;
				DHCP_SIP[0] = *p++;
				DHCP_SIP[1] = *p++;
				DHCP_SIP[2] = *p++;
				DHCP_SIP[3] = *p++;
				break;

			default :
				p++;
				opt_len = *p++;
				p += opt_len;
				break;
			} // switch
		} // while
	} // if

PARSE_END :
	return	type;
}

/*
*********************************************************************************************************
*              CHECK DHCP STATE
*
* Description: This function checks the state of DHCP.
* Arguments  : None.
* Returns    :  DHCP_FAILED
* 				DHCP_SUCCESS
* 				DHCP_IP_LEASED
* 				DHCP_RUNNING
* 				DHCP_IP_CHANGED
* Note       : 
*********************************************************************************************************
*/
uint8_t check_DHCP_state(void)
{
	uint16_t len, i;
	uint8_t type, DHCP_ip_changed_flag;
	uint8_t d_addr[4];
	uint8_t ret;
	
	ret = DHCP_RUNNING;
	type = 0;
	
	if ((len = getSn_RX_RSR(DHCP_SOCKET)) > 0) {
		type = parseDHCPMSG(len);
	}
	switch ( dhcp_state ) {
		case STATE_DHCP_DISCOVER :
			if (type == DHCP_OFFER) {

#ifdef _DHCP_DEBUG_
				printf("> Receive DHCP_OFFER\r\n");
#endif
				
				for (i = 0; i < 4; i++) d_addr[i] = 0xff;
				send_DHCP_REQUEST(Cip, d_addr);
				
				dhcp_state = STATE_DHCP_REQUEST;
			} else ret = check_DHCP_timeout();
		break;

		case STATE_DHCP_REQUEST :
			if (type == DHCP_ACK) {

#ifdef _DHCP_DEBUG_
				printf("> Receive DHCP_ACK\r\n");
#endif
				
				if (check_DHCP_leasedIP()) {
					// Network info assignment from DHCP
					dhcp_ip_assign();
					reset_DHCP_timeout();

					dhcp_state = STATE_DHCP_LEASED;
					ret = DHCP_SUCCESS;
				} else {
					// IP address conflict occurred
					reset_DHCP_timeout();
					dhcp_ip_conflict();
				}
			} else if (type == DHCP_NAK) {

#ifdef _DHCP_DEBUG_
				printf("> Receive DHCP_NACK\r\n");
#endif

				reset_DHCP_timeout();

				dhcp_state = STATE_DHCP_DISCOVER;
			} else ret = check_DHCP_timeout();
		break;

		case STATE_DHCP_LEASED :
			if ((lease_time.lVal != 0xffffffff) && ((lease_time.lVal/2) < my_time)) {
				
#ifdef _DHCP_DEBUG_
 				printf("> Renewal IP address \r\n");
#endif

				type = 0;
				for (i = 0; i < 4; i++)	OLD_SIP[i] = DHCP_allocated_ip[i];
				for (i = 0; i < 4; i++)	d_addr[i] = DHCP_SIP[i];
				
				DHCP_XID++;

				socket(DHCP_SOCKET, Sn_MR_UDP, DHCP_CLIENT_PORT, 0x00); // added
				send_DHCP_REQUEST(DHCP_allocated_ip, d_addr);

				reset_DHCP_timeout();

				dhcp_state = STATE_DHCP_REREQUEST;
			} else {
				ret = DHCP_IP_LEASED;
			}
		break;

		case STATE_DHCP_REREQUEST :
			if (type == DHCP_ACK) {

#ifdef _DHCP_DEBUG_
				printf("> Receive DHCP_ACK, IP Renewal Success\r\n");
#endif

				retry_count = 0;
				DHCP_ip_changed_flag = 0;

				for (i = 0; i < 4; i++)	{
					if (OLD_SIP[i] != DHCP_allocated_ip[i]) {
						DHCP_ip_changed_flag = 1;
						break;
					}
				}

				// change to new IP address
				if (DHCP_ip_changed_flag) {
					ret = DHCP_IP_CHANGED;
					dhcp_ip_update();
				}
				reset_DHCP_timeout();

				dhcp_state = STATE_DHCP_LEASED;
			} else if (type == DHCP_NAK) {

#ifdef _DHCP_DEBUG_
				printf("> Receive DHCP_NACK, IP Renewal Failed\r\n");
#endif

				reset_DHCP_timeout();

				dhcp_state = STATE_DHCP_DISCOVER;
			} else ret = check_DHCP_timeout();
		break;

		case STATE_DHCP_RELEASE :
		break;

		default :
		break;
	}

	return ret;
}

/*
*********************************************************************************************************
*              CHECK DHCP TIMEOUT
*
* Description: This function checks the timeout of DHCP in each state.
* Arguments  : None.
* Returns    : DHCP_FAILED 	- Timeout occurred,
* 			   DHCP_RUNNING - No timeout
* Note       : 
*********************************************************************************************************
*/
uint8_t check_DHCP_timeout(void)
{
	uint8_t i, d_addr[4];
	uint8_t ret = DHCP_RUNNING;
	
	if (retry_count < MAX_DHCP_RETRY) {
		if (next_time < my_time) {

			switch ( dhcp_state ) {
				case STATE_DHCP_DISCOVER :
//					printf("<<timeout>> state : STATE_DHCP_DISCOVER\r\n");
					send_DHCP_DISCOVER();
				break;
		
				case STATE_DHCP_REQUEST :
//					printf("<<timeout>> state : STATE_DHCP_REQUEST\r\n");

					for (i = 0; i < 4; i++) d_addr[i] = 0xff;
					send_DHCP_REQUEST(Cip, d_addr);
				break;

				case STATE_DHCP_REREQUEST :
//					printf("<<timeout>> state : STATE_DHCP_REREQUEST\r\n");
					
					for (i = 0; i < 4; i++)	d_addr[i] = DHCP_SIP[i];
					send_DHCP_REQUEST(DHCP_allocated_ip, d_addr);
				break;
		
				default :
				break;
			}

			my_time = 0;
			next_time = my_time + DHCP_WAIT_TIME;
			retry_count++;
		}
	} else { // timeout occurred

		switch(dhcp_state) {
			case STATE_DHCP_DISCOVER:
				dhcp_state = STATE_DHCP_INIT;
				ret = DHCP_FAILED;
				break;
			case STATE_DHCP_REQUEST:
			case STATE_DHCP_REREQUEST:
				send_DHCP_DISCOVER();
				dhcp_state = STATE_DHCP_DISCOVER;
				break;
			default :
				break;
		}
		reset_DHCP_timeout();
	}
	return ret;
}

/*
*********************************************************************************************************
*              CHECK DHCP TIMEOUT
*
* Description: This function checks if a leased IP is valid
* Arguments  : None.
* Returns    : 1 - leased IP OK,
* 			   0 - IP conflict occurred
* Note       :
*********************************************************************************************************
*/
//
int8_t check_DHCP_leasedIP(void)
{
	uint8_t tmp;
	int32_t ret;

	//WIZchip RCR value changed for ARP Timeout count control
	tmp = getRCR();
	setRCR(0x03);

	// IP conflict detection : ARP request - ARP reply
	// Broadcasting ARP Request for check the IP conflict using UDP sendto() function
	ret = sendto(DHCP_SOCKET, (uint8_t *)"CHECK_IP_CONFLICT", 17, DHCP_allocated_ip, 5000);

	// RCR value restore
	setRCR(tmp);

	if(ret == SOCKERR_TIMEOUT) {
		// UDP send Timeout occurred : allocated IP address is unique, DHCP Success

#ifdef _DHCP_DEBUG_
		printf("\r\n> Check leased IP - OK\r\n");
#endif

		return 1;
	} else {
		// Received ARP reply or etc : IP address conflict occur, DHCP Failed
		send_DHCP_DECLINE();
		for(tmp = 0; tmp < 10; tmp++) getMR(); // for delay
		return 0;
	}
}	

/*
*********************************************************************************************************
*              DHCP INIT
*
* Description: this function initialize DHCP client.
* Arguments  : s 	- is a socket number,
* 			   buf  - is memory space for DHCP message.
* Returns    : None.
* Note       : 
*********************************************************************************************************
*/

void DHCP_init(uint8_t s, uint8_t * buf)
{
	DHCP_SOCKET = s; // SOCK_DHCP
	MSG = (RIP_MSG*)buf;
	DHCP_XID = 0x12345678;

	// Get the MAC address
	getSHAR(wizchip_mac);

	// WIZchip Netinfo Clear
	setSIPR(Cip);
	setSIPR(Cip);
	setGAR(Cip);

	dhcp_state = STATE_DHCP_INIT;
	socket(DHCP_SOCKET, Sn_MR_UDP, DHCP_CLIENT_PORT, 0x00);
}

/*
*********************************************************************************************************
*				DHCP RUN
*
* Description: this function get an IP from the DHCP server and check the DHCP lease time in main loop
* Arguments  : None.
* Returns    : 	DHCP_FAILED
* 				DHCP_SUCCESS
* 				DHCP_IP_LEASED
* 				DHCP_RUNNING
* 				DHCP_IP_CHANGED
* Note       :
*********************************************************************************************************
*/

uint8_t DHCP_run(void)
{
	if(dhcp_state == STATE_DHCP_INIT) {
		send_DHCP_DISCOVER();
		dhcp_state = STATE_DHCP_DISCOVER;
		reset_DHCP_timeout();
	}

	return check_DHCP_state();
}


/*
*********************************************************************************************************
*				RESET DHCP TIMEOUT
*
* Description: this function reset the DHCP timeout count and retry count.
* Arguments  : None.
* Returns    : None.
* Note       :
*********************************************************************************************************
*/
void reset_DHCP_timeout(void)
{
	my_time = 0;
	next_time = DHCP_WAIT_TIME;
	retry_count = 0;
}

/*
*********************************************************************************************************
*				DHCP TIMER HANDLER
*
* Description: This function must be called once per second in the timer interrupt handler.
* Arguments  : None.
* Returns    : None.
* Note       :
*********************************************************************************************************
*/
void DHCP_timerHandler(void)
{
	my_time++;
}

/*
*********************************************************************************************************
*				GET NETWORK INFORMATION FROM DHCP
*
* Description: These functions get the DHCP server assigned network information.
* Arguments  : Netinfo array
* Returns    : None.
* Note       : getIPfromDHCP,
* 			   getGWfromDHCP,
* 			   getSNfromDHCP,
* 			   getDNSfromDHCP
*********************************************************************************************************
*/
void getIPfromDHCP(uint8_t * ip)
{
	ip[0] = DHCP_allocated_ip[0];
	ip[1] = DHCP_allocated_ip[1];
	ip[2] = DHCP_allocated_ip[2];
	ip[3] = DHCP_allocated_ip[3];
}

void getGWfromDHCP(uint8_t * gw)
{
	gw[0] = DHCP_allocated_gw[0];
	gw[1] = DHCP_allocated_gw[1];
	gw[2] = DHCP_allocated_gw[2];
	gw[3] = DHCP_allocated_gw[3];
}

void getSNfromDHCP(uint8_t * sn)
{
	sn[0] = DHCP_allocated_sn[0];
	sn[1] = DHCP_allocated_sn[1];
	sn[2] = DHCP_allocated_sn[2];
	sn[3] = DHCP_allocated_sn[3];
}

void getDNSfromDHCP(uint8_t * dns)
{
	dns[0] = DHCP_allocated_dns[0];
	dns[1] = DHCP_allocated_dns[1];
	dns[2] = DHCP_allocated_dns[2];
	dns[3] = DHCP_allocated_dns[3];
}

/*
*********************************************************************************************************
*				GET DHCP LEASEDTIME
*
* Description: These functions get the DHCP IP leased time.
* Arguments  : None.
* Returns    : uint32_t DHCPleaseTime
* Note       :
*********************************************************************************************************
*/
uint32_t getDHCPLeasetime(void)
{
	return lease_time.lVal;
}



