/*
 * Copyright (c) 1995, 1996, 1997, 1998, 1999 Kungliga Tekniska H�gskolan
 * (Royal Institute of Technology, Stockholm, Sweden).
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* $Id: xfs_common.h,v 1.1.1.1 2002/06/05 17:24:11 hin Exp $ */

#ifndef _xfs_common_h
#define _xfs_common_h

#if defined(MALLOC_DECLARE)
MALLOC_DECLARE(M_XFS);
#elif !defined(M_XFS)
#define M_XFS M_TEMP
#endif

#ifdef XFS_DEBUG
void *xfs_alloc(u_int size);
void xfs_free(void *, u_int size);
#else
#ifdef __osf__
#define xfs_alloc(a) malloc((a), BUCKETINDEX(a), M_XFS, M_WAITOK)
#else
#define xfs_alloc(a) malloc((a), M_XFS, M_WAITOK)
#endif
#define xfs_free(a, size) free(a, M_XFS)
#endif /* XFS_DEBUG */

int xfs_suser(struct proc *p);

#ifndef HAVE_KERNEL_MEMCPY
void *
memcpy (void *s1, const void *s2, size_t n);
#endif

const char *
xfs_devtoname_r (dev_t dev, char *buf, size_t sz);

#ifndef HAVE_KERNEL_STRLCPY
size_t
strlcpy (char *dst, const char *src, size_t dst_sz);
#endif

#endif /* _xfs_common_h */
