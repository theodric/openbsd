/* fcrypt.c */
/* Copyright (C) 1993 Eric Young - see README for more details */
#include <stdio.h>

/* Eric Young.
 * This version of crypt has been developed from my MIT compatable
 * DES library.
 * The library is available at pub/DES at ftp.psy.uq.oz.au
 * eay@psych.psy.uq.oz.au
 */

#if defined(__BORLANDC__)
#pragma warn -8004	/* "'foo' is assigned a value that is never used" */
#endif

typedef unsigned char des_cblock[8];

typedef struct des_ks_struct
	{
	union	{
		des_cblock _;
		/* make sure things are correct size on machines with
		 * 8 byte longs */
		unsigned long pad[2];
		} ks;
#define _	ks._
	} des_key_schedule[16];

#define DES_KEY_SZ 	(sizeof(des_cblock))
#define DES_ENCRYPT	1
#define DES_DECRYPT	0

#define ITERATIONS 16
#define HALF_ITERATIONS 8

#define c2l(c,l)	(l =((unsigned long)(*((c)++)))    , \
			 l|=((unsigned long)(*((c)++)))<< 8, \
			 l|=((unsigned long)(*((c)++)))<<16, \
			 l|=((unsigned long)(*((c)++)))<<24)

#define l2c(l,c)	(*((c)++)=(unsigned char)(((l)    )&0xff), \
			 *((c)++)=(unsigned char)(((l)>> 8)&0xff), \
			 *((c)++)=(unsigned char)(((l)>>16)&0xff), \
			 *((c)++)=(unsigned char)(((l)>>24)&0xff))

