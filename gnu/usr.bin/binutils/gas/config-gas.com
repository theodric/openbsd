$!
$! This file sets things up to build gas on a VMS system to generate object
$! files for a VMS system.  We do not use the configure script, since we
$! do not have /bin/sh to execute it.
$!
$!
$gas_host="vms"
$!
$arch_indx = 1 + ((f$getsyi("CPU").ge.128).and.1)      ! vax==1, alpha==2
$arch = f$element(arch_indx,"|","|VAX|Alpha|")
$if arch .eqs. "VAX"
$then
$cpu_type="vax"
$obj_format="vms"
$atof="vax"
$else
$ cpu_type="alpha"
$ obj_format="evax"
$ atof="ieee"
$endif
$
$emulation="generic"
$!
$	DELETE	= "delete/noConfirm"
$	ECHO	= "write sys$output"
$!
$! Target specific information
$call link targ-cpu.c	[.config]tc-'cpu_type'.c
$call link targ-cpu.h	[.config]tc-'cpu_type'.h
$call link targ-env.h	[.config]te-'emulation'.h
$!
$! Code to handle the object file format.
$call link obj-format.h	[.config]obj-'obj_format'.h
$call link obj-format.c	[.config]obj-'obj_format'.c
$!
$! Code to handle floating point.
$call link atof-targ.c	[.config]atof-'atof'.c
$!
$!
$! Create the file version.opt, which helps identify the executable.
$!
$if f$trnlnm("IFILE$").nes."" then  close/noLog ifile$
$search Makefile.in "VERSION="/Exact/Output=config-gas-tmp.tmp
$open ifile$ config-gas-tmp.tmp
$read ifile$ line
$close ifile$
$DELETE config-gas-tmp.tmp;*
$! Discard "VERSION=" and "\n" parts.
$ijk=f$locate("=",line)+1
$line=f$extract(ijk,f$length(line)-ijk,line)
$! [what "\n" part??  this seems to be useless, but is benign]
$ijk=f$locate("\n",line)
$line=f$extract(0,ijk,line)
$!
$ if f$search("version.opt").nes."" then DELETE version.opt;*
$copy _NL: version.opt
$open/Append ifile$ version.opt
$write ifile$ "identification="+""""+line+""""
$close ifile$
$!
$! Now write config.h.
$!
$ if f$search("config.h").nes."" then DELETE config.h;*
$copy _NL: config.h
$open/Append ifile$ config.h
$write ifile$ "/* config.h.  Generated by config-gas.com. */
$write ifile$ "#ifndef GAS_VERSION"
$write ifile$ "#define GAS_VERSION      """,line,""""
$write ifile$ "#endif"
$write ifile$ "/*--*/"
$if arch .eqs. "VAX"
$then
$append [.config]vms-conf.h ifile$:
$else
$ append [.config]vms-a-conf.h ifile$:
$endif
$close ifile$
$ECHO "Created config.h."
$!
$! Check for, and possibly make, header file <unistd.h>.
$!
$ if f$search("tmp-chk-h.*").nes."" then  DELETE tmp-chk-h.*;*
$!can't use simple `#include HDR' with `gcc /Define="HDR=<foo.h>"'
$!because the 2.6.[0-3] preprocessor handles it wrong (VMS-specific gcc bug)
$ create tmp-chk-h.c
int tmp_chk_h;	/* guarantee non-empty output */
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_UNIXIO_H
#include <unixio.h>
#endif
#ifdef HAVE_UNIXLIB_H
#include <unixlib.h>
#endif
$ on warning then  continue
$ CHECK = "call tmp_chk_h"
$ CHECK "HAVE_STDIO_H"
$ if .not.$status
$ then	type sys$input:

? could not compile <stdio.h>.
  Since gcc is not set up correctly, gas configuration cannot proceed.

$	DELETE tmp-chk-h.c;*
$	exit %x002C
$ endif
$!
$ CHECK "HAVE_UNISTD_H"
$ if .not.$status
$ then
$  if f$trnlnm("HFILE$").nes."" then  close/noLog hfile$
$  CHECK "HAVE_UNIXIO_H"
$  got_unixio = ($status .and. 1)
$  CHECK "HAVE_UNIXLIB_H"
$  got_unixlib = ($status .and. 1)
$  create []unistd.h	!with rudimentary contents
/* <unistd.h> substitute for building gas */
#ifndef UNISTD_H
#define UNISTD_H

$  open/Append hfile$ []unistd.h
$  if got_unixio
$  then  write hfile$ "#include <unixio.h>"
$  else  append sys$input: hfile$:
/* some of the routines normally prototyped in <unixio.h> */
extern int creat(), open(), close(), read(), write();
extern int access(), dup(), dup2(), fstat(), stat();
extern long lseek();
$  endif
$  write hfile$ ""
$  if got_unixlib
$  then  write hfile$ "#include <unixlib.h>"
$  else  append sys$input: hfile$:
/* some of the routines normally prototyped in <unixlib.h> */
extern char *sbrk(), *getcwd(), *cuserid();
extern int brk(), chdir(), chmod(), chown(), mkdir();
extern unsigned getuid(), umask();
$  endif
$  append sys$input: hfile$:

#endif /*UNISTD_H*/
$  close hfile$
$  ECHO "Created ""[]unistd.h""."
$ endif !gcc '#include <unistd.h>' failed
$ DELETE tmp-chk-h.c;*
$
$tmp_chk_h: subroutine
$  set noOn
$  hname = f$edit("<" + (p1 - "HAVE_" - "_H") + ".h>","LOWERCASE")
$  write sys$output "Checking for ''hname'."
$  if f$search("tmp-chk-h.obj").nes."" then  DELETE tmp-chk-h.obj;*
$  define/noLog sys$error _NL:	!can't use /User_Mode here due to gcc
$  define/noLog sys$output _NL:	! driver's use of multiple image activation
$  gcc /Include=([],[-.include]) /Define=("''p1'") tmp-chk-h.c
$!can't just check $status; gcc 2.6.[0-3] preprocessor doesn't set it correctly
$  ok = (($status.and.1).and.(f$search("tmp-chk-h.obj").nes."")) .or. %x10000000
$  deassign sys$error	!restore, more or less
$  deassign sys$output
$  if ok then  DELETE tmp-chk-h.obj;*
$  exit ok
$ endsubroutine !tmp_chk_h
$
$!
$! Done
$!
$ if f$search("config.status") .nes. "" then DELETE config.status;*
$ open/write cfile []config.status
$ write cfile "Links are now set up for use with a "+arch+" running VMS."
$ close cfile
$ type []config.status
$exit
$!
$!
$link:
$subroutine
$  if f$search(p1).nes."" then DELETE 'p1';*
$  copy 'p2' 'p1'
$  ECHO "Copied ''f$edit(p2,"LOWERCASE")' to ''f$edit(p1,"LOWERCASE")'."
$endsubroutine
