#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <vector>
#include <iostream>
#include <functional>
using namespace std;
int const BUFFER_SIZE = 2048;

int handled_signal;
void handler(int signum)
{
  handled_signal = signum;
}

void error_check(int r, string c = "something")
{
  if (r == -1 && errno != EINTR)
  {
    fprintf(stderr, "Error during %s(): %s\n", c.c_str(), strerror(errno));
    exit(0);
  }
}

void write_all(int fd, string &str)
{
  while (str.size() > 0 && handled_signal == 0)
  {
    int written = write(fd, str.c_str(), str.size());
    if (written != -1)
      str.erase(0, written);
    if (handled_signal != 0)
      break;
    error_check(written);
  }
}
int main()
{
  struct sigaction act;
  act.sa_handler = &handler;
  act.sa_flags = 0;
  
  sigemptyset(&act.sa_mask);
  sigaddset(&act.sa_mask, SIGINT);
  sigaddset(&act.sa_mask, SIGCHLD);
  error_check(sigaction(SIGINT, &act, NULL));
  error_check(sigaction(SIGCHLD, &act, NULL));

  string in;
  char buffer[BUFFER_SIZE];
  int length;

  while (42)
  {
    write(STDOUT_FILENO, "$ ", 2);
    if (in.find('\n') == string::npos)
    {
      length = read(STDIN_FILENO, buffer, BUFFER_SIZE);
      if (length == 0)
      {
	write(STDOUT_FILENO, "\n", 1);
	return 0;
      }
      error_check(length, "read");
      in += {buffer, static_cast<size_t>(length)};
    }
    size_t pos = in.find('\n');

    if (pos != string::npos)
    {
      string command = in.substr(0, pos) + '|';
      in.erase(0, pos + 1);
      int *pipefd = new int[2];
      pipe(pipefd);

      int *cur_pipefd = pipefd;
      bool is_first = true;
      bool is_closed = false;
			
      int command_n = 0;

      vector<int> pids;

      while (command.length() > 0)
      {
	pos = command.find('|');
	string program = command.substr(0, pos) + " ";
	command.erase(0, pos + 1);

	vector<string> words;
	while (program.length() > 0)
	{
	  pos = program.find(' ');
	  if (pos != 0)
	    words.push_back(program.substr(0, pos));
	  program.erase(0, pos + 1);
	}

	char *args[words.size() + 1];
	for (size_t i = 0; i < words.size(); ++i)
	  args[i] = const_cast<char*>(words[i].c_str());
	args[words.size()] = nullptr;

	int *out_pipefd = new int[2];
	pipe(out_pipefd);

	int *in_pipefd = cur_pipefd;
	cur_pipefd = out_pipefd;

	int pid = fork();
	if (pid == 0)
	{
	  dup2(in_pipefd[0], STDIN_FILENO);
	  if (command.length() > 0)
	    dup2(out_pipefd[1], STDOUT_FILENO);
	  close(in_pipefd[0]);
	  close(in_pipefd[1]);
	  close(out_pipefd[0]);
	  close(out_pipefd[1]);
	  delete[] in_pipefd;
	  delete[] out_pipefd;
	  error_check(execvp(args[0], args), "execvp");
	  close(pipefd[0]);
	  close(pipefd[1]);
	  delete[] pipefd;
	  exit(0);
	}
	else
	{
	  pids.push_back(pid);
	  if (is_first)
	    is_first = false;
	  else
	  {
	    close(in_pipefd[0]);
	    close(in_pipefd[1]);
	  }
	  handled_signal = 0;
	}
      }

      int end = 0;
      while (handled_signal != SIGINT && end < pids.size())
      {
	write_all(pipefd[1], in);
	length = read(STDIN_FILENO, buffer, BUFFER_SIZE);	
	error_check(length, "read");
	if (length != -1)
	  in += {buffer, static_cast<size_t>(length)};

	if (length == 0)
	{
	  close(pipefd[1]);
	  is_closed = true;
	}

	if (handled_signal == SIGCHLD)
	  ++end;
      }
      if (handled_signal == SIGINT)
	for (int pid : pids)
          kill(pid, SIGINT);

      if (!is_closed)
	close(pipefd[1]);
      close(cur_pipefd[0]);
      close(cur_pipefd[1]);
      delete[] cur_pipefd;
      while ((length = read(pipefd[0], buffer, BUFFER_SIZE)) > 0)
	in += {buffer, static_cast<size_t>(length)};
      error_check(length);
      close(pipefd[0]);

      error_check(sigaction(SIGINT, &act, NULL));
      error_check(sigaction(SIGCHLD, &act, NULL));
    }
  }
}