static unsigned long SPtrans[8][64]={
{ /* nibble 0 */
0x00820200, 0x00020000, 0x80800000, 0x80820200,
0x00800000, 0x80020200, 0x80020000, 0x80800000,
0x80020200, 0x00820200, 0x00820000, 0x80000200,
0x80800200, 0x00800000, 0x00000000, 0x80020000,
0x00020000, 0x80000000, 0x00800200, 0x00020200,
0x80820200, 0x00820000, 0x80000200, 0x00800200,
0x80000000, 0x00000200, 0x00020200, 0x80820000,
0x00000200, 0x80800200, 0x80820000, 0x00000000,
0x00000000, 0x80820200, 0x00800200, 0x80020000,
0x00820200, 0x00020000, 0x80000200, 0x00800200,
0x80820000, 0x00000200, 0x00020200, 0x80800000,
0x80020200, 0x80000000, 0x80800000, 0x00820000,
0x80820200, 0x00020200, 0x00820000, 0x80800200,
0x00800000, 0x80000200, 0x80020000, 0x00000000,
0x00020000, 0x00800000, 0x80800200, 0x00820200,
0x80000000, 0x80820000, 0x00000200, 0x80020200},
{ /* nibble 1 */
0x10042004, 0x00000000, 0x00042000, 0x10040000,
0x10000004, 0x00002004, 0x10002000, 0x00042000,
0x00002000, 0x10040004, 0x00000004, 0x10002000,
0x00040004, 0x10042000, 0x10040000, 0x00000004,
0x00040000, 0x10002004, 0x10040004, 0x00002000,
0x00042004, 0x10000000, 0x00000000, 0x00040004,
0x10002004, 0x00042004, 0x10042000, 0x10000004,
0x10000000, 0x00040000, 0x00002004, 0x10042004,
0x00040004, 0x10042000, 0x10002000, 0x00042004,
0x10042004, 0x00040004, 0x10000004, 0x00000000,
0x10000000, 0x00002004, 0x00040000, 0x10040004,
0x00002000, 0x10000000, 0x00042004, 0x10002004,
0x10042000, 0x00002000, 0x00000000, 0x10000004,
0x00000004, 0x10042004, 0x00042000, 0x10040000,
0x10040004, 0x00040000, 0x00002004, 0x10002000,
0x10002004, 0x00000004, 0x10040000, 0x00042000},
{ /* nibble 2 */
0x41000000, 0x01010040, 0x00000040, 0x41000040,
0x40010000, 0x01000000, 0x41000040, 0x00010040,
0x01000040, 0x00010000, 0x01010000, 0x40000000,
0x41010040, 0x40000040, 0x40000000, 0x41010000,
0x00000000, 0x40010000, 0x01010040, 0x00000040,
0x40000040, 0x41010040, 0x00010000, 0x41000000,
0x41010000, 0x01000040, 0x40010040, 0x01010000,
0x00010040, 0x00000000, 0x01000000, 0x40010040,
0x01010040, 0x00000040, 0x40000000, 0x00010000,
0x40000040, 0x40010000, 0x01010000, 0x41000040,
0x00000000, 0x01010040, 0x00010040, 0x41010000,
0x40010000, 0x01000000, 0x41010040, 0x40000000,
0x40010040, 0x41000000, 0x01000000, 0x41010040,
0x00010000, 0x01000040, 0x41000040, 0x00010040,
0x01000040, 0x00000000, 0x41010000, 0x40000040,
0x41000000, 0x40010040, 0x00000040, 0x01010000},
{ /* nibble 3 */
0x00100402, 0x04000400, 0x00000002, 0x04100402,
0x00000000, 0x04100000, 0x04000402, 0x00100002,
0x04100400, 0x04000002, 0x04000000, 0x00000402,
0x04000002, 0x00100402, 0x00100000, 0x04000000,
0x04100002, 0x00100400, 0x00000400, 0x00000002,
0x00100400, 0x04000402, 0x04100000, 0x00000400,
0x00000402, 0x00000000, 0x00100002, 0x04100400,
0x04000400, 0x04100002, 0x04100402, 0x00100000,
0x04100002, 0x00000402, 0x00100000, 0x04000002,
0x00100400, 0x04000400, 0x00000002, 0x04100000,
0x04000402, 0x00000000, 0x00000400, 0x00100002,
0x00000000, 0x04100002, 0x04100400, 0x00000400,
0x04000000, 0x04100402, 0x00100402, 0x00100000,
0x04100402, 0x00000002, 0x04000400, 0x00100402,
0x00100002, 0x00100400, 0x04100000, 0x04000402,
0x00000402, 0x04000000, 0x04000002, 0x04100400},
{ /* nibble 4 */
0x02000000, 0x00004000, 0x00000100, 0x02004108,
0x02004008, 0x02000100, 0x00004108, 0x02004000,
0x00004000, 0x00000008, 0x02000008, 0x00004100,
0x02000108, 0x02004008, 0x02004100, 0x00000000,
0x00004100, 0x02000000, 0x00004008, 0x00000108,
0x02000100, 0x00004108, 0x00000000, 0x02000008,
0x00000008, 0x02000108, 0x02004108, 0x00004008,
0x02004000, 0x00000100, 0x00000108, 0x02004100,
0x02004100, 0x02000108, 0x00004008, 0x02004000,
0x00004000, 0x00000008, 0x02000008, 0x02000100,
0x02000000, 0x00004100, 0x02004108, 0x00000000,
0x00004108, 0x02000000, 0x00000100, 0x00004008,
0x02000108, 0x00000100, 0x00000000, 0x02004108,
0x02004008, 0x02004100, 0x00000108, 0x00004000,
0x00004100, 0x02004008, 0x02000100, 0x00000108,
0x00000008, 0x00004108, 0x02004000, 0x02000008},
{ /* nibble 5 */
0x20000010, 0x00080010, 0x00000000, 0x20080800,
0x00080010, 0x00000800, 0x20000810, 0x00080000,
0x00000810, 0x20080810, 0x00080800, 0x20000000,
0x20000800, 0x20000010, 0x20080000, 0x00080810,
0x00080000, 0x20000810, 0x20080010, 0x00000000,
0x00000800, 0x00000010, 0x20080800, 0x20080010,
0x20080810, 0x20080000, 0x20000000, 0x00000810,
0x00000010, 0x00080800, 0x00080810, 0x20000800,
0x00000810, 0x20000000, 0x20000800, 0x00080810,
0x20080800, 0x00080010, 0x00000000, 0x20000800,
0x20000000, 0x00000800, 0x20080010, 0x00080000,
0x00080010, 0x20080810, 0x00080800, 0x00000010,
0x20080810, 0x00080800, 0x00080000, 0x20000810,
0x20000010, 0x20080000, 0x00080810, 0x00000000,
0x00000800, 0x20000010, 0x20000810, 0x20080800,
0x20080000, 0x00000810, 0x00000010, 0x20080010},
{ /* nibble 6 */
0x00001000, 0x00000080, 0x00400080, 0x00400001,
0x00401081, 0x00001001, 0x00001080, 0x00000000,
0x00400000, 0x00400081, 0x00000081, 0x00401000,
0x00000001, 0x00401080, 0x00401000, 0x00000081,
0x00400081, 0x00001000, 0x00001001, 0x00401081,
0x00000000, 0x00400080, 0x00400001, 0x00001080,
0x00401001, 0x00001081, 0x00401080, 0x00000001,
0x00001081, 0x00401001, 0x00000080, 0x00400000,
0x00001081, 0x00401000, 0x00401001, 0x00000081,
0x00001000, 0x00000080, 0x00400000, 0x00401001,
0x00400081, 0x00001081, 0x00001080, 0x00000000,
0x00000080, 0x00400001, 0x00000001, 0x00400080,
0x00000000, 0x00400081, 0x00400080, 0x00001080,
0x00000081, 0x00001000, 0x00401081, 0x00400000,
0x00401080, 0x00000001, 0x00001001, 0x00401081,
0x00400001, 0x00401080, 0x00401000, 0x00001001},
{ /* nibble 7 */
0x08200020, 0x08208000, 0x00008020, 0x00000000,
0x08008000, 0x00200020, 0x08200000, 0x08208020,
0x00000020, 0x08000000, 0x00208000, 0x00008020,
0x00208020, 0x08008020, 0x08000020, 0x08200000,
0x00008000, 0x00208020, 0x00200020, 0x08008000,
0x08208020, 0x08000020, 0x00000000, 0x00208000,
0x08000000, 0x00200000, 0x08008020, 0x08200020,
0x00200000, 0x00008000, 0x08208000, 0x00000020,
0x00200000, 0x00008000, 0x08000020, 0x08208020,
0x00008020, 0x08000000, 0x00000000, 0x00208000,
0x08200020, 0x08008020, 0x08008000, 0x00200020,
0x08208000, 0x00000020, 0x00200020, 0x08008000,
0x08208020, 0x00200000, 0x08200000, 0x08000020,
0x00208000, 0x00008020, 0x08008020, 0x08200000,
0x00000020, 0x08208000, 0x00208020, 0x00000000,
0x08000000, 0x08200020, 0x00008000, 0x00208020}
};
static unsigned long skb[8][64]={
{ /* for C bits (numbered as per FIPS 46) 1 2 3 4 5 6 */
0x00000000,0x00000010,0x20000000,0x20000010,
0x00010000,0x00010010,0x20010000,0x20010010,
0x00000800,0x00000810,0x20000800,0x20000810,
0x00010800,0x00010810,0x20010800,0x20010810,
0x00000020,0x00000030,0x20000020,0x20000030,
0x00010020,0x00010030,0x20010020,0x20010030,
0x00000820,0x00000830,0x20000820,0x20000830,
0x00010820,0x00010830,0x20010820,0x20010830,
0x00080000,0x00080010,0x20080000,0x20080010,
0x00090000,0x00090010,0x20090000,0x20090010,
0x00080800,0x00080810,0x20080800,0x20080810,
0x00090800,0x00090810,0x20090800,0x20090810,
0x00080020,0x00080030,0x20080020,0x20080030,
0x00090020,0x00090030,0x20090020,0x20090030,
0x00080820,0x00080830,0x20080820,0x20080830,
0x00090820,0x00090830,0x20090820,0x20090830},
{ /* for C bits (numbered as per FIPS 46) 7 8 10 11 12 13 */
0x00000000,0x02000000,0x00002000,0x02002000,
0x00200000,0x02200000,0x00202000,0x02202000,
0x00000004,0x02000004,0x00002004,0x02002004,
0x00200004,0x02200004,0x00202004,0x02202004,
0x00000400,0x02000400,0x00002400,0x02002400,
0x00200400,0x02200400,0x00202400,0x02202400,
0x00000404,0x02000404,0x00002404,0x02002404,
0x00200404,0x02200404,0x00202404,0x02202404,
0x10000000,0x12000000,0x10002000,0x12002000,
0x10200000,0x12200000,0x10202000,0x12202000,
0x10000004,0x12000004,0x10002004,0x12002004,
0x10200004,0x12200004,0x10202004,0x12202004,
0x10000400,0x12000400,0x10002400,0x12002400,
0x10200400,0x12200400,0x10202400,0x12202400,
0x10000404,0x12000404,0x10002404,0x12002404,
0x10200404,0x12200404,0x10202404,0x12202404},
{ /* for C bits (numbered as per FIPS 46) 14 15 16 17 19 20 */
0x00000000,0x00000001,0x00040000,0x00040001,
0x01000000,0x01000001,0x01040000,0x01040001,
0x00000002,0x00000003,0x00040002,0x00040003,
0x01000002,0x01000003,0x01040002,0x01040003,
0x00000200,0x00000201,0x00040200,0x00040201,
0x01000200,0x01000201,0x01040200,0x01040201,
0x00000202,0x00000203,0x00040202,0x00040203,
0x01000202,0x01000203,0x01040202,0x01040203,
0x08000000,0x08000001,0x08040000,0x08040001,
0x09000000,0x09000001,0x09040000,0x09040001,
0x08000002,0x08000003,0x08040002,0x08040003,
0x09000002,0x09000003,0x09040002,0x09040003,
0x08000200,0x08000201,0x08040200,0x08040201,
0x09000200,0x09000201,0x09040200,0x09040201,
0x08000202,0x08000203,0x08040202,0x08040203,
0x09000202,0x09000203,0x09040202,0x09040203},
{ /* for C bits (numbered as per FIPS 46) 21 23 24 26 27 28 */
0x00000000,0x00100000,0x00000100,0x00100100,
0x00000008,0x00100008,0x00000108,0x00100108,
0x00001000,0x00101000,0x00001100,0x00101100,
0x00001008,0x00101008,0x00001108,0x00101108,
0x04000000,0x04100000,0x04000100,0x04100100,
0x04000008,0x04100008,0x04000108,0x04100108,
0x04001000,0x04101000,0x04001100,0x04101100,
0x04001008,0x04101008,0x04001108,0x04101108,
0x00020000,0x00120000,0x00020100,0x00120100,
0x00020008,0x00120008,0x00020108,0x00120108,
0x00021000,0x00121000,0x00021100,0x00121100,
0x00021008,0x00121008,0x00021108,0x00121108,
0x04020000,0x04120000,0x04020100,0x04120100,
0x04020008,0x04120008,0x04020108,0x04120108,
0x04021000,0x04121000,0x04021100,0x04121100,
0x04021008,0x04121008,0x04021108,0x04121108},
{ /* for D bits (numbered as per FIPS 46) 1 2 3 4 5 6 */
0x00000000,0x10000000,0x00010000,0x10010000,
0x00000004,0x10000004,0x00010004,0x10010004,
0x20000000,0x30000000,0x20010000,0x30010000,
0x20000004,0x30000004,0x20010004,0x30010004,
0x00100000,0x10100000,0x00110000,0x10110000,
0x00100004,0x10100004,0x00110004,0x10110004,
0x20100000,0x30100000,0x20110000,0x30110000,
0x20100004,0x30100004,0x20110004,0x30110004,
0x00001000,0x10001000,0x00011000,0x10011000,
0x00001004,0x10001004,0x00011004,0x10011004,
0x20001000,0x30001000,0x20011000,0x30011000,
0x20001004,0x30001004,0x20011004,0x30011004,
0x00101000,0x10101000,0x00111000,0x10111000,
0x00101004,0x10101004,0x00111004,0x10111004,
0x20101000,0x30101000,0x20111000,0x30111000,
0x20101004,0x30101004,0x20111004,0x30111004},
{ /* for D bits (numbered as per FIPS 46) 8 9 11 12 13 14 */
0x00000000,0x08000000,0x00000008,0x08000008,
0x00000400,0x08000400,0x00000408,0x08000408,
0x00020000,0x08020000,0x00020008,0x08020008,
0x00020400,0x08020400,0x00020408,0x08020408,
0x00000001,0x08000001,0x00000009,0x08000009,
0x00000401,0x08000401,0x00000409,0x08000409,
0x00020001,0x08020001,0x00020009,0x08020009,
0x00020401,0x08020401,0x00020409,0x08020409,
0x02000000,0x0A000000,0x02000008,0x0A000008,
0x02000400,0x0A000400,0x02000408,0x0A000408,
0x02020000,0x0A020000,0x02020008,0x0A020008,
0x02020400,0x0A020400,0x02020408,0x0A020408,
0x02000001,0x0A000001,0x02000009,0x0A000009,
0x02000401,0x0A000401,0x02000409,0x0A000409,
0x02020001,0x0A020001,0x02020009,0x0A020009,
0x02020401,0x0A020401,0x02020409,0x0A020409},
{ /* for D bits (numbered as per FIPS 46) 16 17 18 19 20 21 */
0x00000000,0x00000100,0x00080000,0x00080100,
0x01000000,0x01000100,0x01080000,0x01080100,
0x00000010,0x00000110,0x00080010,0x00080110,
0x01000010,0x01000110,0x01080010,0x01080110,
0x00200000,0x00200100,0x00280000,0x00280100,
0x01200000,0x01200100,0x01280000,0x01280100,
0x00200010,0x00200110,0x00280010,0x00280110,
0x01200010,0x01200110,0x01280010,0x01280110,
0x00000200,0x00000300,0x00080200,0x00080300,
0x01000200,0x01000300,0x01080200,0x01080300,
0x00000210,0x00000310,0x00080210,0x00080310,
0x01000210,0x01000310,0x01080210,0x01080310,
0x00200200,0x00200300,0x00280200,0x00280300,
0x01200200,0x01200300,0x01280200,0x01280300,
0x00200210,0x00200310,0x00280210,0x00280310,
0x01200210,0x01200310,0x01280210,0x01280310},
{ /* for D bits (numbered as per FIPS 46) 22 23 24 25 27 28 */
0x00000000,0x04000000,0x00040000,0x04040000,
0x00000002,0x04000002,0x00040002,0x04040002,
0x00002000,0x04002000,0x00042000,0x04042000,
0x00002002,0x04002002,0x00042002,0x04042002,
0x00000020,0x04000020,0x00040020,0x04040020,
0x00000022,0x04000022,0x00040022,0x04040022,
0x00002020,0x04002020,0x00042020,0x04042020,
0x00002022,0x04002022,0x00042022,0x04042022,
0x00000800,0x04000800,0x00040800,0x04040800,
0x00000802,0x04000802,0x00040802,0x04040802,
0x00002800,0x04002800,0x00042800,0x04042800,
0x00002802,0x04002802,0x00042802,0x04042802,
0x00000820,0x04000820,0x00040820,0x04040820,
0x00000822,0x04000822,0x00040822,0x04040822,
0x00002820,0x04002820,0x00042820,0x04042820,
0x00002822,0x04002822,0x00042822,0x04042822}
};

