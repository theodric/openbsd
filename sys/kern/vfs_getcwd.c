/* $OpenBSD: src/sys/kern/vfs_getcwd.c,v 1.3 2006/04/30 00:08:13 pedro Exp $ */
/* $NetBSD: vfs_getcwd.c,v 1.3.2.3 1999/07/11 10:24:09 sommerfeld Exp $ */

/*
 * Copyright (c) 1999 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Bill Sommerfeld.
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
 *        This product includes software developed by the NetBSD
 *        Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/namei.h>
#include <sys/filedesc.h>
#include <sys/kernel.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <sys/proc.h>
#include <sys/uio.h>
#include <sys/malloc.h>
#include <sys/dirent.h>
#include <ufs/ufs/dir.h>	/* XXX only for DIRBLKSIZ */

#include <sys/syscallargs.h>

static int getcwd_scandir(struct vnode **, struct vnode **,
    char **, char *, struct proc *);
static int getcwd_getcache(struct vnode **, struct vnode **, char **, char *);
static int getcwd_common(struct vnode *, struct vnode *,
    char **, char *, int, int, struct proc *);

static int vn_isunder(struct vnode *, struct vnode *, struct proc *);

#define DIRENT_MINSIZE (sizeof(struct dirent) - (MAXNAMLEN+1) + 4)

/*
 * Vnode variable naming conventions in this file:
 *
 * rvp: the current root we're aiming towards.
 * lvp, *lvpp: the "lower" vnode
 * uvp, *uvpp: the "upper" vnode.
 *
 * Since all the vnodes we're dealing with are directories, and the
 * lookups are going *up* in the filesystem rather than *down*, the
 * usual "pvp" (parent) or "dvp" (directory) naming conventions are
 * too confusing.
 */

/*
 * XXX: is EINVAL the right thing to return if a directory is
 *      malformed?
 */


/*
 * Find parent vnode of *lvpp, return in *uvpp
 *
 * If we care about the name, scan it looking for name of directory
 * entry pointing at lvp.
 *
 * Place the name in the buffer which starts at bufp, immediately
 * before *bpp, and move bpp backwards to point at the start of it.
 *
 * On entry, *lvpp is a locked vnode reference; on exit, it is vput and NULL'ed
 * On exit, *uvpp is either NULL or is a locked vnode reference.
 */
static int
getcwd_scandir(struct vnode **lvpp, struct vnode **uvpp,
    char **bpp, char *bufp, struct proc *p)
{
	int     error = 0;
	int     eofflag;
	off_t   off;
	int     tries;
	struct uio uio;
	struct iovec iov;
	char   *dirbuf = NULL;
	int	dirbuflen;
	ino_t   fileno;
	struct vattr va;
	struct vnode *uvp = NULL;
	struct vnode *lvp = *lvpp;	
	struct componentname cn;
	int len, reclen;
	tries = 0;

	/*
	 * If we want the filename, get some info we need while the
	 * current directory is still locked.
	 */
	if (bufp != NULL) {
		error = VOP_GETATTR(lvp, &va, p->p_ucred, p);
		if (error) {
			vput(lvp);
			*lvpp = NULL;
			*uvpp = NULL;
			return (error);
		}
	}

	/*
	 * Ok, we have to do it the hard way..
	 * Next, get parent vnode using lookup of ..
	 */
	cn.cn_nameiop = LOOKUP;
	cn.cn_flags = ISLASTCN | ISDOTDOT | RDONLY;
	cn.cn_proc = p;
	cn.cn_cred = p->p_ucred;
	cn.cn_pnbuf = NULL;
	cn.cn_nameptr = "..";
	cn.cn_namelen = 2;
	cn.cn_hash = 0;
	cn.cn_consume = 0;
	
	/*
	 * At this point, lvp is locked and will be unlocked by the lookup.
	 * On successful return, *uvpp will be locked
	 */
	error = VOP_LOOKUP(lvp, uvpp, &cn);
	if (error) {
		vput(lvp);
		*lvpp = NULL;
		*uvpp = NULL;
		return (error);
	}
	uvp = *uvpp;

	/* If we don't care about the pathname, we're done */
	if (bufp == NULL) {
		vrele(lvp);
		*lvpp = NULL;
		return (0);
	}
	
	fileno = va.va_fileid;

	dirbuflen = DIRBLKSIZ;
	if (dirbuflen < va.va_blocksize)
		dirbuflen = va.va_blocksize;
	dirbuf = (char *)malloc(dirbuflen, M_TEMP, M_WAITOK);

	off = 0;
	do {
		char   *cpos;
		struct dirent *dp;

		/* call VOP_READDIR of parent */
		iov.iov_base = dirbuf;
		iov.iov_len = dirbuflen;

		uio.uio_iov = &iov;
		uio.uio_iovcnt = 1;
		uio.uio_offset = off;
		uio.uio_resid = dirbuflen;
		uio.uio_segflg = UIO_SYSSPACE;
		uio.uio_rw = UIO_READ;
		uio.uio_procp = p;

		eofflag = 0;

		error = VOP_READDIR(uvp, &uio, p->p_ucred, &eofflag, 0, 0);

		off = uio.uio_offset;

		/*
		 * Try again if NFS tosses its cookies.
		 */
		if (error == EINVAL && tries < 3) {
			tries++;
			off = 0;
			continue;
		} else if (error != 0) {
			/*
			 * We simply bail on error, even if we don't
			 * have eofflag yet.  This is the exact
			 * behavior of the old userland getcwd()
			 */
			goto out;
		}

		/* No error */
		cpos = dirbuf;
		tries = 0;

		/* scan directory page looking for matching vnode */ 
		for (len = (dirbuflen - uio.uio_resid); len > 0;
		     len -= reclen) {
			dp = (struct dirent *)cpos;
			reclen = dp->d_reclen;

			/* check for malformed directory.. */
			if (reclen < DIRENT_MINSIZE) {
				error = EINVAL;
				goto out;
			}
			/*
			 * XXX should perhaps do VOP_LOOKUP to
			 * check that we got back to the right place,
			 * but getting the locking games for that
			 * right would be heinous.
			 */
			if (dp->d_fileno == fileno) {
				char *bp = *bpp;
				bp -= dp->d_namlen;

				if (bp <= bufp) {
					error = ERANGE;
					goto out;
				}
				bcopy(dp->d_name, bp, dp->d_namlen);
				error = 0;
				*bpp = bp;
				goto out;
			}
			cpos += reclen;
		}

	} while (!eofflag);

	error = ENOENT;

out:
	vrele(lvp);
	*lvpp = NULL;
	free(dirbuf, M_TEMP);
	return (error);
}

