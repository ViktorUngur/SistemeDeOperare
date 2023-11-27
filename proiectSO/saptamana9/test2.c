#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>

#define MAX 250

int main(void)
{
	int pfd2[2];
	int pfd1[2];
	int pid1, pid2;
	char* string = (char*)malloc(sizeof(char)*MAX);
	int childPidCount = 2;

		
	if(pipe(pfd2) < 0 || pipe(pfd1) < 0)
	{
		printf("Eroare la crearea pipe-ului\n");
		exit(1);
	}
	
	if((pid1=fork())<0)
	{
		printf("Eroare la fork\n");
		exit(1);
	}
	if(pid1==0) 
	{
        close(pfd2[0]);
					   
		
		dup2(pfd2[1],1); 
		
		execlp("cat", "cat", "text.txt", NULL); 
		printf("Eroare la exec\n");
		
	}

	close(pfd2[1]);

	if((pid2=fork())<0)
	{
		printf("Eroare la fork\n");
		exit(1);
	}
	if(pid2==0)
	{
		close(pfd2[1]);
		close(pfd1[0]);

		dup2(pfd2[0], STDIN_FILENO);
		dup2(pfd1[1], STDOUT_FILENO);

		execlp("/bin/bash", "bash", "script.sh", "c", NULL);
		printf("Eroare la exec\n");
	}

	close(pfd2[0]);
	close(pfd1[1]);

	read(pfd1[0], string, MAX);

	printf("%s", string);
	close(pfd1[0]);

	for(int i = 0; i<childPidCount; i++)
    {
        int status;
        int pidDone = wait(&status);
        if(pidDone == -1)
        {
            printf("Error in wait function\n");
            exit(-1);
        }

        printf("Process with id %d exited with status %d\n", pidDone, WEXITSTATUS(status));
        
    }
         
    return 0;
}