/* See ecb_encrypt.c for a pseudo description of these macros. */
#define PERM_OP(a,b,t,n,m) ((t)=((((a)>>(n))^(b))&(m)),\
	(b)^=(t),\
	(a)^=((t)<<(n)))

#define HPERM_OP(a,t,n,m) ((t)=((((a)<<(16-(n)))^(a))&(m)),\
	(a)=(a)^(t)^(t>>(16-(n))))\

static char shifts2[16]={0,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0};

static int body(
	unsigned long *out0,
	unsigned long *out1,
	des_key_schedule ks,
	unsigned long Eswap0,
	unsigned long Eswap1);

static int
des_set_key(des_cblock *key, des_key_schedule schedule)
	{
	register unsigned long c,d,t,s;
	register unsigned char *in;
	register unsigned long *k;
	register int i;

	k=(unsigned long *)schedule;
	in=(unsigned char *)key;

	c2l(in,c);
	c2l(in,d);

	/* I now do it in 47 simple operations :-)
	 * Thanks to John Fletcher (john_fletcher@lccmail.ocf.llnl.gov)
	 * for the inspiration. :-) */
	PERM_OP (d,c,t,4,0x0f0f0f0f);
	HPERM_OP(c,t,-2,0xcccc0000);
	HPERM_OP(d,t,-2,0xcccc0000);
	PERM_OP (d,c,t,1,0x55555555);
	PERM_OP (c,d,t,8,0x00ff00ff);
	PERM_OP (d,c,t,1,0x55555555);
	d=	(((d&0x000000ff)<<16)| (d&0x0000ff00)     |
		 ((d&0x00ff0000)>>16)|((c&0xf0000000)>>4));
	c&=0x0fffffff;

	for (i=0; i<ITERATIONS; i++)
		{
		if (shifts2[i])
			{ c=((c>>2)|(c<<26)); d=((d>>2)|(d<<26)); }
		else
			{ c=((c>>1)|(c<<27)); d=((d>>1)|(d<<27)); }
		c&=0x0fffffff;
		d&=0x0fffffff;
		/* could be a few less shifts but I am to lazy at this
		 * point in time to investigate */
		s=	skb[0][ (c    )&0x3f                ]|
			skb[1][((c>> 6)&0x03)|((c>> 7)&0x3c)]|
			skb[2][((c>>13)&0x0f)|((c>>14)&0x30)]|
			skb[3][((c>>20)&0x01)|((c>>21)&0x06) |
			                      ((c>>22)&0x38)];
		t=	skb[4][ (d    )&0x3f                ]|
			skb[5][((d>> 7)&0x03)|((d>> 8)&0x3c)]|
			skb[6][ (d>>15)&0x3f                ]|
			skb[7][((d>>21)&0x0f)|((d>>22)&0x30)];

		/* table contained 0213 4657 */
		*(k++)=((t<<16)|(s&0x0000ffff))&0xffffffff;
		s=     ((s>>16)|(t&0xffff0000));
		
		s=(s<<4)|(s>>28);
		*(k++)=s&0xffffffff;
		}
	return(0);
	}

