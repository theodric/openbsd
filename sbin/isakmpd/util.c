/*	$OpenBSD: src/sbin/isakmpd/util.c,v 1.14 2001/06/27 05:16:49 ho Exp $	*/
/*	$EOM: util.c,v 1.23 2000/11/23 12:22:08 niklas Exp $	*/

/*
 * Copyright (c) 1998, 1999, 2001 Niklas Hallqvist.  All rights reserved.
 * Copyright (c) 2000, 2001 H�kan Olsson.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Ericsson Radio Systems.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This code was written under funding by Ericsson Radio Systems.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "sysdep.h"

#include "log.h"
#include "message.h"
#include "sysdep.h"
#include "transport.h"
#include "util.h"

/*
 * Set if -N is given, allowing name lookups to be done, possibly stalling
 * the daemon for quite a while.
 */
int allow_name_lookups = 0;

/*
 * This is set to true in case of regression-test mode, when it will
 * cause predictable random numbers be generated.
 */
int regrand = 0;

/*
 * XXX These might be turned into inlines or macros, maybe even
 * machine-dependent ones, for performance reasons.
 */
u_int16_t
decode_16 (u_int8_t *cp)
{
  return cp[0] << 8 | cp[1];
}

u_int32_t
decode_32 (u_int8_t *cp)
{
  return cp[0] << 24 | cp[1] << 16 | cp[2] << 8 | cp[3];
}

u_int64_t
decode_64 (u_int8_t *cp)
{
  return (u_int64_t)cp[0] << 56 | (u_int64_t)cp[1] << 48
    | (u_int64_t)cp[2] << 40 | (u_int64_t)cp[3] << 32
    | cp[4] << 24 | cp[5] << 16 | cp[6] << 8 | cp[7];
}

#if 0
/*
 * XXX I severly doubt that we will need this.  IPv6 does not have the legacy
 * of representation in host byte order, AFAIK.
 */

void
decode_128 (u_int8_t *cp, u_int8_t *cpp)
{
#if BYTE_ORDER == LITTLE_ENDIAN
  int i;

  for (i = 0; i < 16; i++)
    cpp[i] = cp[15 - i];
#elif BYTE_ORDER == BIG_ENDIAN
  bcopy (cp, cpp, 16);
#else
#error "Byte order unknown!"
#endif
}
#endif

void
encode_16 (u_int8_t *cp, u_int16_t x)
{
  *cp++ = x >> 8;
  *cp = x & 0xff;
}

void
encode_32 (u_int8_t *cp, u_int32_t x)
{
  *cp++ = x >> 24;
  *cp++ = (x >> 16) & 0xff;
  *cp++ = (x >> 8) & 0xff;
  *cp = x & 0xff;
}

void
encode_64 (u_int8_t *cp, u_int64_t x)
{
  *cp++ = x >> 56;
  *cp++ = (x >> 48) & 0xff;
  *cp++ = (x >> 40) & 0xff;
  *cp++ = (x >> 32) & 0xff;
  *cp++ = (x >> 24) & 0xff;
  *cp++ = (x >> 16) & 0xff;
  *cp++ = (x >> 8) & 0xff;
  *cp = x & 0xff;
}

#if 0
/*
 * XXX I severly doubt that we will need this.  IPv6 does not have the legacy
 * of representation in host byte order, AFAIK.
 */

void
encode_128 (u_int8_t *cp, u_int8_t *cpp)
{
  decode_128 (cpp, cp);
}
#endif

/* Check a buffer for all zeroes.  */
int
zero_test (const u_int8_t *p, size_t sz)
{
  while (sz-- > 0)
    if (*p++ != 0)
      return 0;
  return 1;
}

/* Check a buffer for all ones.  */
int
ones_test (const u_int8_t *p, size_t sz)
{
  while (sz-- > 0)
    if (*p++ != 0xff)
      return 0;
  return 1;
}

/*
 * Generate a random data, len bytes long.
 */
u_int8_t *
getrandom (u_int8_t *buf, size_t len)
{
  u_int32_t tmp = 0;
  int i;

  for (i = 0; i < len; i++)
    {
      if (i % sizeof tmp == 0)
	tmp = sysdep_random ();

      buf[i] = tmp & 0xff;
      tmp >>= 8;
    }

  return buf;
}

static __inline int
hex2nibble (char c)
{
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  return -1;
}

