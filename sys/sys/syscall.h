/*	$OpenBSD: src/sys/sys/syscall.h,v 1.67 2003/06/23 04:27:55 deraadt Exp $	*/

/*
 * System call numbers.
 *
 * DO NOT EDIT-- this file is automatically generated.
 * created from;	OpenBSD: syscalls.master,v 1.60 2003/06/23 04:26:53 deraadt Exp 
 */

/* syscall: "syscall" ret: "int" args: "int" "..." */
#define	SYS_syscall	0

/* syscall: "exit" ret: "void" args: "int" */
#define	SYS_exit	1

/* syscall: "fork" ret: "int" args: */
#define	SYS_fork	2

/* syscall: "read" ret: "ssize_t" args: "int" "void *" "size_t" */
#define	SYS_read	3

/* syscall: "write" ret: "ssize_t" args: "int" "const void *" "size_t" */
#define	SYS_write	4

/* syscall: "open" ret: "int" args: "const char *" "int" "..." */
#define	SYS_open	5

/* syscall: "close" ret: "int" args: "int" */
#define	SYS_close	6

/* syscall: "wait4" ret: "int" args: "int" "int *" "int" "struct rusage *" */
#define	SYS_wait4	7

				/* 8 is compat_43 ocreat */

/* syscall: "link" ret: "int" args: "const char *" "const char *" */
#define	SYS_link	9

/* syscall: "unlink" ret: "int" args: "const char *" */
#define	SYS_unlink	10

				/* 11 is obsolete execv */
/* syscall: "chdir" ret: "int" args: "const char *" */
#define	SYS_chdir	12

/* syscall: "fchdir" ret: "int" args: "int" */
#define	SYS_fchdir	13

/* syscall: "mknod" ret: "int" args: "const char *" "int" "dev_t" */
#define	SYS_mknod	14

/* syscall: "chmod" ret: "int" args: "const char *" "int" */
#define	SYS_chmod	15

/* syscall: "chown" ret: "int" args: "const char *" "uid_t" "gid_t" */
#define	SYS_chown	16

/* syscall: "break" ret: "int" args: "char *" */
#define	SYS_break	17

				/* 18 is compat_25 ogetfsstat */

				/* 19 is compat_43 olseek */

/* syscall: "getpid" ret: "pid_t" args: */
#define	SYS_getpid	20

/* syscall: "mount" ret: "int" args: "const char *" "const char *" "int" "void *" */
#define	SYS_mount	21

/* syscall: "unmount" ret: "int" args: "const char *" "int" */
#define	SYS_unmount	22

/* syscall: "setuid" ret: "int" args: "uid_t" */
#define	SYS_setuid	23

/* syscall: "getuid" ret: "uid_t" args: */
#define	SYS_getuid	24

/* syscall: "geteuid" ret: "uid_t" args: */
#define	SYS_geteuid	25

/* syscall: "ptrace" ret: "int" args: "int" "pid_t" "caddr_t" "int" */
#define	SYS_ptrace	26

/* syscall: "recvmsg" ret: "ssize_t" args: "int" "struct msghdr *" "int" */
#define	SYS_recvmsg	27

/* syscall: "sendmsg" ret: "ssize_t" args: "int" "const struct msghdr *" "int" */
#define	SYS_sendmsg	28

/* syscall: "recvfrom" ret: "ssize_t" args: "int" "void *" "size_t" "int" "struct sockaddr *" "socklen_t *" */
#define	SYS_recvfrom	29

/* syscall: "accept" ret: "int" args: "int" "struct sockaddr *" "socklen_t *" */
#define	SYS_accept	30

/* syscall: "getpeername" ret: "int" args: "int" "struct sockaddr *" "int *" */
#define	SYS_getpeername	31

/* syscall: "getsockname" ret: "int" args: "int" "struct sockaddr *" "socklen_t *" */
#define	SYS_getsockname	32

/* syscall: "access" ret: "int" args: "const char *" "int" */
#define	SYS_access	33

/* syscall: "chflags" ret: "int" args: "const char *" "u_int" */
#define	SYS_chflags	34

/* syscall: "fchflags" ret: "int" args: "int" "u_int" */
#define	SYS_fchflags	35

/* syscall: "sync" ret: "void" args: */
#define	SYS_sync	36

