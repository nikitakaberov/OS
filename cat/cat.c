#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

ssize_t cat(ssize_t fn)
{
    char buffer[2048];
    ssize_t nread;

    while (42)
    {
        nread = read(fn, buffer, sizeof(buffer));
        if (nread < 0)
        {
            if (errno == EINTR)
                continue;
            break;
        } else if (nread == 0)
            break;
        ssize_t p = 0;
        while (p < nread)
        {   
            ssize_t nwrite = write(STDOUT_FILENO, buffer + p, nread - p);
            if (nwrite < 0)
            {
                if (errno == EINTR)
                    continue;
                return -1;
            }
            p += nwrite;
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
