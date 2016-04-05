#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

void print$()
{
  write(STDOUT_FILENO, "$ ", 2);
}

void sig_handler(int signo)
{
  if (signo == SIGINT)
  {
    write(STDOUT_FILENO, "\n", 1);
    print$();
  }
}

int main()
{
  while(42) 
  {
    print$();
    int cnt = 0;
    signal(SIGINT, sig_handler);
    char *progs[4096];
    int k = 0;
    int flag = 0;
    char com[4096];
    char arg[4096];
    int first[2];
    int second[2];
    memset(com, 0, sizeof(char) * 4096);
    ssize_t len = read(STDIN_FILENO, com, 4096);
    char* chs = strtok(com, " ");
    while (chs != NULL)
    {
      progs[k++] = chs;
      chs = strtok(NULL, " ");
    }

    int i = 0;
    for (; i < k; i++)
    {
      char * args[4096];
      int t = 0;
      char* chs = strtok(progs[i], " ");
      while (chs != NULL)
      {

        args[t++] = chs;
        chs = strtok(NULL, " ");
      }

      args[t++] = NULL;
 execvp(args[0],  args); 
      pipe(second);
      pid_t child = fork();

      if (!child)
      {
        if (cnt)
          dup2(first[0], 0);
        if (cnt)
          close(first[0]);
        if (cnt)
          close(first[1]);
        if (flag != 0)
          dup2(second[1], 1);
        close(second[0]);
        close(second[1]);
        //execvp(args[0],  args);    
      }
      else
      {
        if (cnt)
          close(first[0]);
        if (cnt)
          close(first[1]);
        if (flag == 0)
          close(second[0]);
        if (flag == 0)
          close(second[1]);
      } 
      first[0] = second[0];
      first[1] = second[1];

      cnt++;    
    }
    pid_t wpid;
    int status;
    while ((wpid = wait(&status)) > 0) { }
  }
 
}
