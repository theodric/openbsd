/* $OpenBSD: src/usr.bin/tmux/cmd-list-sessions.c,v 1.5 2009/11/03 20:29:47 nicm Exp $ */

/*
 * Copyright (c) 2007 Nicholas Marriott <nicm@users.sourceforge.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>

#include <string.h>
#include <time.h>

#include "tmux.h"

/*
 * List all sessions.
 */

int	cmd_list_sessions_exec(struct cmd *, struct cmd_ctx *);

const struct cmd_entry cmd_list_sessions_entry = {
	"list-sessions", "ls", "",
	0, 0,
	NULL,
	NULL,
	cmd_list_sessions_exec,
	NULL,
	NULL
};

int
cmd_list_sessions_exec(unused struct cmd *self, struct cmd_ctx *ctx)
{
	struct session		*s;
	struct session_group	*sg;
	char			*tim, tmp[64];
	u_int			 i, idx;
	time_t			 t;

	for (i = 0; i < ARRAY_LENGTH(&sessions); i++) {
		s = ARRAY_ITEM(&sessions, i);
		if (s == NULL)
			continue;

		sg = session_group_find(s);
		if (sg == NULL)
			*tmp = '\0';
		else {
			idx = session_group_index(sg);
			xsnprintf(tmp, sizeof tmp, " (group %u)", idx);
		}

		t = s->creation_time.tv_sec;
		tim = ctime(&t);
		*strchr(tim, '\n') = '\0';

		ctx->print(ctx, "%s: %u windows (created %s) [%ux%u]%s%s",
		    s->name, winlink_count(&s->windows), tim, s->sx, s->sy,
		    tmp, s->flags & SESSION_UNATTACHED ? "" : " (attached)");
	}

	return (0);
}