/* syscall: "kill" ret: "int" args: "int" "int" */
#define	SYS_kill	37

				/* 38 is compat_43 ostat */

/* syscall: "getppid" ret: "pid_t" args: */
#define	SYS_getppid	39

				/* 40 is compat_43 olstat */

/* syscall: "dup" ret: "int" args: "int" */
#define	SYS_dup	41

/* syscall: "opipe" ret: "int" args: */
#define	SYS_opipe	42

/* syscall: "getegid" ret: "gid_t" args: */
#define	SYS_getegid	43

/* syscall: "profil" ret: "int" args: "caddr_t" "size_t" "u_long" "u_int" */
#define	SYS_profil	44

/* syscall: "ktrace" ret: "int" args: "const char *" "int" "int" "pid_t" */
#define	SYS_ktrace	45

/* syscall: "sigaction" ret: "int" args: "int" "const struct sigaction *" "struct sigaction *" */
#define	SYS_sigaction	46

/* syscall: "getgid" ret: "gid_t" args: */
#define	SYS_getgid	47

/* syscall: "sigprocmask" ret: "int" args: "int" "sigset_t" */
#define	SYS_sigprocmask	48

/* syscall: "getlogin" ret: "int" args: "char *" "u_int" */
#define	SYS_getlogin	49

/* syscall: "setlogin" ret: "int" args: "const char *" */
#define	SYS_setlogin	50

/* syscall: "acct" ret: "int" args: "const char *" */
#define	SYS_acct	51

/* syscall: "sigpending" ret: "int" args: */
#define	SYS_sigpending	52

/* syscall: "sigaltstack" ret: "int" args: "const struct sigaltstack *" "struct sigaltstack *" */
#define	SYS_sigaltstack	53

/* syscall: "ioctl" ret: "int" args: "int" "u_long" "..." */
#define	SYS_ioctl	54

/* syscall: "reboot" ret: "int" args: "int" */
#define	SYS_reboot	55

/* syscall: "revoke" ret: "int" args: "const char *" */
#define	SYS_revoke	56

/* syscall: "symlink" ret: "int" args: "const char *" "const char *" */
#define	SYS_symlink	57

/* syscall: "readlink" ret: "int" args: "const char *" "char *" "size_t" */
#define	SYS_readlink	58

/* syscall: "execve" ret: "int" args: "const char *" "char *const *" "char *const *" */
#define	SYS_execve	59

/* syscall: "umask" ret: "int" args: "int" */
#define	SYS_umask	60

/* syscall: "chroot" ret: "int" args: "const char *" */
#define	SYS_chroot	61

				/* 62 is compat_43 ofstat */

				/* 63 is compat_43 ogetkerninfo */

				/* 64 is compat_43 ogetpagesize */

				/* 65 is compat_25 omsync */

/* syscall: "vfork" ret: "int" args: */
#define	SYS_vfork	66

				/* 67 is obsolete vread */
				/* 68 is obsolete vwrite */
/* syscall: "sbrk" ret: "int" args: "int" */
#define	SYS_sbrk	69

/* syscall: "sstk" ret: "int" args: "int" */
#define	SYS_sstk	70

				/* 71 is compat_43 ommap */

/* syscall: "vadvise" ret: "int" args: "int" */
#define	SYS_vadvise	72

/* syscall: "munmap" ret: "int" args: "void *" "size_t" */
#define	SYS_munmap	73

/* syscall: "mprotect" ret: "int" args: "void *" "size_t" "int" */
#define	SYS_mprotect	74

/* syscall: "madvise" ret: "int" args: "void *" "size_t" "int" */
#define	SYS_madvise	75

				/* 76 is obsolete vhangup */
				/* 77 is obsolete vlimit */
/* syscall: "mincore" ret: "int" args: "void *" "size_t" "char *" */
#define	SYS_mincore	78

/* syscall: "getgroups" ret: "int" args: "int" "gid_t *" */
#define	SYS_getgroups	79

/* syscall: "setgroups" ret: "int" args: "int" "const gid_t *" */
#define	SYS_setgroups	80

/* syscall: "getpgrp" ret: "int" args: */
#define	SYS_getpgrp	81

/* syscall: "setpgid" ret: "int" args: "pid_t" "int" */
#define	SYS_setpgid	82

