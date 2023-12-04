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
#define OFFSET_DATA 10
#define OFFSET_BITCOUNT 28
#define OFFSET_COLORS_USED 46
#define RDWR_BUFFER 4096
#define HEIGHT_FIELD_SIZE 4
#define WIDTH_FIELD_SIZE 4
#define FORMATTED_STRING_SIZE 500
#define TIME_STRING_SIZE 11
#define RIGHTS_STRING_SIZE 4
#define WRITE_BUFFER 100
#define FILE_STRING_SIZE 50
#define INPUT_CHAR_SIZE 2
#define OUTPUT_VALID 2

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

void checkDirectory(char* path)
{
    struct stat info;

    if(stat(path, &info) == -1)
    {
        printf("Error with stat function\n");
        exit(-1);
    }

    if(!S_ISDIR(info.st_mode))
    {
        printf("The given argument is not a directory!\n");
        exit(-1);
    }
}

void getImageWidthAndHeight(int fileDescriptor, int* width, int* height) 
{
    if(lseek(fileDescriptor, OFFSET_WIDTH, SEEK_SET) == -1)
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
    int totalSize = strlen(content);
    int countLines = 0;

    for(int i = 0; i<totalSize; i++)
    {
        if(content[i] == '\n')
        {
            countLines++;
        }
    }

    countLines = countLines - 1;

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
    {
        //printf("in write %d\n", countLines);
        return countLines;
    }
    
    return 0;
}

void formatImageOutput(char* stringToFormat, char* fileName, int width, int height, struct stat info)
{
    char* timeString = (char*)malloc(sizeof(char)*TIME_STRING_SIZE);
    formatTime(info.st_mtim, timeString);

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
                             fileName, height, width, info.st_size, info.st_uid, timeString, info.st_nlink, formatAccessRightsUser(info.st_mode), formatAccessRightsGroup(info.st_mode), formatAccessRightsOthers(info.st_mode));

    free(timeString);
}

void formatRegularFileOutput(char* stringToFormat, char* fileName, struct stat info)
{
    char* timeString = (char*)malloc(sizeof(char)*TIME_STRING_SIZE);
    formatTime(info.st_mtim, timeString);

    sprintf(stringToFormat, "nume fisier: %s\n"
                             "dimensiune: %ld\n"
                             "identificatorul utilizatorului: %d\n"
                             "timpul ultimei modificari: %s\n"
                             "contorul de legaturi: %ld\n"
                             "drepturi de acces user: %s\n"
                             "drepturi de acces grup: %s\n"
                             "drepturi de acces altii: %s\n\n",
                             fileName, info.st_size, info.st_uid, timeString, info.st_nlink, formatAccessRightsUser(info.st_mode), formatAccessRightsGroup(info.st_mode), formatAccessRightsOthers(info.st_mode));

    free(timeString);
}

void formatSymbolicLinkOutput(char* stringToFormat, char* inputPath, char* fileName, struct stat info)
{
    struct stat fileTarget;
    if(stat(inputPath, &fileTarget) == -1)
    {
        printf("Error with stat function\n");
        exit(-1);
    }

    sprintf(stringToFormat, "nume fisier: %s\n"
                             "dimensiune legatura: %ld\n"
                             "dimensiune fisier: %ld\n"
                             "drepturi de acces user: %s\n"
                             "drepturi de acces grup: %s\n"
                             "drepturi de acces altii: %s\n\n",
                             fileName, info.st_size, fileTarget.st_size, formatAccessRightsUser(info.st_mode), formatAccessRightsGroup(info.st_mode), formatAccessRightsOthers(info.st_mode));
}

void formatDirectoryOutput(char* stringToFormat, char* fileName, struct stat info)
{

    sprintf(stringToFormat, "nume fisier: %s\n"
                             "identificatorul utilizatorului: %d\n"
                             "drepturi de acces user: %s\n"
                             "drepturi de acces grup: %s\n"
                             "drepturi de acces altii: %s\n\n",
                             fileName, info.st_uid, formatAccessRightsUser(info.st_mode), formatAccessRightsGroup(info.st_mode), formatAccessRightsOthers(info.st_mode));
}

