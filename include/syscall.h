#ifndef __SYSCALL_H
#define __SYSCALL_H

long do_syscall0(int sysno);
long do_syscall1(void *arg1, int sysno);
long do_syscall2(void *arg1, void *arg2, int sysno);
long do_syscall3(void *arg1, void *arg2, void *arg3, int sysno);

#endif
