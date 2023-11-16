#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>


#define SIZE_PATH 100
#define OFFSET 18
#define HEIGHT_FIELD_SIZE 4
#define WIDTH_FIELD_SIZE 4
#define FORMATTED_STRING_SIZE 500
#define TIME_STRING_SIZE 11
#define RIGHTS_STRING_SIZE 4
#define WRITE_BUFFER 100
#define FILE_STRING_SIZE 50

int isImageBMP(char* fileName)
{
    char* copyFileName = (char*)malloc(sizeof(char)*SIZE_PATH);
    strcpy(copyFileName, fileName);
    strtok(copyFileName, ".");
    if(strcmp(strtok(NULL, "."), "bmp") == 0)
    {
        free(copyFileName);
        return 1;
    }
    else
    {
        free(copyFileName);
        return 0;
    }
}

void getImageWidthAndHeight(int fileDescriptor, int* width, int* height) 
{
    if(lseek(fileDescriptor, OFFSET, SEEK_SET) == -1)
    {
        printf("Error when setting the cursor\n");
        exit(-1);
    }

    if(read(fileDescriptor, width, WIDTH_FIELD_SIZE) == -1)
    {
        printf("Error while reading\n");
        exit(-1);
    }

    if(read(fileDescriptor, height, HEIGHT_FIELD_SIZE) == -1)
    {
        printf("Error while reading\n");
        exit(-1);
    }
}

void formatTime(struct timespec initialRepresentation, char* timeString)
{
    //seconds since epoch
    time_t seconds = initialRepresentation.tv_sec;

    //breaking down into segments
    struct tm timeInfo; 
    localtime_r(&seconds, &timeInfo);

    strftime(timeString, TIME_STRING_SIZE, "%d.%m.%Y", &timeInfo);

    //return timeString;
}

char* formatAccessRightsUser(mode_t mod)
{
    char* rights = (char*)malloc(sizeof(char)*RIGHTS_STRING_SIZE);
    
    rights[0] = (mod & S_IRUSR) ? 'R' : '-';
    rights[1] = (mod & S_IWUSR) ? 'W' : '-';
    rights[2] = (mod & S_IXUSR) ? 'X' : '-';
    rights[3] = 0;

    return rights;
}

char* formatAccessRightsGroup(mode_t mod)
{
    char* rights = (char*)malloc(sizeof(char)*RIGHTS_STRING_SIZE);
    
    rights[0] = (mod & S_IRGRP) ? 'R' : '-';
    rights[1] = (mod & S_IWGRP) ? 'W' : '-';
    rights[2] = (mod & S_IXGRP) ? 'X' : '-';
    rights[3] = 0;

    return rights;
}

char* formatAccessRightsOthers(mode_t mod)
{
    char* rights = (char*)malloc(sizeof(char)*RIGHTS_STRING_SIZE);
    
    rights[0] = (mod & S_IROTH) ? 'R' : '-';
    rights[1] = (mod & S_IWOTH) ? 'W' : '-';
    rights[2] = (mod & S_IXOTH) ? 'X' : '-';
    rights[3] = 0;

    return rights;
}

void closeFile(int fileDescriptor)
{
    if(close(fileDescriptor) == -1)
    {
        printf("Error with file close\n");
        exit(-1);
    }
}

int openFile(char* path, int mode)
{
    int fileDescriptor = open(path, mode);
    if(fileDescriptor == -1)
    {
        printf("Error with file opening\n");
        exit(-1);
    }

    return fileDescriptor;
}

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

int writeFile(int fileDescriptor, char* content)
{
    ssize_t writtenBytes = 0;
    size_t remainingBytes = strlen(content);

    while (remainingBytes > 0) {
        int bytesToWrite = (remainingBytes > WRITE_BUFFER) ? WRITE_BUFFER : remainingBytes;

        if((writtenBytes = write(fileDescriptor, content, bytesToWrite)) == -1)
        {
            printf("Error while writining to file\n");
            exit(-1);
        }

        remainingBytes -= writtenBytes;
        content += writtenBytes;
    }

    if(!remainingBytes)
        return 1;
    
    return 0;
}

void formatImageOutput(char* stringToFormat, char* inputPath, int width, int height, struct stat info)
{
    char* timeString = (char*)malloc(sizeof(char)*TIME_STRING_SIZE);
    formatTime(info.st_mtim, timeString);

    strtok(inputPath, "/");

    sprintf(stringToFormat, "nume fisier: %s\n"
                             "inaltime: %d\n"
                             "lungime: %d\n"
                             "dimensiune: %ld\n"
                             "identificatorul utilizatorului: %d\n"
                             "timpul ultimei modificari: %s\n"
                             "contorul de legaturi: %ld\n"
                             "drepturi de acces user: %s\n"
                             "drepturi de acces grup: %s\n"
                             "drepturi de acces altii: %s\n\n",
                             strtok(NULL, "/"), height, width, info.st_size, info.st_uid, timeString, info.st_nlink, formatAccessRightsUser(info.st_mode), formatAccessRightsGroup(info.st_mode), formatAccessRightsOthers(info.st_mode));

    free(timeString);
}

