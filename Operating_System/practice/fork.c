#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
int main()
{
    if (fork() == 0)
    {
        printf("child process generated\n");
        printf("executing foo...\n");
        execl("./foo", "foo", (char *)0x0);
        perror("fail to execute foo\n");
        return 1;
    }
    printf("wait for child process\n");
    wait(0x0);
    printf("child process must be terminated now\nbye!");
    return 0;
}

