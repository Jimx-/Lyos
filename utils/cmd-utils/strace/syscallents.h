/* clang-format off */
#define _S(nr, name) [nr] = {nr, SEN(name)}

_S(OPEN, open),
_S(OPENAT, openat),
_S(CLOSE, close),
_S(READ, read),
_S(WRITE, write),
_S(GETSETID, getsetid),
_S(GETDENTS, getdents),
_S(STAT, stat),
_S(LSTAT, lstat),
_S(FSTAT, fstat),
_S(FCNTL, fcntl),
_S(ACCESS, access),
_S(BRK, brk),
_S(MMAP, mmap),
_S(MUNMAP, munmap),
_S(DUP, dup),
_S(UMASK, umask),
_S(CHMOD, chmod),
_S(IOCTL, ioctl),
_S(SYMLINK, symlink),
_S(UNLINK, unlink),
_S(PIPE2, pipe2),
_S(FORK, fork),
_S(EXEC, exec),
_S(WAIT, waitpid),
_S(POLL, poll),
_S(EVENTFD, eventfd),
_S(SIGPROCMASK, sigprocmask),
_S(SIGNALFD, signalfd),
_S(KILL, kill),
_S(TIMERFD_CREATE, timerfd_create),
_S(TIMERFD_SETTIME, timerfd_settime),
_S(TIMERFD_GETTIME, timerfd_gettime),
_S(EPOLL_CREATE1, epoll_create1),
_S(EPOLL_CTL, epoll_ctl),
_S(EPOLL_WAIT, epoll_wait),
_S(SOCKET, socket),
_S(BIND, bind),
_S(CONNECT, connect),
_S(LISTEN, listen),
_S(ACCEPT, accept),
_S(SENDTO, sendto),
_S(RECVFROM, recvfrom),
_S(SENDMSG, sendmsg),
_S(RECVMSG, recvmsg),

#undef _S
    /* clang-format on */
