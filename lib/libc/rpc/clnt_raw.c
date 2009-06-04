/*	$OpenBSD: src/lib/libc/rpc/clnt_raw.c,v 1.16 2009/06/04 03:51:29 schwarze Exp $ */
/*
 * Sun RPC is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify Sun RPC without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user.
 * 
 * SUN RPC IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 * 
 * Sun RPC is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 *
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY SUN RPC
 * OR ANY PART THEREOF.
 * 
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 * 
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */

/*
 * clnt_raw.c
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 *
 * Memory based rpc for simple testing and timing.
 * Interface to create an rpc client and server in the same process.
 * This lets us similate rpc and get round trip overhead, without
 * any interference from the kernel.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <rpc/rpc.h>

#define MCALL_MSG_SIZE 24

/*
 * This is the "network" we will be moving stuff over.
 */
static struct clntraw_private {
	CLIENT	client_object;
	XDR	xdr_stream;
	char	_raw_buf[UDPMSGSIZE];
	char	mashl_callmsg[MCALL_MSG_SIZE];
	u_int	mcnt;
} *clntraw_private;

static enum clnt_stat	clntraw_call(CLIENT *, u_long, xdrproc_t, caddr_t,
			    xdrproc_t, caddr_t, struct timeval);
static void		clntraw_abort(CLIENT *);
static void		clntraw_geterr(CLIENT *, struct rpc_err *);
static bool_t		clntraw_freeres(CLIENT *, xdrproc_t, caddr_t);
static bool_t		clntraw_control(CLIENT *, u_int, void *);
static void		clntraw_destroy(CLIENT *);

static struct clnt_ops client_ops = {
	clntraw_call,
	clntraw_abort,
	clntraw_geterr,
	clntraw_freeres,
	clntraw_destroy,
	clntraw_control
};

void	svc_getreq(int rdfds);

/*
 * Create a client handle for memory based rpc.
 */
CLIENT *
clntraw_create(u_long prog, u_long vers)
{
	struct clntraw_private *clp = clntraw_private;
	struct rpc_msg call_msg;
	XDR *xdrs;
	CLIENT	*client;

	if (clp == NULL) {
		clp = (struct clntraw_private *)calloc(1, sizeof (*clp));
		if (clp == NULL)
			goto fail;
		clntraw_private = clp;
	}
	xdrs = &clp->xdr_stream;
	client = &clp->client_object;
	/*
	 * pre-serialize the static part of the call msg and stash it away
	 */
	call_msg.rm_direction = CALL;
	call_msg.rm_call.cb_rpcvers = RPC_MSG_VERSION;
	call_msg.rm_call.cb_prog = prog;
	call_msg.rm_call.cb_vers = vers;
	xdrmem_create(xdrs, clp->mashl_callmsg, MCALL_MSG_SIZE, XDR_ENCODE); 
	if (!xdr_callhdr(xdrs, &call_msg))
		goto fail;
	clp->mcnt = XDR_GETPOS(xdrs);
	XDR_DESTROY(xdrs);

	/*
	 * Set xdrmem for client/server shared buffer
	 */
	xdrmem_create(xdrs, clp->_raw_buf, UDPMSGSIZE, XDR_FREE);

	/*
	 * create client handle
	 */
	client->cl_ops = &client_ops;
	client->cl_auth = authnone_create();
	if (client->cl_auth == NULL)
		goto fail;
	return (client);

fail:
	mem_free((caddr_t)clntraw_private, sizeof(clntraw_private));
	clntraw_private = NULL;
	rpc_createerr.cf_stat = RPC_SYSTEMERROR;
	rpc_createerr.cf_error.re_errno = errno;
	return (NULL);
}

/* ARGSUSED */
static enum clnt_stat 
clntraw_call(CLIENT *h, u_long proc, xdrproc_t xargs, caddr_t argsp,
    xdrproc_t xresults, caddr_t resultsp, struct timeval timeout)
{
	struct clntraw_private *clp = clntraw_private;
	XDR *xdrs;
	struct rpc_msg msg;
	enum clnt_stat status;
	struct rpc_err error;

	if (clp == NULL)
		return (RPC_FAILED);
	xdrs = &clp->xdr_stream;
call_again:
	/*
	 * send request
	 */
	xdrs->x_op = XDR_ENCODE;
	XDR_SETPOS(xdrs, 0);
	((struct rpc_msg *)clp->mashl_callmsg)->rm_xid ++ ;
	if ((! XDR_PUTBYTES(xdrs, clp->mashl_callmsg, clp->mcnt)) ||
	    (! XDR_PUTLONG(xdrs, (long *)&proc)) ||
	    (! AUTH_MARSHALL(h->cl_auth, xdrs)) ||
	    (! (*xargs)(xdrs, argsp))) {
		return (RPC_CANTENCODEARGS);
	}
	(void)XDR_GETPOS(xdrs);  /* called just to cause overhead */

	/*
	 * We have to call server input routine here because this is
	 * all going on in one process. Yuk.
	 */
	svc_getreq(1);

	/*
	 * get results
	 */
	xdrs->x_op = XDR_DECODE;
	XDR_SETPOS(xdrs, 0);
	msg.acpted_rply.ar_verf = _null_auth;
	msg.acpted_rply.ar_results.where = resultsp;
	msg.acpted_rply.ar_results.proc = xresults;
	if (! xdr_replymsg(xdrs, &msg)) {
		/* xdr_replymsg() may have left some things allocated */
		int op = xdrs->x_op;
		xdrs->x_op = XDR_FREE;
		xdr_replymsg(xdrs, &msg);
		xdrs->x_op = op;
		return (RPC_CANTDECODERES);
	}
	_seterr_reply(&msg, &error);
	status = error.re_status;

	if (status == RPC_SUCCESS) {
		if (! AUTH_VALIDATE(h->cl_auth, &msg.acpted_rply.ar_verf)) {
			status = RPC_AUTHERROR;
		}
	}  /* end successful completion */
	else {
		if (AUTH_REFRESH(h->cl_auth))
			goto call_again;
	}  /* end of unsuccessful completion */

	if (status == RPC_SUCCESS) {
		if (! AUTH_VALIDATE(h->cl_auth, &msg.acpted_rply.ar_verf)) {
			status = RPC_AUTHERROR;
		}
		if (msg.acpted_rply.ar_verf.oa_base != NULL) {
			xdrs->x_op = XDR_FREE;
			(void)xdr_opaque_auth(xdrs, &(msg.acpted_rply.ar_verf));
		}
	}

	return (status);
}

/*ARGSUSED*/
static void
clntraw_geterr(CLIENT *clnt, struct rpc_err *err)
{
}

/* ARGSUSED */
static bool_t
clntraw_freeres(CLIENT *cl, xdrproc_t xdr_res, caddr_t res_ptr)
{
	struct clntraw_private *clp = clntraw_private;
	XDR *xdrs;
	bool_t rval;

	if (clp == NULL) {
		rval = (bool_t) RPC_FAILED;
		return (rval);
	}
	xdrs = &clp->xdr_stream;
	xdrs->x_op = XDR_FREE;
	return ((*xdr_res)(xdrs, res_ptr));
}

/*ARGSUSED*/
static void
clntraw_abort(CLIENT *clnt)
{
}

/*ARGSUSED*/
static bool_t
clntraw_control(CLIENT *clnt, u_int i, void *v)
{
	return (FALSE);
}

/*ARGSUSED*/
static void
clntraw_destroy(CLIENT *clnt)
{
}
