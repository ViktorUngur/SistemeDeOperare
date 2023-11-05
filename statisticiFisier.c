#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

#define PATH_LENGTH 20
#define STRING_LENGTH 100

int main(int argc, char* argv[]) {

    char* pathInput = (char*)malloc(sizeof(char)*PATH_LENGTH);
    char* pathOutput = (char*)malloc(sizeof(char)*PATH_LENGTH);
    char* formatedString = (char*)malloc(sizeof(char)*STRING_LENGTH);
    struct stat st;
    char myChar;
    int countLowerCase = 0;
    int countUpperCase = 0;
    int countDigit = 0;
    int countMyChar = 0;
    int dimensiune = 0;
    char charFromFile = 0;

    if(argc < 3) {
        perror("Lipsesc argumente!\n");
        exit(-1);
    }

    strcpy(pathInput, argv[1]);
    strcpy(pathOutput, argv[2]);
    myChar = argv[3][0];
    int fileDescriptorInput = open(pathInput, O_RDONLY);

    if(fileDescriptorInput == -1) 
    {
        perror("Eroare la deschiderea fisierului\n");
        exit(-1);
    }

    while(read(fileDescriptorInput, &charFromFile, 1)) 
    {
        if(isupper(charFromFile))
            countUpperCase++;

        if(islower(charFromFile))
            countLowerCase++;

        if(isdigit(charFromFile))
            countDigit++;

        if(charFromFile == myChar) 
            countMyChar++;
    }

    if(stat(pathInput, &st) == -1) 
    {
        perror("Eroare la functia stat");
        exit(-1);
    }

    dimensiune = st.st_size;

    if(close(fileDescriptorInput) == -1) 
    {
        perror("Eroare la inchiderea fisierului\n");
        exit(-1);
    }

    sprintf(formatedString, "numar litere mici: %d\nnumar litere mari: %d\nnumar cifre: %d\nnumar aparitii caracter: %d\ndimensiune fisier: %d", countLowerCase, countUpperCase, countDigit, countMyChar, dimensiune);


    int fileDescriptorOutput = open(pathOutput, O_WRONLY);
    if(fileDescriptorInput == -1)
    {
        perror("Eroare la deschiderea fisierului");
        exit(-1);
    }

    if(write(fileDescriptorOutput, formatedString, STRING_LENGTH) == -1) 
    {
        perror("Eroare la scriere\n");
        exit(-1);
    }

    if(close(fileDescriptorOutput) == -1) 
    {
        perror("Eroare la inchiderea fisierului\n");
        exit(-1);
    }
    return 0;
}