int createFile(char* outputDirectoryPath, char* fileName)
{
    char* path = (char*)malloc(sizeof(char)*SIZE_PATH);
    sprintf(path, "%s/%s_statistica.txt", outputDirectoryPath, fileName);

    int fileDescriptor = open(path, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(fileDescriptor == -1)
    {
        printf("Error with creating the file\n");
        exit(-1);
    }

    free(path);
    return fileDescriptor;
}

void createWriteProcess(char* outputDirectoryPath, char* fileName, char* content)
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
        int outputFileDescriptor = createFile(outputDirectoryPath, fileName);

        totalLines = writeFile(outputFileDescriptor, content);

        closeFile(outputFileDescriptor);
        exit(totalLines);
    }
}

void readImagePixels(unsigned char* buffer, int fileSize, int fileDescriptor)
{
    ssize_t readBytes = 0;
    size_t remainingBytes = fileSize;
    unsigned char* currentBufferPos = buffer;

    if(lseek(fileDescriptor, OFFSET_RGB, SEEK_SET) == -1)
    {
        printf("Error while setting the cursor\n");
        exit(-1);
    }

     while(remainingBytes > 0)
    {
        int bytesToRead = (remainingBytes < RDWR_BUFFER) ? remainingBytes : RDWR_BUFFER;

        if((readBytes = read(fileDescriptor, currentBufferPos, bytesToRead)) == -1)
        {
            printf("Error while reading\n");
            exit(-1);
        }

        currentBufferPos += readBytes;
        remainingBytes -= readBytes;
    }
}

void writeImagePixels(unsigned char* buffer, int fileSize, int fileDescriptor)
{
    ssize_t writtenBytes = 0;
    size_t remainingBytes = fileSize;
    unsigned char* currentBufferPos = buffer;

    if(lseek(fileDescriptor, OFFSET_RGB, SEEK_SET) == -1)
    {
        printf("Error while setting the cursor\n");
        exit(-1);
    }

    while(remainingBytes > 0)
    {
        int bytesToWrite = (remainingBytes < RDWR_BUFFER) ? remainingBytes : RDWR_BUFFER;

        if((writtenBytes = write(fileDescriptor, currentBufferPos, bytesToWrite)) == -1)
        {
            printf("Error while writing\n");
            exit(-1);
        }

        currentBufferPos += writtenBytes;
        remainingBytes -= writtenBytes;
    }
}

void convert8BitToGrayScale(int fileDescriptor, int colorUsed, int imageDataOffset, int height, int width)
{
    unsigned char colorTable[colorUsed*4];
    if(read(fileDescriptor, colorTable, colorUsed*4) == -1)
    {
        printf("Error while reading\n");
        exit(-1);
    }

    unsigned char pixel;
    int fileSize = height * width;

    if(lseek(fileDescriptor, imageDataOffset, SEEK_SET) == -1)
    {
        printf("Error while setting the cursor\n");
        exit(-1);
    }

    for(int i = 0; i<fileSize; i++)
    {
        if(read(fileDescriptor, &pixel, sizeof(unsigned char)) == -1)
        {
            printf("Error while reading\n");
            exit(-1);
        }

        unsigned char grey = (unsigned char)((colorTable[pixel * 4] * 0.299 + colorTable[pixel * 4 + 1] * 0.587 + colorTable[pixel * 4 + 2]) * 0.114);
                                              
        if(lseek(fileDescriptor, -1, SEEK_CUR) == -1)
        {
            printf("Error while setting the cursor\n");
            exit(-1);
        }

        if(write(fileDescriptor, &grey, sizeof(unsigned char)) == -1)
        {
            printf("Error while writting\n");
            exit(-1);
        }                      
    }
}

