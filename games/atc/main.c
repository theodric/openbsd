/*	$OpenBSD: src/games/atc/main.c,v 1.21 2009/10/27 23:59:23 deraadt Exp $	*/
/*	$NetBSD: main.c,v 1.4 1995/04/27 21:22:25 mycroft Exp $	*/

/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Ed James.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Copyright (c) 1987 by Ed James, UC Berkeley.  All rights reserved.
 *
 * Copy permission is hereby granted provided that this notice is
 * retained on all partial or complete copies.
 *
 * For more info on this and all of my stuff, mail edjames@berkeley.edu.
 */

#include "include.h"
#include "pathnames.h"

int
main(int ac, char *av[])
{
	int			f_usage = 0, f_list = 0, f_showscore = 0;
	int			f_printpath = 0;
	const char		*file = NULL;
	char			*name, *ptr, *seed;
	struct sigaction	sa;
	gid_t			gid;
#ifdef BSD
	struct itimerval	itv;
#endif

	open_score_file();

	/* revoke privs */
	gid = getgid();
	setresgid(gid, gid, gid);

	start_time = time(0);
	makenoise = 1;
	seed = NULL;

	name = *av++;
	while (*av) {
#ifndef SAVEDASH
		if (**av == '-') 
			++*av;
		else
			break;
#endif
		ptr = *av++;
		while (*ptr) {
			switch (*ptr) {
			case '?':
			case 'u':
				f_usage++;
				break;
			case 'l':
				f_list++;
				break;
			case 's':
			case 't':
				f_showscore++;
				break;
			case 'p':
				f_printpath++;
				break;
			case 'q':
				makenoise = 0;
				break;
			case 'r':
				seed = *av;
				av++;
				break;
			case 'f':
			case 'g':
				file = *av;
				av++;
				break;
			default: 
				warnx("unknown option '%c'", *ptr);
				f_usage++;
				break;
			}
			ptr++;
		}
	}
	if (seed != NULL)
		srandom(atol(seed));
	else
		srandomdev();

	if (f_usage)
		fprintf(stderr, 
		    "usage: %s [-lpqstu?] [-f game] [-g game] [-r seed]\n",
		    name);
	if (f_showscore)
		log_score(1);
	if (f_list)
		list_games();
	if (f_printpath) {
		size_t	len;
		char	buf[256];

		strlcpy(buf, _PATH_GAMES, sizeof buf);
		len = strlen(buf);
		if (len != 0 && buf[len - 1] == '/')
			buf[len - 1] = '\0';
		puts(buf);
	}
		
	if (f_usage || f_showscore || f_list || f_printpath)
		exit(0);

	if (file == NULL)
		file = default_game();
	else
		file = okay_game(file);

	if (file == NULL || read_file(file) < 0)
		exit(1);

	setup_screen(sp);

	addplane();

	signal(SIGINT, quit);
	signal(SIGQUIT, quit);
#ifdef BSD
	signal(SIGTSTP, SIG_IGN);
	signal(SIGSTOP, SIG_IGN);
#endif
	signal(SIGHUP, log_score_quit);
	signal(SIGTERM, log_score_quit);

	tcgetattr(fileno(stdin), &tty_start);
	tty_new = tty_start;
	tty_new.c_lflag &= ~(ICANON|ECHO);
	tty_new.c_iflag |= ICRNL;
	tty_new.c_cc[VMIN] = 1;
	tty_new.c_cc[VTIME] = 0;
	tcsetattr(fileno(stdin), TCSADRAIN, &tty_new);

	memset(&sa, 0, sizeof sa);
	sa.sa_handler = update;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGALRM);
	sigaddset(&sa.sa_mask, SIGINT);
	sa.sa_flags = 0;
	sigaction(SIGALRM, &sa, (struct sigaction *)0);

#ifdef BSD
	itv.it_value.tv_sec = 0;
	itv.it_value.tv_usec = 1;
	itv.it_interval.tv_sec = sp->update_secs;
	itv.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &itv, NULL);
