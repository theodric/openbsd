#include "includes.h"
RCSID("$OpenBSD: src/usr.bin/ssh/Attic/aux.c,v 1.1 2000/05/17 08:20:15 markus Exp $");

char *
chop(char *s)
{
	char *t = s;
	while (*t) {
		if(*t == '\n' || *t == '\r') {
			*t = '\0';
			return s;
		}
		t++;
	}
	return s;

}

void
set_nonblock(int fd)
{
	int val;
	val = fcntl(fd, F_GETFL, 0);
	if (val < 0) {
		error("fcntl(%d, F_GETFL, 0): %s", fd, strerror(errno));
		return;
	}
	if (val & O_NONBLOCK)
		return;
	debug("fd %d setting O_NONBLOCK", fd);
	val |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, val) == -1)
		error("fcntl(%d, F_SETFL, O_NONBLOCK): %s", fd, strerror(errno));
}
