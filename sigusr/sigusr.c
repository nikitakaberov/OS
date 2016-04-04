#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

void sig_handler(int signo, siginfo_t *info, void *context)
{
  pid_t pid = info->si_pid;
  if (signo == SIGUSR1 || signo == SIGUSR2)
  {
    printf("%s from %lu\n", signo == SIGUSR1 ? "SIGUSR1" : "SIGUSR2", (unsigned long) pid);
    exit(signo);
  }
}

int main()
{
  struct sigaction s;
  s.sa_sigaction = *sig_handler;
  s.sa_flags = SA_SIGINFO;
  sigset_t set;
  sigfillset(&set);
  s.sa_mask = set;
  if (sigaction(SIGUSR1, &s, NULL) != 0 || sigaction(SIGUSR1, &s, NULL) != 0)
  {
    printf("ERROR!\n");
    return errno;
  }
  sleep(10);
  printf("No signals were caught\n");
  return 0;
}