/******************************************************************
 * modified stuff for crypt.
 ******************************************************************/

/* The changes to this macro may help or hinder, depending on the
 * compiler and the achitecture.  gcc2 always seems to do well :-). 
 * Inspired by Dana How <how@isl.stanford.edu>
 * DO NOT use the alternative version on machines with 8 byte longs.
 */
#ifdef ALT_ECB
#define D_ENCRYPT(L,R,S) \
	v=(R^(R>>16)); \
	u=(v&E0); \
	v=(v&E1); \
	u=((u^(u<<16))^R^s[S  ])<<2; \
	t=(v^(v<<16))^R^s[S+1]; \
	t=(t>>2)|(t<<30); \
	L^= \
	*(unsigned long *)(des_SP+0x0100+((t    )&0xfc))+ \
	*(unsigned long *)(des_SP+0x0300+((t>> 8)&0xfc))+ \
	*(unsigned long *)(des_SP+0x0500+((t>>16)&0xfc))+ \
	*(unsigned long *)(des_SP+0x0700+((t>>24)&0xfc))+ \
	*(unsigned long *)(des_SP+       ((u    )&0xfc))+ \
  	*(unsigned long *)(des_SP+0x0200+((u>> 8)&0xfc))+ \
  	*(unsigned long *)(des_SP+0x0400+((u>>16)&0xfc))+ \
 	*(unsigned long *)(des_SP+0x0600+((u>>24)&0xfc));
