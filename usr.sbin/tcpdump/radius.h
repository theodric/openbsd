/*	$OpenBSD: src/usr.sbin/tcpdump/radius.h,v 1.2 2000/10/03 14:31:58 ho Exp $	*/

/* RADIUS support for tcpdump, Thomas Ptacek <tqbf@enteract.com> */

/* ------------------------------------------------------------ */

/* RADIUS attribute encoding types */

#define RD_INT				1
#define RD_DATE				2
#define RD_ADDRESS			3
#define RD_STRING			4
#define RD_HEX				5

/* ------------------------------------------------------------ */

/* RADIUS packet opcodes */

#define RADIUS_CODE_ACCESS_REQUEST		1
#define RADIUS_CODE_ACCESS_ACCEPT		2
#define RADIUS_CODE_ACCESS_REJECT		3
#define RADIUS_CODE_ACCOUNT_REQUEST		4
#define RADIUS_CODE_ACCOUNT_RESPONSE		5
#define RADIUS_CODE_ACCOUNT_STATUS		6
#define RADIUS_CODE_PASSCHG_REQUEST		7
#define RADIUS_CODE_PASSCHG_ACCEPT		8
#define RADIUS_CODE_PASSCHG_REJECT		9
#define RADIUS_CODE_ACCOUNT_MESSAGE		10
#define RADIUS_CODE_ACCESS_CHALLENGE		11

/* ------------------------------------------------------------ */

/* slew o' attributes */

#define RADIUS_ATT_USER_NAME			1
#define RADIUS_ATT_PASSWORD			2
#define RADIUS_ATT_CHAP_PASS			3
#define RADIUS_ATT_NAS_IP			4
#define RADIUS_ATT_NAS_PORT			5
#define RADIUS_ATT_USER_SERVICE			6
#define RADIUS_ATT_PROTOCOL			7
#define RADIUS_ATT_FRAMED_ADDRESS		8
#define RADIUS_ATT_NETMASK			9
#define RADIUS_ATT_ROUTING			10
#define RADIUS_ATT_FILTER			11
#define RADIUS_ATT_MTU				12
#define RADIUS_ATT_COMPRESSION			13
#define RADIUS_ATT_LOGIN_HOST			14
#define RADIUS_ATT_LOGIN_SERVICE		15
#define RADIUS_ATT_LOGIN_TCP_PORT		16
#define RADIUS_ATT_OLD_PASSWORD			17
#define RADIUS_ATT_PORT_MESSAGE			18
#define RADIUS_ATT_DIALBACK_NO			19
#define RADIUS_ATT_DIALBACK_NAME		20
#define RADIUS_ATT_EXPIRATION			21
#define RADIUS_ATT_FRAMED_ROUTE			22
#define RADIUS_ATT_FRAMED_IPX			23
#define RADIUS_ATT_CHALLENGE_STATE		24
#define RADIUS_ATT_CLASS			25
#define RADIUS_ATT_VENDOR_SPECIFIC		26
#define RADIUS_ATT_SESSION_TIMEOUT		27
#define RADIUS_ATT_IDLE_TIMEOUT			28
#define RADIUS_ATT_TERMINATE_ACTION		29
#define RADIUS_ATT_CALLED_ID			30
#define RADIUS_ATT_CALLER_ID			31

#define RADIUS_ATT_STATUS_TYPE			40

/* the accounting attributes change way too much 
 * for me to want to hardcode them in. 
 */

/* ------------------------------------------------------------ */

/* RADIUS packet header */

#define RADFIXEDSZ		20

struct radius_header {
	u_char	code;
	u_char 	id;
	u_short len;
	u_char 	auth[16];
};

/* ------------------------------------------------------------ */