/* syscall: "setitimer" ret: "int" args: "int" "const struct itimerval *" "struct itimerval *" */
#define	SYS_setitimer	83

				/* 84 is compat_43 owait */

				/* 85 is compat_25 swapon */

/* syscall: "getitimer" ret: "int" args: "int" "struct itimerval *" */
#define	SYS_getitimer	86

				/* 87 is compat_43 ogethostname */

				/* 88 is compat_43 osethostname */

				/* 89 is compat_43 ogetdtablesize */

/* syscall: "dup2" ret: "int" args: "int" "int" */
#define	SYS_dup2	90

/* syscall: "fcntl" ret: "int" args: "int" "int" "..." */
#define	SYS_fcntl	92

/* syscall: "select" ret: "int" args: "int" "fd_set *" "fd_set *" "fd_set *" "struct timeval *" */
#define	SYS_select	93

/* syscall: "fsync" ret: "int" args: "int" */
#define	SYS_fsync	95

/* syscall: "setpriority" ret: "int" args: "int" "int" "int" */
#define	SYS_setpriority	96

/* syscall: "socket" ret: "int" args: "int" "int" "int" */
#define	SYS_socket	97

/* syscall: "connect" ret: "int" args: "int" "const struct sockaddr *" "socklen_t" */
#define	SYS_connect	98

				/* 99 is compat_43 oaccept */

/* syscall: "getpriority" ret: "int" args: "int" "int" */
#define	SYS_getpriority	100

				/* 101 is compat_43 osend */

				/* 102 is compat_43 orecv */

/* syscall: "sigreturn" ret: "int" args: "struct sigcontext *" */
#define	SYS_sigreturn	103

/* syscall: "bind" ret: "int" args: "int" "const struct sockaddr *" "socklen_t" */
#define	SYS_bind	104

/* syscall: "setsockopt" ret: "int" args: "int" "int" "int" "const void *" "socklen_t" */
#define	SYS_setsockopt	105

/* syscall: "listen" ret: "int" args: "int" "int" */
#define	SYS_listen	106

				/* 107 is obsolete vtimes */
				/* 108 is compat_43 osigvec */

				/* 109 is compat_43 osigblock */

				/* 110 is compat_43 osigsetmask */

/* syscall: "sigsuspend" ret: "int" args: "int" */
#define	SYS_sigsuspend	111

				/* 112 is compat_43 osigstack */

				/* 113 is compat_43 orecvmsg */

				/* 114 is compat_43 osendmsg */

				/* 115 is obsolete vtrace */
/* syscall: "gettimeofday" ret: "int" args: "struct timeval *" "struct timezone *" */
#define	SYS_gettimeofday	116

/* syscall: "getrusage" ret: "int" args: "int" "struct rusage *" */
#define	SYS_getrusage	117

/* syscall: "getsockopt" ret: "int" args: "int" "int" "int" "void *" "socklen_t *" */
#define	SYS_getsockopt	118

				/* 119 is obsolete resuba */
/* syscall: "readv" ret: "ssize_t" args: "int" "const struct iovec *" "int" */
#define	SYS_readv	120

/* syscall: "writev" ret: "ssize_t" args: "int" "const struct iovec *" "int" */
#define	SYS_writev	121

/* syscall: "settimeofday" ret: "int" args: "const struct timeval *" "const struct timezone *" */
#define	SYS_settimeofday	122

/* syscall: "fchown" ret: "int" args: "int" "uid_t" "gid_t" */
#define	SYS_fchown	123

/* syscall: "fchmod" ret: "int" args: "int" "int" */
#define	SYS_fchmod	124

				/* 125 is compat_43 orecvfrom */

/* syscall: "setreuid" ret: "int" args: "uid_t" "uid_t" */
#define	SYS_setreuid	126

/* syscall: "setregid" ret: "int" args: "gid_t" "gid_t" */
#define	SYS_setregid	127

/* syscall: "rename" ret: "int" args: "const char *" "const char *" */
#define	SYS_rename	128

				/* 129 is compat_43 otruncate */

				/* 130 is compat_43 oftruncate */

/* syscall: "flock" ret: "int" args: "int" "int" */
#define	SYS_flock	131

