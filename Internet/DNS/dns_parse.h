#ifndef	_DNS_PARSE_H_
#define	_DNS_PARSE_H_

/*
********************************************************************************
Define Part
********************************************************************************
*/

#define	MAX_DNS_BUF_SIZE	256		/* maximum size of DNS buffer. */

/*
********************************************************************************
Function Prototype Definition Part
********************************************************************************
*/

uint8_t parseMSG(struct dhdr * dhdr, uint8_t * buf, uint8_t * ip_from_dns);

#endif /* _DNS_PARSE_H_ */