#else /* original version */
#define D_ENCRYPT(L,R,S)	\
	v=(R^(R>>16)); \
	u=(v&E0); \
	v=(v&E1); \
	u=(u^(u<<16))^R^s[S  ]; \
	t=(v^(v<<16))^R^s[S+1]; \
	t=(t>>4)|(t<<28); \
	L^=	SPtrans[1][(t    )&0x3f]| \
		SPtrans[3][(t>> 8)&0x3f]| \
		SPtrans[5][(t>>16)&0x3f]| \
		SPtrans[7][(t>>24)&0x3f]| \
		SPtrans[0][(u    )&0x3f]| \
		SPtrans[2][(u>> 8)&0x3f]| \
		SPtrans[4][(u>>16)&0x3f]| \
		SPtrans[6][(u>>24)&0x3f];
#endif

unsigned char con_salt[128]={
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,
0x0A,0x0B,0x05,0x06,0x07,0x08,0x09,0x0A,
0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,
0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,
0x1B,0x1C,0x1D,0x1E,0x1F,0x20,0x21,0x22,
0x23,0x24,0x25,0x20,0x21,0x22,0x23,0x24,
0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,
0x2D,0x2E,0x2F,0x30,0x31,0x32,0x33,0x34,
0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,
0x3D,0x3E,0x3F,0x00,0x00,0x00,0x00,0x00,
};