int createPhotoProcess(char* path, int heigth, int width)
{
    int pid = fork();
    if(pid < 0)
    {
        printf("Error with process\n");
        exit(-1);
    }
    else if(pid == 0)
    {
        int fileDescriptor = openFile(path, O_RDWR);

        short int bitCount = 0;
        if(lseek(fileDescriptor, OFFSET_BITCOUNT, SEEK_SET) == -1)
        {
            printf("Error while setting the cursor\n");
            exit(-1);
        }
        if(read(fileDescriptor, &bitCount, 2) == -1)
        {
            printf("Error while reading\n");
            exit(-1);
        }

        if(bitCount <= 8)
        {
            int colorUsed = 0;
            if(lseek(fileDescriptor, OFFSET_COLORS_USED, SEEK_SET) == -1)
            {
                printf("Error while setting the cursor\n");
                exit(-1);
            }
            if(read(fileDescriptor, &colorUsed, 4) == -1)
            {
                printf("Error while reading\n");
                exit(-1);
            }

            int imageDataOffset = 0;
            if(lseek(fileDescriptor, OFFSET_DATA, SEEK_SET) == -1)
            {
                printf("Error while setting the cursor\n");
                exit(-1);
            }
            if(read(fileDescriptor, &imageDataOffset, 4) == -1)
            {
                printf("Error while reading\n");
                exit(-1);
            }

            convert8BitToGrayScale(fileDescriptor, colorUsed, imageDataOffset, heigth, width);
        }
        else
        {
            int fileSize = heigth*width*3;
            unsigned char* buffer = (unsigned char*)malloc(fileSize);

            readImagePixels(buffer, fileSize, fileDescriptor);


            for (int i = 0; i < fileSize; i += 3) {
                unsigned char grey = 0.299 * buffer[i + 2] + 0.587 * buffer[i + 1] + 0.114 * buffer[i];
                buffer[i] = buffer[i + 1] = buffer[i + 2] = grey;
            }

            writeImagePixels(buffer, fileSize, fileDescriptor);

            free(buffer);
        }

        closeFile(fileDescriptor);

        exit(0);
    }
    else
    {
        return pid;
    }
}

void createWriteRegularFileProcess(char* outputDirectoryPath, char* fileName, char* content, int* pfdBrothers, int* pfdLines, char* path)
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
        close(pfdLines[0]);

        int outputFileDescriptor = createFile(outputDirectoryPath, fileName);

        totalLines = writeFile(outputFileDescriptor, content);
        write(pfdLines[1], &totalLines, 4);

        closeFile(outputFileDescriptor);

        dup2(pfdBrothers[1], 1);

       execlp("cat", "cat", path, NULL);
       printf("Error with cat exec\n");
       exit(-1);
    }
}

