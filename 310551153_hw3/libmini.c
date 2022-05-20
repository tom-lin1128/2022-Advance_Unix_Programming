#include "libmini.h"
#define sig_mask(_sig)  ( (1UL) << ((_sig) - 1) )

#define	WRAPPER_RETval(type)	errno = 0; if(ret < 0) { errno = -ret; return -1; } return ((type) ret);
#define	WRAPPER_RETptr(type)	errno = 0; if(ret < 0) { errno = -ret; return NULL; } return ((type) ret);

long errno;

void *memset(void *d, int val, long long len) {
    char *ptr = (char *)d;
    while(len-- > 0) *ptr++ = val;
    return d;
}

size_t strlen(const char *s){
    size_t count = 0;
    while(*s++) count++;
    return count;
}

ssize_t	 write(int fd, const void *buf, size_t	count){
     long ret = sys_write(fd, buf, count);
     return ret;
}

void exit(int error_code){
    sys_exit(error_code);
}

unsigned int alarm(unsigned int sec){
    long ret = sys_alarm(sec);
    return (unsigned int)ret;
}

long pause(){
    long ret = sys_pause();
    return ret;
}

unsigned sleep(unsigned seconds){
    struct timespec rqtp, rmtp;
    rqtp.tv_sec = seconds;
    rqtp.tv_nsec = 0;
    long ret = sys_nanosleep(&rqtp,&rmtp);
    return ret;
}

#define	PERRMSG_MIN	0
#define	PERRMSG_MAX	34

static const char *errmsg[] = {
    "Success",
    "Operation not permitted",
    "No such file or directory",
    "No such process",
    "Interrupted system call",
    "I/O error",
    "No such device or address",
    "Argument list too long",
    "Exec format error",
    "Bad file number",
    "No child processes",
    "Try again",
    "Out of memory",
    "Permission denied",
    "Bad address",
    "Block device required",
    "Device or resource busy",
    "File exists",
    "Cross-device link",
    "No such device",
    "Not a directory",
    "Is a directory",
    "Invalid argument",
    "File table overflow",
    "Too many open files",
    "Not a typewriter",
    "Text file busy",
    "File too large",
    "No space left on device",
    "Illegal seek",
    "Read-only file system",
    "Too many links",
    "Broken pipe",
    "Math argument out of domain of func",
    "Math result not representable"
};


void perror(const char *prefix) {
    const char *unknown = "Unknown";
    long backup = errno;
    if(prefix) {
        write(2, prefix, strlen(prefix));
        write(2, ": ", 2);
    }
    if(errno < PERRMSG_MIN || errno > PERRMSG_MAX) write(2, unknown, strlen(unknown));
    else write(2, errmsg[backup], strlen(errmsg[backup]));
    write(2, "\n", 1);
    return;
}


int sigaction(int signum, struct sigaction *act, struct sigaction *oldact){
    act->sa_flags |= SA_RESTORER;
	act->sa_restorer = __myrt /* your customized restore routine, e.g., __myrt */;
	int ret = sys_rt_sigaction(signum, act, oldact, sizeof(sigset_t));
    return ret;
}

sighandler_t signal(int signum, sighandler_t handler){
    struct sigaction ac, oac;
    ac.sa_handler = handler;
    ac.sa_flags = SA_RESTART;
    sigemptyset(&ac.sa_mask);
    sigaddset(&ac.sa_mask,signum);
    sigaction(signum,&ac,&oac);
    return oac.sa_handler;
}


int sigismember(const sigset_t *set, int _sig){
    return (set->val[0] & sig_mask(_sig)) == 0 ? 0 : 1;
}

int sigaddset(sigset_t *set, int _sig){
    set->val[0] |= sig_mask(_sig);
    return 0;
}

int sigdelset(sigset_t *set, int _sig){
    set->val[0] &= ~(sig_mask(_sig));
    return 0;
}

int sigemptyset(sigset_t *set){
    memset(set, 0, sizeof(sigset_t));
    return 0;
}

int sigfillset(sigset_t *set){
    memset(set, -1, sizeof(sigset_t));
    return 0;
}

int sigpending(sigset_t *set){
    long ret = sys_rt_sigpending(set, 8);
    return ret;
}

int sigprocmask(int how, const sigset_t *set, sigset_t *oldset){
    long ret = sys_rt_sigprocmask(how, set, oldset,  sizeof(sigset_t));
    return ret;
}