unsigned char cov_2char[64]={
0x2E,0x2F,0x30,0x31,0x32,0x33,0x34,0x35,
0x36,0x37,0x38,0x39,0x41,0x42,0x43,0x44,
0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,
0x4D,0x4E,0x4F,0x50,0x51,0x52,0x53,0x54,
0x55,0x56,0x57,0x58,0x59,0x5A,0x61,0x62,
0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,
0x6B,0x6C,0x6D,0x6E,0x6F,0x70,0x71,0x72,
0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A
};

char *
des_fcrypt(const char *buf, const char *salt, char *buff)
	{
	unsigned int i,j,x,y;
	unsigned long Eswap0,Eswap1;
	unsigned long out[2],ll;
	des_cblock key;
	des_key_schedule ks;
	unsigned char bb[9];
	unsigned char *b=bb;
	unsigned char c,u;

	/* eay 25/08/92
	 * If you call crypt("pwd","*") as often happens when you
	 * have * as the pwd field in /etc/passwd, the function
	 * returns *\0XXXXXXXXX
	 * The \0 makes the string look like * so the pwd "*" would
	 * crypt to "*".  This was found when replacing the crypt in
	 * our shared libraries.  People found that the disbled
	 * accounts effectivly had no passwd :-(. */
	x=buff[0]=((salt[0] == '\0')?(char)'A':salt[0]);
	Eswap0=con_salt[x];
	x=buff[1]=((salt[1] == '\0')?(char)'A':salt[1]);
	Eswap1=con_salt[x]<<4;

	for (i=0; i<8; i++)
		{
		c= *(buf++);
		if (!c) break;
		key[i]=(char)(c<<1);
		}
	for (; i<8; i++)
		key[i]=0;

	des_set_key((des_cblock *)(key),ks);
	body(&out[0],&out[1],ks,Eswap0,Eswap1);

	ll=out[0]; l2c(ll,b);
	ll=out[1]; l2c(ll,b);
	y=0;
	u=0x80;
	bb[8]=0;
	for (i=2; i<13; i++)
		{
		c=0;
		for (j=0; j<6; j++)
			{
			c<<=1;
			if (bb[y] & u) c|=1;
			u>>=1;
			if (!u)
				{
				y++;
				u=0x80;
				}
			}
		buff[i]=cov_2char[c];
		}
	buff[13]='\0';
	return buff;
	}

