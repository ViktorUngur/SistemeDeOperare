#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[])
{
    if(strcmp(argv[1], "6.2") == 0)
    {
        printf("Viktor\n");
        printf("Cristi\n");
        printf("Ion\n");
    }
    else if(strcmp(argv[1], "6.1") == 0)
    {
        printf("Adrian\n");
        printf("Andra\n");
    }
    return 0;
}