/* syscall: "mkfifo" ret: "int" args: "const char *" "int" */
#define	SYS_mkfifo	132

/* syscall: "sendto" ret: "ssize_t" args: "int" "const void *" "size_t" "int" "const struct sockaddr *" "socklen_t" */
#define	SYS_sendto	133

/* syscall: "shutdown" ret: "int" args: "int" "int" */
#define	SYS_shutdown	134

/* syscall: "socketpair" ret: "int" args: "int" "int" "int" "int *" */
#define	SYS_socketpair	135

/* syscall: "mkdir" ret: "int" args: "const char *" "int" */
#define	SYS_mkdir	136

/* syscall: "rmdir" ret: "int" args: "const char *" */
#define	SYS_rmdir	137

/* syscall: "utimes" ret: "int" args: "const char *" "const struct timeval *" */
#define	SYS_utimes	138

				/* 139 is obsolete 4.2 sigreturn */
/* syscall: "adjtime" ret: "int" args: "const struct timeval *" "struct timeval *" */
#define	SYS_adjtime	140

				/* 141 is compat_43 ogetpeername */

				/* 142 is compat_43 ogethostid */

				/* 143 is compat_43 osethostid */

				/* 144 is compat_43 ogetrlimit */

				/* 145 is compat_43 osetrlimit */

				/* 146 is compat_43 okillpg */

/* syscall: "setsid" ret: "int" args: */
#define	SYS_setsid	147

/* syscall: "quotactl" ret: "int" args: "const char *" "int" "int" "char *" */
#define	SYS_quotactl	148

				/* 149 is compat_43 oquota */

				/* 150 is compat_43 ogetsockname */

/* syscall: "nfssvc" ret: "int" args: "int" "void *" */
#define	SYS_nfssvc	155

				/* 156 is compat_43 ogetdirentries */

				/* 157 is compat_25 ostatfs */

				/* 158 is compat_25 ostatfs */

/* syscall: "getfh" ret: "int" args: "const char *" "fhandle_t *" */
#define	SYS_getfh	161

				/* 162 is compat_09 ogetdomainname */

				/* 163 is compat_09 osetdomainname */

				/* 164 is compat_09 ouname */

/* syscall: "sysarch" ret: "int" args: "int" "void *" */
#define	SYS_sysarch	165

				/* 169 is compat_10 osemsys */

				/* 170 is compat_10 omsgsys */

				/* 171 is compat_10 oshmsys */

/* syscall: "pread" ret: "ssize_t" args: "int" "void *" "size_t" "int" "off_t" */
#define	SYS_pread	173

/* syscall: "pwrite" ret: "ssize_t" args: "int" "const void *" "size_t" "int" "off_t" */
#define	SYS_pwrite	174

/* syscall: "setgid" ret: "int" args: "gid_t" */
#define	SYS_setgid	181

/* syscall: "setegid" ret: "int" args: "gid_t" */
#define	SYS_setegid	182

/* syscall: "seteuid" ret: "int" args: "uid_t" */
#define	SYS_seteuid	183

/* syscall: "lfs_bmapv" ret: "int" args: "fsid_t *" "struct block_info *" "int" */
#define	SYS_lfs_bmapv	184

/* syscall: "lfs_markv" ret: "int" args: "fsid_t *" "struct block_info *" "int" */
#define	SYS_lfs_markv	185

/* syscall: "lfs_segclean" ret: "int" args: "fsid_t *" "u_long" */
#define	SYS_lfs_segclean	186

/* syscall: "lfs_segwait" ret: "int" args: "fsid_t *" "struct timeval *" */
#define	SYS_lfs_segwait	187

/* syscall: "stat" ret: "int" args: "const char *" "struct stat *" */
#define	SYS_stat	188

/* syscall: "fstat" ret: "int" args: "int" "struct stat *" */
#define	SYS_fstat	189

/* syscall: "lstat" ret: "int" args: "const char *" "struct stat *" */
#define	SYS_lstat	190

/* syscall: "pathconf" ret: "long" args: "const char *" "int" */
#define	SYS_pathconf	191

/* syscall: "fpathconf" ret: "long" args: "int" "int" */
#define	SYS_fpathconf	192

/* syscall: "swapctl" ret: "int" args: "int" "const void *" "int" */
#define	SYS_swapctl	193

