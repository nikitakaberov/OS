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
    signal(SIGINT, sig_handler);
    char *progs[4096];
    int k = 0;
    char com[4096];
    char arg[4096];
    int first[2];
    int second[2];
    memset(com, 0, sizeof(char) * 4096);
    ssize_t len = read(STDIN_FILENO, com, 4096);
    com[len - 1] = 0;
    char* chs = strtok(com, "|");
    while (chs != NULL)
    {
      progs[k++] = chs;
      chs = strtok(NULL, "|");
    }

    int childs[4096];
    int cld_cnt = 0;
    int i = 0;
    for (; i < k; i++)
    {
    
      char * args[4096];
      int t = 0;
      memset(args, 0, sizeof(args));
      char* chs = strtok(progs[i], " ");
      while (chs != NULL)
      {

        args[t++] = chs;
        chs = strtok(NULL, " ");
      }

      args[t++] = NULL;
      pipe(second);
      pid_t child = fork();
      if (child  < 0)
      {
        perror("Failed\n");
      }
      else
      {
        childs[cld_cnt++] = child;
      }
      if (child == 0)
      {
        if (i > 0)
        {
          dup2(first[0], STDIN_FILENO);
          close(first[0]);
          close(first[1]);
        }
        if (i != k - 1)
          dup2(second[1], STDOUT_FILENO);
        close(second[0]);
        close(second[1]);
        execvp(args[0], args);
      }
      else
      {
        if (i > 0)
        {
          close(first[0]);
          close(first[1]);
        }
        if (i == k - 1)
        {
        close(second[0]);
        close(second[1]);
        }
      }
      first[0] = second[0];
      first[1] = second[1];
     }
    for (i = 0; i < cld_cnt; i++)
       waitpid(childs[i], 0, NULL);
 
  }
 
}
