#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
int main()
{
    if (fork() == 0)
    {
        putchar('A');
        if (fork() == 0)
        {
            putchar('B');
            return 0;
        }
        execl("./foo",”foo”, (char *)0x0);
        wait(0x0);
        putchar('D');
        return 0;
    }
    putchar('E');
    wait(0x0);
    putchar('F');
}