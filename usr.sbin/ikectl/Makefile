# $OpenBSD: src/usr.sbin/ikectl/Makefile,v 1.3 2014/01/18 05:54:51 martynas Exp $

.PATH:		${.CURDIR}/../../sbin/iked

PROG=		ikectl
SRCS=		log.c ikeca.c ikectl.c parser.c

MAN=		ikectl.8

LDADD=		-lutil -lcrypto
DPADD=		${LIBUTIL}
CFLAGS+=	-Wall -I${.CURDIR} -I${.CURDIR}/../../sbin/iked
CFLAGS+=	-Wstrict-prototypes -Wmissing-prototypes
CFLAGS+=	-Wmissing-declarations
CFLAGS+=	-Wshadow -Wpointer-arith -Wcast-qual
CFLAGS+=	-Wsign-compare

#distribution:
#	${INSTALL} -C -g wheel -m 0644 ${.CURDIR}/ikeca.cnf \
#	    ${DESTDIR}/etc/ssl/ikeca.cnf

.include <bsd.prog.mk>
