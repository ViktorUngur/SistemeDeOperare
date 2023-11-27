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


#define SIZE_PATH 100
#define OFFSET_WIDTH 18
#define OFFSET_RGB 54
#define RDWR_BUFFER 4096
#define HEIGHT_FIELD_SIZE 4
#define WIDTH_FIELD_SIZE 4
#define FORMATTED_STRING_SIZE 500
#define TIME_STRING_SIZE 11
#define RIGHTS_STRING_SIZE 4
#define WRITE_BUFFER 100
#define FILE_STRING_SIZE 50
#define INPUT_CHAR_SIZE 2

DIR* openDirectory(char* path)
{
    DIR* directory;
    directory = opendir(path);
    if(directory == NULL)
    {
        printf("Error during directory opening\n");
        exit(-1);
    }

    return directory;
}

void closeDirectory(DIR* directory)
{
    if(closedir(directory) == -1)
    {
        printf("Error during directory closing\n");
        exit(-1);
    }
}

void createWriteRegularFileProcess(char* outputDirectoryPath, char* fileName, char* content, int* pfdBrothers, char* pathToFile)
{
    int pid = fork();
    int totalLines = 0;
    if(pid < 0)
    {
        printf("Error with process\n");
        exit(-1);
    }
    else if(pid == 0)
    {
        close(pfdBrothers[0]);

        dup2(pfdBrothers[1], 1);  
        close(pfdBrothers[1]);
        
        execlp("cat", "cat", pathToFile, NULL);
        exit(totalLines);
    }
}

int createCountValidSentenceProcess(int* pfdBrothers, int* pfdFatherChild, char* inputCharacter)
{
    int pid = fork();
    if(pid < 0)
    {
        printf("Error with process\n");
        exit(-1);
    }
    else if(pid == 0)
    {
        close(pfdBrothers[1]);  
        close(pfdFatherChild[0]); 
        
        
        dup2(pfdBrothers[0], 0);
        dup2(pfdFatherChild[1], 1);

        char* string = (char*)malloc(sizeof(char)*30);

        FILE *stream = NULL;
        printf("after stream\n");
        
        stream=fdopen(pfdBrothers[0],"r"); 

        fscanf(stream,"%s",string);
        printf("%s\n", string);
        
        //close(pfdBrothers[0]); 
        //close(pfdFatherChild[1]);

        //execlp("/bin/bash", "bash", "script.sh", inputCharacter, NULL);
        exit(0);
    }
    else
    {
        return pid;
    }
}

void readMyDirectory(DIR* inputDirectory, char* inputDirectoryPath, char* outputDirectoryPath, char* inputCharacter)
{
    struct dirent* direntStruct;
    char* newPath = (char*)malloc(sizeof(char) * FILE_STRING_SIZE);

    int childPidCount = 0;
    int validPid = 0;
    int countCorrectSentence = 0;

    while((direntStruct = readdir(inputDirectory)) != NULL)
    {
        if((strcmp(".", direntStruct->d_name) != 0) && (strcmp("..", direntStruct->d_name) != 0))
        {
            sprintf(newPath, "%s/%s", inputDirectoryPath, direntStruct->d_name);
            struct stat fileInfo;
            if(!lstat(newPath, &fileInfo))
            {
                char* formattedString = (char*)malloc(sizeof(char)*FORMATTED_STRING_SIZE);
                if(S_ISREG(fileInfo.st_mode))
                {
                    int pfdFatherChild[2];
                    int pfdBrothers[2];

                    if(pipe(pfdBrothers) < 0)
                    {
                        printf("Error with pipe\n");
                        exit(-1);
                    }

                    if(pipe(pfdFatherChild) < 0)
                    {
                        printf("Error with pipe\n");
                        exit(-1);
                    }
                    
                    createWriteRegularFileProcess(outputDirectoryPath, direntStruct->d_name, formattedString, pfdBrothers, newPath);
                    validPid = createCountValidSentenceProcess(pfdBrothers, pfdFatherChild, inputCharacter);
                    
                    close(pfdFatherChild[1]);
                    char* string = (char*)malloc(sizeof(char)*30);

                    FILE *stream = NULL;
                    
                    stream=fdopen(pfdFatherChild[0],"r");
                    fscanf(stream,"%s",string);
                    printf("%s\n", string);

                    countCorrectSentence += 2;
                    childPidCount+=2;

                    close(pfdFatherChild[0]);
                    close(pfdFatherChild[1]);
                    close(pfdBrothers[0]);
                    close(pfdBrothers[1]);    
                    
                }

            }
            else
            {
                printf("Error with lstat function\n");
                exit(-1);
            }
        }
    }

    for(int i = 0; i<childPidCount; i++)
    {
        int status;
        int pidDone = wait(&status);
        if(pidDone == -1)
        {
            printf("Error in wait function\n");
            exit(-1);
        }

        if (pidDone == validPid && WIFEXITED(status))
        {
            printf("Process with id %d exited with status %d\n", pidDone, WEXITSTATUS(status));
        }
    }

    printf("Total valid sentences in text files: %d\n", countCorrectSentence);
}

int main(void)
{
    

    char* inputCharacter = "C";
  
    char* inputPath = "director";

    char* outputPath = "outputDir";

    DIR* inputDirectory = openDirectory(inputPath);

    readMyDirectory(inputDirectory, inputPath, outputPath, inputCharacter);

    closeDirectory(inputDirectory); 

    free(inputPath);

    return 0;
}