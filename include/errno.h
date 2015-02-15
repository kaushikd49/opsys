#ifndef __ERRNO_H
#define __ERRNO_H
//source:http://lxr.free-electrons.com/source/include/asm-x86_64/errno.h?v=2.4.37
//Note: Reference has been made to the linux errno.h which is distributed under the GPLv2 license
// Not all the constants are used and this errno.h only includes those errno(s) that this Sbush needs.
//for the sake clarity, the numbers have been retained as it is, since it matches the values that syscall returns,
//there in lies the main reason for using the errno provided by linux.
int errno;
#define EBADF 9
#define READERR 127
#define WRITEERR 126
//open errors
#define ENOMEM 12
#define EACCES 13
#define OPENERROR 125
#define ENOENT 2
#define EEXIST 17
#define EDQUOT 122
#define EFAULT 14
#define EFBIG 27
#define EINTR 4
#define EISDIR 21
#define ELOOP 40
#define EMFILE 24
#define ENAMETOOLONG 36
#define EISIDIR 21
#define ENFILE 23
#define ENODEV 19
#define ENOMEM 12
#define ENOSPC 28
#define ENOTDIR 20
#define ENXIO 6
#define EOVERFLOW 75
#define EPERM 1
#define EROFS 30
#define ETXTBSY 26
#define EWOULDBLOCK 11
//end open errors
//fork errors
#define EAGAIN 11
//execve errors
#define E2BIG 7
#define ENOEXEC 8
#define EIO 5
#define EXECVEERROR 128
//wait pid errors
#define EINVAL 22
#define ECHILD 10
//lseek
#define LSEEKERROR 129
//sleep
#define NANOSLEEPERROR 130
#define CHDIRERROR 131
//getcwd
#define GETCWDERROR 132
#define ERANGE 34
#endif
