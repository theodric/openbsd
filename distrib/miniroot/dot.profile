#	$OpenBSD: src/distrib/miniroot/dot.profile,v 1.10 2009/06/04 06:23:53 krw Exp $
#	$NetBSD: dot.profile,v 1.1 1995/12/18 22:54:43 pk Exp $
#
# Copyright (c) 2009 Kenneth R. Westerback
# Copyright (c) 1995 Jason R. Thorpe
# Copyright (c) 1994 Christopher G. Demetriou
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. All advertising materials mentioning features or use of this software
#    must display the following acknowledgement:
#	This product includes software developed by Christopher G. Demetriou.
# 4. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

export VNAME=$(sysctl -n kern.osrelease)
export VERSION="${VNAME%.*}${VNAME#*.}"
export ARCH=$(sysctl -n hw.machine)
export OBSD="OpenBSD/$ARCH $VNAME"

export PATH=/sbin:/bin:/usr/bin:/usr/sbin:/
umask 022
# emacs-style command line editing
set -o emacs

# Extract rootdisk from last 'root on ...' line. e.g.
# 	root on wd0a swap on wd0b dump on wd0b
set -- $(dmesg | sed -n '/^root on /h;${g;p;}')
rootdisk=$3

mount -u /dev/${rootdisk:-rd0a} /

# set up some sane defaults
echo 'erase ^?, werase ^W, kill ^U, intr ^C, status ^T'
stty newcrt werase ^W intr ^C kill ^U erase ^? status ^T

# Installing or upgrading?
cat <<__EOT

Welcome to the $OBSD installation program.

This program can install $OBSD, or upgrade an existing
OpenBSD installation.

At any prompt except password prompts you can escape to a shell by
typing '!'. Default answers are shown in []'s and are selected by
pressing RETURN.  At any time you can exit this program by pressing
Control-C, but exiting during an $MODE can leave your system in an
inconsistent state.

__EOT
while :; do
	read REPLY?'(I)nstall, (U)pgrade or (S)hell? '
	case $REPLY in
	i*|I*)	echo "\nCool! Let's get to it."
		/install && break
		;;
	u*|U*)	cat <<__EOT

NOTE: Once your system has been upgraded, you must manually merge any
changes to files in the 'etc' set into the files already on your system.
sysmerge(8) can help.

__EOT
		/upgrade && break
		;;
	s*|S*)	break
		;;
	esac
done