void formatRegularFileOutput(char* stringToFormat, char* inputPath, struct stat info)
{
    char* timeString = (char*)malloc(sizeof(char)*TIME_STRING_SIZE);
    formatTime(info.st_mtim, timeString);

    strtok(inputPath, "/");

    sprintf(stringToFormat, "nume fisier: %s\n"
                             "dimensiune: %ld\n"
                             "identificatorul utilizatorului: %d\n"
                             "timpul ultimei modificari: %s\n"
                             "contorul de legaturi: %ld\n"
                             "drepturi de acces user: %s\n"
                             "drepturi de acces grup: %s\n"
                             "drepturi de acces altii: %s\n\n",
                             strtok(NULL, "/"), info.st_size, info.st_uid, timeString, info.st_nlink, formatAccessRightsUser(info.st_mode), formatAccessRightsGroup(info.st_mode), formatAccessRightsOthers(info.st_mode));

    free(timeString);
}

void formatSymbolicLinkOutput(char* stringToFormat, char* inputPath, struct stat info)
{
    struct stat fileTarget;
    if(stat(inputPath, &fileTarget) == -1)
    {
        printf("Error with stat function\n");
        exit(-1);
    }
    strtok(inputPath, "/");

    sprintf(stringToFormat, "nume fisier: %s\n"
                             "dimensiune legatura: %ld\n"
                             "dimensiune fisier: %ld\n"
                             "drepturi de acces user: %s\n"
                             "drepturi de acces grup: %s\n"
                             "drepturi de acces altii: %s\n\n",
                             strtok(NULL, "/"), info.st_size, fileTarget.st_size, formatAccessRightsUser(info.st_mode), formatAccessRightsGroup(info.st_mode), formatAccessRightsOthers(info.st_mode));
}

void formatDirectoryOutput(char* stringToFormat, char* inputPath, struct stat info)
{
    strtok(inputPath, "/");

    sprintf(stringToFormat, "nume fisier: %s\n"
                             "identificatorul utilizatorului: %d\n"
                             "drepturi de acces user: %s\n"
                             "drepturi de acces grup: %s\n"
                             "drepturi de acces altii: %s\n\n",
                             strtok(NULL, "/"), info.st_uid, formatAccessRightsUser(info.st_mode), formatAccessRightsGroup(info.st_mode), formatAccessRightsOthers(info.st_mode));
}

int createFile(char* path, int mode, mode_t permissions)
{
    int outputFileDescriptor = open(path, mode, permissions);
    if(outputFileDescriptor == -1)
    {
        printf("Error while creating file\n");
        exit(-1);
    }

    return outputFileDescriptor;
}

void readMyDirectory(DIR* inputDirectory, char* inputDirectoryPath)
{
    struct dirent* direntStruct;
    char* newPath = (char*)malloc(sizeof(char) * FILE_STRING_SIZE);
    
    int outputFileDescriptor = open("statistica.txt", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

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
                    if(isImageBMP(newPath))
                    {
                        int imageWidth = 0;
                        int imageHeight = 0;
                        int fileDescriptor = openFile(newPath, O_RDONLY);
                        getImageWidthAndHeight(fileDescriptor, &imageWidth, &imageHeight);
                        closeFile(fileDescriptor);
                        formatImageOutput(formattedString, newPath, imageWidth, imageHeight, fileInfo);
                    }
                    else
                    {
                        formatRegularFileOutput(formattedString, newPath, fileInfo);
                    }
                }

                if(S_ISLNK(fileInfo.st_mode))
                {
                    formatSymbolicLinkOutput(formattedString, newPath, fileInfo);
                }

                if(S_ISDIR(fileInfo.st_mode))
                {
                    formatDirectoryOutput(formattedString, newPath, fileInfo);
                }

                if(writeFile(outputFileDescriptor, formattedString))
                {
                    printf("Content was written to the file\n");
                }
                free(formattedString);
            }
            else
            {
                printf("Error with lstat function\n");
                exit(-1);
            }
        }
    }

    closeFile(outputFileDescriptor);
}

int main(int argc, char* argv[])
{
    struct stat info;

    //check number of arguments
    if(argc != 2)
    {
        printf("Usage: %s <nume_fisier>", argv[0]);
        exit(-1);
    }
    
    //save path in a variable
    char* inputPath = (char*)malloc(sizeof(char)*SIZE_PATH);
    strcpy(inputPath, argv[1]);


    //check user input
    if(stat(inputPath, &info) == -1)
    {
        printf("Error with stat function\n");
        exit(-1);
    }

    if(!S_ISDIR(info.st_mode))
    {
        printf("The given argument is not a directory!\n");
        exit(-1);
    }

    DIR* inputDirectory = openDirectory(inputPath);

    readMyDirectory(inputDirectory, inputPath);

    closeDirectory(inputDirectory); 

    free(inputPath);

    return 0;
}