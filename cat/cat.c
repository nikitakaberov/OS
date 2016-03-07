#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

ssize_t cat(ssize_t fn)
{
    ssize_t nread;
    char buffer[128];
    while ((nread = read(fn, buffer, 128)) > 0)
    {
        ssize_t nwrite = write(STDOUT_FILENO, buffer, nread);
        if (nwrite < nread)
        {
            perror("write's error");
            return -1;
        }
    }

    if (nread != 0)
        return -1;

    return 0;
}

int main(int argc, char ** argv)
{
    if (argc < 2)
    {
        if (cat(STDIN_FILENO) != 0)
        {
            perror("read's error");
            return -1;
        }
        return 0;
    }

    int i;
    for (i = 1; i < argc; i++)
    {
        int f;
        if ((f = open(argv[i], O_RDONLY)) == -1)
            return -1;
        else
        {
            if (cat(f) != 0 || close(f) != 0)
                return -1;
        }
    }
    return 0;
}