/*
 * Look in the vnode-to-name reverse cache to see if
 * we can find things the easy way.
 *
 * XXX vget failure path is untested.
 *
 * On entry, *lvpp is a locked vnode reference.
 * On exit, one of the following is the case:
 *	0) Both *lvpp and *uvpp are NULL and failure is returned.
 * 	1) *uvpp is NULL, *lvpp remains locked and -1 is returned (cache miss)
 *      2) *uvpp is a locked vnode reference, *lvpp is vput and NULL'ed
 *	   and 0 is returned (cache hit)
 */

static int
getcwd_getcache(struct vnode **lvpp, struct vnode **uvpp,
    char **bpp, char *bufp)
{
	struct vnode *lvp, *uvp = NULL;
	int error;
	int vpid;
	struct proc *p = curproc;
	
	lvp = *lvpp;

	/*
	 * This returns 0 on a cache hit, -1 on a clean cache miss, or
	 * an errno on other failure.
	 */
	error = cache_revlookup(lvp, uvpp, bpp, bufp);
	if (error) {
		if (error != -1) {
			vput(lvp);
			*lvpp = NULL;
			*uvpp = NULL;
		}
		return (error);
	}
	uvp = *uvpp;
	vpid = uvp->v_id;

	/*
	 * Since we're going up, we have to release the current lock
	 * before we take the parent lock.
	 */

	VOP_UNLOCK(lvp, 0, p);

	error = vget(uvp, LK_EXCLUSIVE | LK_RETRY, p);
	if (error != 0)
		*uvpp = NULL;
	/*
	 * Verify that vget succeeded, and check that vnode capability
	 * didn't change while we were waiting for the lock.
	 *
	 * XXX: this is kind of nasty to have to check here.  It
	 * should really be done in cache_revlookup() (see comments
	 * there, too).
	 */
	if (error || (vpid != uvp->v_id)) {
		/*
		 * Oops, we missed.  If the vget failed, or the
		 * capability changed, try to get our lock back; if
		 * that works, tell caller to try things the hard way,
		 * otherwise give up.
		 */
		if (!error)
			vput(uvp);
		*uvpp = NULL;
		
		error = vn_lock(lvp, LK_EXCLUSIVE | LK_RETRY, p);

		if (!error)
			return (-1);
	}
	vrele(lvp);
	*lvpp = NULL;
	return (error);
}

/*
 * common routine shared by sys___getcwd() and vn_isunder()
 */

#define GETCWD_CHECK_ACCESS 0x0001