/* syscall: "getrlimit" ret: "int" args: "int" "struct rlimit *" */
#define	SYS_getrlimit	194

/* syscall: "setrlimit" ret: "int" args: "int" "const struct rlimit *" */
#define	SYS_setrlimit	195

/* syscall: "getdirentries" ret: "int" args: "int" "char *" "int" "long *" */
#define	SYS_getdirentries	196

/* syscall: "mmap" ret: "void *" args: "void *" "size_t" "int" "int" "int" "long" "off_t" */
#define	SYS_mmap	197

/* syscall: "__syscall" ret: "quad_t" args: "quad_t" "..." */
#define	SYS___syscall	198

/* syscall: "lseek" ret: "off_t" args: "int" "int" "off_t" "int" */
#define	SYS_lseek	199

/* syscall: "truncate" ret: "int" args: "const char *" "int" "off_t" */
#define	SYS_truncate	200

/* syscall: "ftruncate" ret: "int" args: "int" "int" "off_t" */
#define	SYS_ftruncate	201

/* syscall: "__sysctl" ret: "int" args: "int *" "u_int" "void *" "size_t *" "void *" "size_t" */
#define	SYS___sysctl	202

/* syscall: "mlock" ret: "int" args: "const void *" "size_t" */
#define	SYS_mlock	203

/* syscall: "munlock" ret: "int" args: "const void *" "size_t" */
#define	SYS_munlock	204

/* syscall: "undelete" ret: "int" args: "const char *" */
#define	SYS_undelete	205

/* syscall: "futimes" ret: "int" args: "int" "const struct timeval *" */
#define	SYS_futimes	206

/* syscall: "getpgid" ret: "pid_t" args: "pid_t" */
#define	SYS_getpgid	207

/* syscall: "xfspioctl" ret: "int" args: "int" "char *" "int" "struct ViceIoctl *" "int" */
#define	SYS_xfspioctl	208

				/* 220 is compat_23 __osemctl */

/* syscall: "semget" ret: "int" args: "key_t" "int" "int" */
#define	SYS_semget	221

/* syscall: "semop" ret: "int" args: "int" "struct sembuf *" "u_int" */
#define	SYS_semop	222

				/* 223 is obsolete sys_semconfig */
				/* 224 is compat_23 omsgctl */

/* syscall: "msgget" ret: "int" args: "key_t" "int" */
#define	SYS_msgget	225

/* syscall: "msgsnd" ret: "int" args: "int" "const void *" "size_t" "int" */
#define	SYS_msgsnd	226

/* syscall: "msgrcv" ret: "int" args: "int" "void *" "size_t" "long" "int" */
#define	SYS_msgrcv	227

/* syscall: "shmat" ret: "void *" args: "int" "const void *" "int" */
#define	SYS_shmat	228

				/* 229 is compat_23 oshmctl */

/* syscall: "shmdt" ret: "int" args: "const void *" */
#define	SYS_shmdt	230

/* syscall: "shmget" ret: "int" args: "key_t" "int" "int" */
#define	SYS_shmget	231

/* syscall: "clock_gettime" ret: "int" args: "clockid_t" "struct timespec *" */
#define	SYS_clock_gettime	232

/* syscall: "clock_settime" ret: "int" args: "clockid_t" "const struct timespec *" */
#define	SYS_clock_settime	233

/* syscall: "clock_getres" ret: "int" args: "clockid_t" "struct timespec *" */
#define	SYS_clock_getres	234

/* syscall: "nanosleep" ret: "int" args: "const struct timespec *" "struct timespec *" */
#define	SYS_nanosleep	240

/* syscall: "minherit" ret: "int" args: "void *" "size_t" "int" */
#define	SYS_minherit	250

/* syscall: "rfork" ret: "int" args: "int" */
#define	SYS_rfork	251

/* syscall: "poll" ret: "int" args: "struct pollfd *" "int" "int" */
#define	SYS_poll	252

/* syscall: "issetugid" ret: "int" args: */
#define	SYS_issetugid	253

/* syscall: "lchown" ret: "int" args: "const char *" "uid_t" "gid_t" */
#define	SYS_lchown	254

/* syscall: "getsid" ret: "pid_t" args: "pid_t" */
#define	SYS_getsid	255

