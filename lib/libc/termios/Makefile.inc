#	$OpenBSD: src/lib/libc/termios/Makefile.inc,v 1.4 2013/12/17 16:31:36 millert Exp $

.PATH:	${LIBCSRCDIR}/termios

SRCS+=	cfgetispeed.c cfgetospeed.c cfmakeraw.c cfsetispeed.c cfsetospeed.c \
	cfsetspeed.c tcdrain.c tcflow.c tcflush.c tcgetattr.c tcgetpgrp.c \
	tcsendbreak.c tcsetattr.c tcsetpgrp.c tcgetsid.c

MAN+=	tcgetpgrp.3 tcsendbreak.3 tcsetattr.3 tcsetpgrp.3 tcgetsid.3

MLINKS+=tcsendbreak.3 tcdrain.3 tcsendbreak.3 tcflow.3 \
	tcsendbreak.3 tcflush.3
MLINKS+=tcsetattr.3 tcgetattr.3 tcsetattr.3 cfgetispeed.3 \
	tcsetattr.3 cfgetospeed.3 tcsetattr.3 cfmakeraw.3 \
	tcsetattr.3 cfsetispeed.3 tcsetattr.3 cfsetospeed.3 \
	tcsetattr.3 cfsetspeed.3
