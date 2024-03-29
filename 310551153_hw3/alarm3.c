#include "libmini.h"

void handler(int s) { /* do nothing */
}

int main() {
    sigset_t s;
    sigemptyset(&s);
    sigaddset(&s, SIGALRM);
    sigprocmask(SIG_BLOCK, &s, NULL);
    signal(SIGALRM, SIG_IGN);
    signal(SIGINT, handler);
    alarm(1);
    pause();
    if (sigpending(&s) < 0) {
        char m[] = "sigalrm is pend.\n";
        write(1, m, strlen(m));
    }
    if (sigismember(&s, SIGALRM)) {
        char m[] = "sigalrm is pending.\n";
        write(1, m, strlen(m));
    } else {
        char m[] = "sigalrm is not pending.\n";
        write(1, m, strlen(m));
    }
    return 0;
}