/* syscall: "msync" ret: "int" args: "void *" "size_t" "int" */
#define	SYS_msync	256

/* syscall: "__semctl" ret: "int" args: "int" "int" "int" "union semun *" */
#define	SYS___semctl	257

/* syscall: "shmctl" ret: "int" args: "int" "int" "struct shmid_ds *" */
#define	SYS_shmctl	258

/* syscall: "msgctl" ret: "int" args: "int" "int" "struct msqid_ds *" */
#define	SYS_msgctl	259

/* syscall: "getfsstat" ret: "int" args: "struct statfs *" "size_t" "int" */
#define	SYS_getfsstat	260

/* syscall: "statfs" ret: "int" args: "const char *" "struct statfs *" */
#define	SYS_statfs	261

/* syscall: "fstatfs" ret: "int" args: "int" "struct statfs *" */
#define	SYS_fstatfs	262

/* syscall: "pipe" ret: "int" args: "int *" */
#define	SYS_pipe	263

/* syscall: "fhopen" ret: "int" args: "const fhandle_t *" "int" */
#define	SYS_fhopen	264

/* syscall: "fhstat" ret: "int" args: "const fhandle_t *" "struct stat *" */
#define	SYS_fhstat	265

/* syscall: "fhstatfs" ret: "int" args: "const fhandle_t *" "struct statfs *" */
#define	SYS_fhstatfs	266

/* syscall: "preadv" ret: "ssize_t" args: "int" "const struct iovec *" "int" "int" "off_t" */
#define	SYS_preadv	267

/* syscall: "pwritev" ret: "ssize_t" args: "int" "const struct iovec *" "int" "int" "off_t" */
#define	SYS_pwritev	268

/* syscall: "kqueue" ret: "int" args: */
#define	SYS_kqueue	269

/* syscall: "kevent" ret: "int" args: "int" "const struct kevent *" "int" "struct kevent *" "int" "const struct timespec *" */
#define	SYS_kevent	270

/* syscall: "mlockall" ret: "int" args: "int" */
#define	SYS_mlockall	271

/* syscall: "munlockall" ret: "int" args: */
#define	SYS_munlockall	272

/* syscall: "getpeereid" ret: "int" args: "int" "uid_t *" "gid_t *" */
#define	SYS_getpeereid	273

/* syscall: "extattrctl" ret: "int" args: "const char *" "int" "const char *" "int" "const char *" */
#define	SYS_extattrctl	274

/* syscall: "extattr_set_file" ret: "int" args: "const char *" "int" "const char *" "void *" "size_t" */
#define	SYS_extattr_set_file	275

/* syscall: "extattr_get_file" ret: "ssize_t" args: "const char *" "int" "const char *" "void *" "size_t" */
#define	SYS_extattr_get_file	276

/* syscall: "extattr_delete_file" ret: "int" args: "const char *" "int" "const char *" */
#define	SYS_extattr_delete_file	277

/* syscall: "extattr_set_fd" ret: "int" args: "int" "int" "const char *" "void *" "size_t" */
#define	SYS_extattr_set_fd	278

/* syscall: "extattr_get_fd" ret: "ssize_t" args: "int" "int" "const char *" "void *" "size_t" */
#define	SYS_extattr_get_fd	279

/* syscall: "extattr_delete_fd" ret: "int" args: "int" "int" "const char *" */
#define	SYS_extattr_delete_fd	280

/* syscall: "getresuid" ret: "int" args: "uid_t *" "uid_t *" "uid_t *" */
#define	SYS_getresuid	281

/* syscall: "setresuid" ret: "int" args: "uid_t" "uid_t" "uid_t" */
#define	SYS_setresuid	282

/* syscall: "getresgid" ret: "int" args: "gid_t *" "gid_t *" "gid_t *" */
#define	SYS_getresgid	283

/* syscall: "setresgid" ret: "int" args: "gid_t" "gid_t" "gid_t" */
#define	SYS_setresgid	284

/* syscall: "omquery" ret: "int" args: "int" "void **" "size_t" "int" "off_t" */
#define	SYS_omquery	285

/* syscall: "mquery" ret: "void *" args: "void *" "size_t" "int" "int" "int" "long" "off_t" */
#define	SYS_mquery	286

#define	SYS_MAXSYSCALL	287