static int 
body(	unsigned long *out0,
	unsigned long *out1,
	des_key_schedule ks,
	unsigned long Eswap0,
	unsigned long Eswap1)
	{
	register unsigned long l,r,t,u,v;
#ifdef ALT_ECB
	register unsigned char *des_SP=(unsigned char *)SPtrans;
#endif
	register unsigned long *s;
	register int i,j;
	register unsigned long E0,E1;

	l=0;
	r=0;

	s=(unsigned long *)ks;
	E0=Eswap0;
	E1=Eswap1;

	for (j=0; j<25; j++)
		{
		for (i=0; i<(ITERATIONS*2); i+=4)
			{
			D_ENCRYPT(l,r,  i);	/*  1 */
			D_ENCRYPT(r,l,  i+2);	/*  2 */
			}
		t=l;
		l=r;
		r=t;
		}
	t=r;
	r=(l>>1)|(l<<31);
	l=(t>>1)|(t<<31);
	/* clear the top bits on machines with 8byte longs */
	l&=0xffffffff;
	r&=0xffffffff;

	PERM_OP(r,l,t, 1,0x55555555);
	PERM_OP(l,r,t, 8,0x00ff00ff);
	PERM_OP(r,l,t, 2,0x33333333);
	PERM_OP(l,r,t,16,0x0000ffff);
	PERM_OP(r,l,t, 4,0x0f0f0f0f);

	*out0=l;
	*out1=r;
	return(0);
	}