static int
getcwd_common(struct vnode *lvp, struct vnode *rvp,
    char **bpp, char *bufp, int limit, int flags, struct proc *p)
{
	struct filedesc *fdp = p->p_fd;
	struct vnode *uvp = NULL;
	char *bp = NULL;
	int error;
	int perms = VEXEC;

	if (rvp == NULL) {
		rvp = fdp->fd_rdir;
		if (rvp == NULL)
			rvp = rootvnode;
	}

	VREF(rvp);
	VREF(lvp);

	/*
	 * Error handling invariant:
	 * Before a `goto out':
	 *	lvp is either NULL, or locked and held.
	 *	uvp is either NULL, or locked and held.
	 */

	error = vn_lock(lvp, LK_EXCLUSIVE | LK_RETRY, p);
	if (error) {
		vrele(lvp);
		lvp = NULL;
		goto out;
	}
	if (bufp)
		bp = *bpp;
	/*
	 * this loop will terminate when one of the following happens:
	 *	- we hit the root
	 *	- getdirentries or lookup fails
	 *	- we run out of space in the buffer.
	 */
	if (lvp == rvp) {
		if (bp)
			*(--bp) = '/';
		goto out;
	}
	do {
		if (lvp->v_type != VDIR) {
			error = ENOTDIR;
			goto out;
		}
		
		/*
		 * access check here is optional, depending on
		 * whether or not caller cares.
		 */
		if (flags & GETCWD_CHECK_ACCESS) {
			error = VOP_ACCESS(lvp, perms, p->p_ucred, p);
			if (error)
				goto out;
			perms = VEXEC|VREAD;
		}
		
		/*
		 * step up if we're a covered vnode..
		 */
		while (lvp->v_flag & VROOT) {
			struct vnode *tvp;

			if (lvp == rvp)
				goto out;
			
			tvp = lvp;
			lvp = lvp->v_mount->mnt_vnodecovered;
			vput(tvp);
			/*
			 * hodie natus est radici frater
			 */
			if (lvp == NULL) {
				error = ENOENT;
				goto out;
			}
			VREF(lvp);
			error = vn_lock(lvp, LK_EXCLUSIVE | LK_RETRY, p);
			if (error != 0) {
				vrele(lvp);
				lvp = NULL;
				goto out;
			}
		}
		/*
		 * Look in the name cache; if that fails, look in the
		 * directory..
		 */
		error = getcwd_getcache(&lvp, &uvp, &bp, bufp);
		if (error == -1)
			error = getcwd_scandir(&lvp, &uvp, &bp, bufp, p);
		if (error)
			goto out;
#ifdef DIAGNOSTIC		
		if (lvp != NULL)
			panic("getcwd: oops, forgot to null lvp");
		if (bufp && (bp <= bufp)) {
			panic("getcwd: oops, went back too far");
		}
#endif		
		if (bp)
			*(--bp) = '/';
		lvp = uvp;
		uvp = NULL;
		limit--;
	} while ((lvp != rvp) && (limit > 0)); 

out:
	if (bpp)
		*bpp = bp;
	if (uvp)
		vput(uvp);
	if (lvp)
		vput(lvp);
	vrele(rvp);
	return (error);
}

/*
 * Check if one directory can be found inside another in the directory
 * hierarchy.
 *
 * Intended to be used in chroot, chdir, fchdir, etc., to ensure that
 * chroot() actually means something.
 */
static int
vn_isunder(struct vnode *lvp, struct vnode *rvp, struct proc *p)
{
	int error;

	error = getcwd_common(lvp, rvp, NULL, NULL, MAXPATHLEN/2, 0, p);

	if (!error)
		return (1);
	else
		return (0);
}

/* True if p1's root directory is equal to or under p2's root directory */
int
proc_isunder(struct proc *p1, struct proc *p2)
{
	struct vnode *r1 = p1->p_fd->fd_rdir;
	struct vnode *r2 = p2->p_fd->fd_rdir;

	if (r1 == NULL)
		return (r2 == NULL);

	if (r2 == NULL)
		return (1);

	return (vn_isunder(r1, r2, p2));
}

/* Find pathname of a process's current directory */
int
sys___getcwd(struct proc *p, void *v, register_t *retval) 
{
	struct sys___getcwd_args *uap = v;
	int error, lenused, len = SCARG(uap, len);
	char *path, *bp, *bend;

	if (len > MAXPATHLEN * 4)
		len = MAXPATHLEN * 4;
	else if (len < 2)
		return (ERANGE);

	path = (char *) malloc(len, M_TEMP, M_WAITOK);

	bp = &path[len];
	bend = bp;
	*(--bp) = '\0';

	/*
	 * 5th argument here is "max number of vnodes to traverse".
	 * Since each entry takes up at least 2 bytes in the output
	 * buffer, limit it to N/2 vnodes for an N byte buffer.
	 */
	error = getcwd_common(p->p_fd->fd_cdir, NULL, &bp, path, len/2,
	    GETCWD_CHECK_ACCESS, p);

	if (error)
		goto out;

	lenused = bend - bp;
	*retval = lenused;

	/* Put the result into user buffer */
	error = copyout(bp, SCARG(uap, buf), lenused);

out:
	free(path, M_TEMP);

	return (error);
}
