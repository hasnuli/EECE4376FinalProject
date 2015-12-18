#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#define FILE_NAME "file.bin"


int *playback()
{
    // Declare variables needed for
    unsigned char* buffer;
    FILE *fp;
    unsigned long fileLen;
    int i;
    
    // Make sure the file is created and accessible
    if ((fp = fopen(FILE_NAME, "rb")) == NULL)
    {
        printf("Error opening file\n");
        exit(1);
    }
    
    // Get the file length for playback
    fseek(fp, 0, SEEK_CUR);
    fileLen = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    
    // Allocate memory into the buffer
    buffer = (unsigned char *) malloc(fileLen + 1);
    if (!buffer)
    {
        fprintf(stderr, "Memory error!");
        fclose(fp);
        return 0;
    }
    
    // Read file contents into buffer
    fread(buffer, fileLen, 1, fp);
    fclose(fp);
    
    // Run through the file and play the music
    for (i = 0; i < fileLen; ++i)
    {
        if (buffer[i] == '1')
            pin_high(9, 28);
        else
            pin_low(9, 28);
        
        nanosleep((const struct timespec[]){{0, 651.041667L}}, NULL);
    }
    
    return 0;
}

