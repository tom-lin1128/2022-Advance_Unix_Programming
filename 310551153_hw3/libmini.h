#define SIGHUP		 1
#define SIGINT		 2
#define SIGQUIT		 3
#define SIGILL		 4
#define SIGTRAP		 5
#define SIGABRT		 6
#define SIGIOT		 6
#define SIGBUS		 7
#define SIGFPE		 8
#define SIGKILL		 9
#define SIGUSR1		10
#define SIGSEGV		11
#define SIGUSR2		12
#define SIGPIPE		13
#define SIGALRM		14
#define SIGTERM		15
#define SIGSTKFLT	16
#define SIGCHLD		17
#define SIGCONT		18
#define SIGSTOP		19
#define SIGTSTP		20
#define SIGTTIN		21
#define SIGTTOU		22
#define SIGURG		23
#define SIGXCPU		24
#define SIGXFSZ		25
#define SIGVTALRM	26
#define SIGPROF		27
#define SIGWINCH	28
#define SIGIO		29
#define SIGPWR      30
#define SIGSYS      31

#define NSIG        (31 + 1)

#define	SIG_BLOCK     0		 /* Block signals.  */
#define	SIG_UNBLOCK   1		 /* Unblock signals.  */
#define	SIG_SETMASK   2		 /* Set the set of blocked signals.*/

#define SIG_ERR       ((sighandler_t)-1)
#define SIG_DEF       ((sighandler_t) 0)
#define SIG_IGN       ((sighandler_t) 1)

#define SA_RESTORER        0x04000000
#define SA_RESTART         0x10000000 /* Restart syscall on signal return.  */

#define NULL ((void*)0)

typedef long long size_t;
typedef long long ssize_t;
typedef void (*sighandler_t) (int);
typedef void (*sigrestore_t) (void);

struct timespec {
    long	tv_sec;		
    long	tv_nsec;	
};

typedef struct {
    unsigned long int val[1];
} sigset_t;

struct sigaction {
    sighandler_t    sa_handler;
    unsigned int    sa_flags;
    sigrestore_t    sa_restorer;
    sigset_t        sa_mask;
};

typedef struct jmp_buf_s {
	long long reg[8];
	sigset_t mask;
} jmp_buf[1];

//syscall
long sys_write(int fd, const void *buf, ssize_t count);
long sys_exit(int error_code);
long sys_alarm(unsigned int sec);
long sys_pause();
long sys_nanosleep(struct timespec *rqtp, struct timespec *rmtp);
long sys_rt_sigaction(int sig, const struct sigaction *act, struct sigaction *oact, size_t sigsetsize);
long sys_rt_sigpending(sigset_t *set, size_t sigsetsize);
long sys_rt_sigprocmask(int how, const sigset_t *set, sigset_t *oldset, size_t sigsetsize);
void __myrt();

//function
ssize_t write(int fd, const void *buf, size_t count);
void exit(int error_code);
size_t strlen(const char *);
long pause();
unsigned sleep(unsigned);

//homework
int sigaction(int, struct sigaction *, struct sigaction *);
int sigismember(const sigset_t *, int);
int sigaddset (sigset_t *, int);
int sigdelset (sigset_t *, int);
int sigemptyset(sigset_t *);
int sigfillset(sigset_t *);
int sigpending(sigset_t *);
int sigprocmask(int, const sigset_t *, sigset_t *);
sighandler_t signal(int, sighandler_t);
int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);
unsigned int alarm(unsigned int);
void perror(const char *);