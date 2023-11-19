#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    char caracter;
    //char* argv[] = {"6.2"};
    int i = 0;
    int pid;
    int status;

    if ((pid = fork()) < 0)
    {
        printf("Error creating process\n");
        exit(1);
    }
    
    if (pid == 0)
    {
        char* args[] = {"./s", "6.2", NULL};
        execvp(args[0], args);
        printf("execvp failed\n");
        exit(EXIT_FAILURE);
    }

    while (i < 10)
    {
        scanf(" %c", &caracter);
        sleep(2);
        i++;
    }

    if (wait(&status) == -1)
    {
        perror("waitpid");
        exit(EXIT_FAILURE);
    }

    return 0;
}