#endif
#ifdef SYSV
	alarm(sp->update_secs);
#endif

	for (;;) {
		if (getcommand() != 1)
			planewin();
		else {
#ifdef BSD
			itv.it_value.tv_sec = 0;
			itv.it_value.tv_usec = 0;
			setitimer(ITIMER_REAL, &itv, NULL);
#endif
#ifdef SYSV
			alarm(0);
#endif

			update(0);

#ifdef BSD
			itv.it_value.tv_sec = sp->update_secs;
			itv.it_value.tv_usec = 0;
			itv.it_interval.tv_sec = sp->update_secs;
			itv.it_interval.tv_usec = 0;
			setitimer(ITIMER_REAL, &itv, NULL);
#endif
#ifdef SYSV
			alarm(sp->update_secs);
#endif
		}
	}
}

int
read_file(const char *s)
{
	extern FILE	*yyin;
	int		retval;

	file = s;
	yyin = fopen(s, "r");
	if (yyin == NULL) {
		warn("fopen %s", s);
		return (-1);
	}
	retval = yyparse();
	fclose(yyin);

	if (retval != 0)
		return (-1);
	else
		return (0);
}

const char	*
default_game(void)
{
	FILE		*fp;
	static char	file[256];
	char		line[256], games[256];

	strlcpy(games, _PATH_GAMES, sizeof games);
	strlcat(games, GAMES, sizeof games);

	if ((fp = fopen(games, "r")) == NULL) {
		warn("fopen %s", games);
		return (NULL);
	}
	if (fgets(line, sizeof(line), fp) == NULL) {
		warnx("%s: no default game available", games);
		fclose(fp);
		return (NULL);
	}
	fclose(fp);

	line[strcspn(line, "\n")] = '\0';
	if (strlen(line) + strlen(_PATH_GAMES) >= sizeof(file)) {
		warnx("default game name too long");
		return (NULL);
	}
	strlcpy(file, _PATH_GAMES, sizeof file);
	strlcat(file, line, sizeof file);
	return (file);
}

const char	*
okay_game(const char *s)
{
	FILE		*fp;
	static char	file[256];
	const char	*ret = NULL;
	char		line[256], games[256];

	strlcpy(games, _PATH_GAMES, sizeof games);
	strlcat(games, GAMES, sizeof games);

	if ((fp = fopen(games, "r")) == NULL) {
		warn("fopen %s", games);
		return (NULL);
	}
	while (fgets(line, sizeof(line), fp) != NULL) {
		line[strcspn(line, "\n")] = '\0';
		if (strcmp(s, line) == 0) {
			if (strlen(line) + strlen(_PATH_GAMES) >= sizeof(file)) {
				warnx("game name too long");
				return (NULL);
			}
			strlcpy(file, _PATH_GAMES, sizeof file);
			strlcat(file, line, sizeof file);
			ret = file;
			break;
		}
	}
	fclose(fp);
	if (ret == NULL) {
		test_mode = 1;
		ret = s;
		fprintf(stderr, "%s: %s: game not found\n", games, s);
		fprintf(stderr, "Your score will not be logged.\n");
		sleep(2);	/* give the guy time to read it */
	}
	return (ret);
}

int
list_games(void)
{
	FILE		*fp;
	char		line[256], games[256];
	int		num_games = 0;

	strlcpy(games, _PATH_GAMES, sizeof games);
	strlcat(games, GAMES, sizeof games);

	if ((fp = fopen(games, "r")) == NULL) {
		warn("fopen %s", games);
		return (-1);
	}
	puts("available games:");
	while (fgets(line, sizeof(line), fp) != NULL) {
		line[strcspn(line, "\n")] = '\0';
		printf("	%s\n", line);
		num_games++;
	}
	fclose(fp);
	if (num_games == 0) {
		fprintf(stderr, "%s: no games available\n", games);
		return (-1);
	}
	return (0);
}