int createCountValidSEntencesProcess(int* pfdBrothers, int* pfdParentChild, char* inputCharacter)
{
    int pid = fork();
    if(pid < 0)
    {
        printf("Error with fork\n");
        exit(-1);
    }
    else if(pid == 0)
    {
        close(pfdBrothers[1]);
        close(pfdParentChild[0]);

        dup2(pfdBrothers[0], 0);
        dup2(pfdParentChild[1], 1);

        execlp("/bin/bash", "bash", "script.sh", inputCharacter, NULL);
        printf("Error with script exec\n");
       exit(-1);
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
    int photoPid = 0;
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
                    if(isImageBMP(newPath))
                    {
                        int imageWidth = 0;
                        int imageHeight = 0;

                        int fileDescriptor = openFile(newPath, O_RDONLY);
                        getImageWidthAndHeight(fileDescriptor, &imageWidth, &imageHeight);
                        closeFile(fileDescriptor);

                        formatImageOutput(formattedString, direntStruct->d_name, imageWidth, imageHeight, fileInfo);  
                        
                        createWriteProcess(outputDirectoryPath, direntStruct->d_name, formattedString);

                        photoPid = createPhotoProcess(newPath, imageHeight, imageWidth);
                        
                        childPidCount+=2;
                    }
                    else
                    {
                        int pfdBrothers[2];
                        int pfdParentChild[2];
                        int pfdLines[2];

                        formatRegularFileOutput(formattedString, direntStruct->d_name, fileInfo);

                        if(pipe(pfdBrothers) < 0 || pipe(pfdParentChild) < 0 || pipe(pfdLines) < 0)
                        {
                            printf("Error with pipe creation\n");
                            exit(-1);
                        }
                        
                        createWriteRegularFileProcess(outputDirectoryPath, direntStruct->d_name, formattedString, pfdBrothers, pfdLines, newPath);
                        
                        close(pfdBrothers[1]);
                        close(pfdLines[1]);

                        validPid = createCountValidSEntencesProcess(pfdBrothers, pfdParentChild, inputCharacter);

                        close(pfdBrothers[0]);
                        close(pfdParentChild[1]);
                        close(pfdLines[1]);

                        int totalLines = 0;
                        read(pfdLines[0], &totalLines, 4);
                        printf("Total lines written: %d\n", totalLines);

                        close(pfdLines[0]);

                        char* string = (char*)malloc(sizeof(char)*OUTPUT_VALID);
                        read(pfdParentChild[0], string, OUTPUT_VALID);

                        close(pfdParentChild[0]);
                        
                        int validForOneFile = atoi(string);
                        countCorrectSentence += validForOneFile;

                        childPidCount+=2;
   
                    }
                }

                if(S_ISLNK(fileInfo.st_mode))
                {
                    formatSymbolicLinkOutput(formattedString, newPath, direntStruct->d_name, fileInfo);
                    
                    createWriteProcess(outputDirectoryPath, direntStruct->d_name, formattedString);
                    childPidCount++;
                }

                if(S_ISDIR(fileInfo.st_mode))
                {
                    formatDirectoryOutput(formattedString, direntStruct->d_name, fileInfo);
                    
                    createWriteProcess(outputDirectoryPath, direntStruct->d_name, formattedString);
                    childPidCount++;
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

    for(int i = 0; i<childPidCount; i++)
    {
        int status;
        int pidDone = wait(&status);
        if(pidDone == -1)
        {
            printf("Error in wait function\n");
            exit(-1);
        }

        if(WIFEXITED(status) && WEXITSTATUS(status) > 0)
        {
            printf("Process with id %d exited with success, number of lines written: %d\n", pidDone, WEXITSTATUS(status));
        }
        else if(pidDone == photoPid && WIFEXITED(status))
        {
            printf("Process with id %d exited with status %d\n", pidDone, WEXITSTATUS(status));
        }
        else if (pidDone == validPid && WIFEXITED(status))
        {
            printf("Process with id %d exited with status %d\n", pidDone, WEXITSTATUS(status));
        }
        else
        {
            printf("Process with id %d exited with status %d\n", pidDone, WEXITSTATUS(status));
        }
    }

    printf("Total valid sentences in text files: %d\n", countCorrectSentence);
}

int main(int argc, char* argv[])
{
    //check number of arguments
    if(argc != 4)
    {
        printf("Usage: %s <nume_director_intrare> <nume_director_iesire> <c>\n", argv[0]);
        exit(-1);
    }

    char* inputCharacter = (char*)malloc(sizeof(char)*INPUT_CHAR_SIZE);
    strcpy(inputCharacter, argv[3]);
    
    //save path in a variable
    char* inputPath = (char*)malloc(sizeof(char)*SIZE_PATH);
    strcpy(inputPath, argv[1]);
    char* outputPath = (char*)malloc(sizeof(char)*SIZE_PATH);
    strcpy(outputPath, argv[2]);

    checkDirectory(inputPath);
    checkDirectory(outputPath);

    DIR* inputDirectory = openDirectory(inputPath);

    readMyDirectory(inputDirectory, inputPath, outputPath, inputCharacter);

    closeDirectory(inputDirectory); 

    free(inputPath);

    return 0;
}