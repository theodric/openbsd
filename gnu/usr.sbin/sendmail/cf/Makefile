#	$OpenBSD: src/gnu/usr.sbin/sendmail/cf/Makefile,v 1.1 2000/04/02 19:48:10 millert Exp $
#

NOOBJ=	noobj
FILES=	README
DIRS=	cf domain feature hack m4 mailer ostype sh siteconfig

install:
	@${INSTALL} -d -o ${BINOWN} -g wheel -m 755 \
	    ${DESTDIR}${SHAREDIR}/sendmail
	@echo installing ${FILES}
	@-for i in ${FILES}; do \
		cmp -s $$i ${DESTDIR}${SHAREDIR}/sendmail/$$i || \
		${INSTALL} ${INSTALL_COPY} -o ${BINOWN} -g ${BINGRP} \
		    -m 444  $$i ${DESTDIR}${SHAREDIR}/sendmail/$$i ; \
	done
	@echo installing ${DIRS}
	@-for i in ${DIRS}; do \
		${INSTALL} -d -o ${BINOWN} -g wheel -m 755 \
		    ${DESTDIR}${SHAREDIR}/sendmail/$$i; \
		(cd $$i; for j in *.m[4c] *.sh Makefile; do \
		    if [ -f $$j ]; then \
			cmp -s $$j ${DESTDIR}${SHAREDIR}/sendmail/$$i/$$j || \
			${INSTALL} ${INSTALL_COPY} -o ${BINOWN} -g ${BINGRP} \
			    -m 444 $$j ${DESTDIR}${SHAREDIR}/sendmail/$$i/$$j; \
		    fi; \
		done); \
	done

.include <bsd.prog.mk>