/*
 * Convert hexadecimal string in S to raw binary buffer at BUF sized SZ
 * bytes.  Return 0 if everything is OK, -1 otherwise.
 */
int
hex2raw (char *s, u_int8_t *buf, size_t sz)
{
  char *p;
  u_int8_t *bp;
  int tmp;

  if (strlen (s) > sz * 2)
    return -1;
  for (p = s + strlen (s) - 1, bp = &buf[sz - 1]; bp >= buf; bp--)
    {
      *bp = 0;
      if (p >= s)
	{
	  tmp = hex2nibble (*p--);
	  if (tmp == -1)
	    return -1;
	  *bp = tmp;
	}
      if (p >= s)
	{
	  tmp = hex2nibble (*p--);
	  if (tmp == -1)
	    return -1;
	  *bp |= tmp << 4;
	}
    }
  return 0;
}

int
text2sockaddr (char *address, char *port, struct sockaddr **sa)
{
#ifdef HAVE_GETNAMEINFO
  struct addrinfo *ai, hints;

  memset (&hints, 0, sizeof hints);
  if (!allow_name_lookups)
    hints.ai_flags = AI_NUMERICHOST;
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;

  if (getaddrinfo (address, port, &hints, &ai))
    return -1;

  *sa = malloc (ai->ai_addr->sa_len);
  if (!sa)
    return -1;

  memcpy (*sa, ai->ai_addr, ai->ai_addr->sa_len);
  freeaddrinfo (ai);
  return 0;
#else
  return -1;
#endif
}

int
sockaddr2text (struct sockaddr *sa, char **address)
{
  char buf[NI_MAXHOST];

#ifdef HAVE_GETNAMEINFO
  if (getnameinfo (sa, sa->sa_len, buf, sizeof buf, 0, 0,
		   allow_name_lookups ? 0 : NI_NUMERICHOST))
    return -1;
#else
  if (sa->sa_family == AF_INET)
    {
      strncpy (buf, inet_ntoa (((struct sockaddr_in *)sa)->sin_addr), 
	       NI_MAXHOST - 1);
      buf[NI_MAXHOST - 1] = '\0';
    }
  else
    {
      log_print ("sockaddr2text: unsupported protocol family %d\n",
		  sa->sa_family);
      strcpy (buf, "<error>");
    }
#endif

  *address = malloc (strlen (buf) + 1);
  if (!address)
    return -1;
  
  strcpy (*address, buf);
  return 0;
}

/*
 * sockaddr_len and sockaddr_data return the relevant sockaddr info depending
 * on address family. Useful to keep other code shorter(/clearer?).
 */
int
sockaddr_len (struct sockaddr *sa)
{
  switch (sa->sa_family)
    {
    case AF_INET6:
      return sizeof ((struct sockaddr_in6 *)sa)->sin6_addr.s6_addr;
    case AF_INET:
      return sizeof ((struct sockaddr_in *)sa)->sin_addr.s_addr;
    default: 
      log_print ("sockaddr_len: unsupported protocol family %d", 
		 sa->sa_family);
      return 0;
    }
}

u_int8_t *
sockaddr_data (struct sockaddr *sa)
{
  switch (sa->sa_family)
    {
    case AF_INET6:
      return (u_int8_t *)&((struct sockaddr_in6 *)sa)->sin6_addr.s6_addr;
    case AF_INET:
      return (u_int8_t *)&((struct sockaddr_in *)sa)->sin_addr.s_addr;
    default:
      return 0; /* XXX */
    }
}
     
/*
 * Perform sanity check on files containing secret information.
 * Returns -1 on failure, 0 otherwise.
 * Also, if *file_size != NULL, store file size here.
 */
int
check_file_secrecy (char *name, off_t *file_size)
{
  struct stat st;
  
  if (stat (name, &st) == -1)
    {
      log_error ("check_file_secrecy: stat (\"%s\") failed", name);
      return -1;
    }
  if (st.st_uid != geteuid () && st.st_uid != getuid ())
    {
      log_print ("check_file_secrecy: "
		 "not loading %s - file owner is not process user", name);
      errno = EPERM;
      return -1;
    }
  if ((st.st_mode & (S_IRWXG | S_IRWXO)) != 0)
    {
      log_print ("conf_file_secrecy: not loading %s - too open permissions",
		 name);
      errno = EPERM;
      return -1;
    }
  
  if (file_size)
    *file_size = st.st_size;

  return 0;
}

