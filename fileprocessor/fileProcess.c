#include "fileProcess.h"
#include<stdio.h>
#include <stdlib.h>

FILE *fp;
char str[100];

void openFile(char *filePath) {
    fp = fopen(filePath, "r");
    if(fp == NULL) {
        printf("\n");
        printf("Error opening file, input correct file path");
        exit(1);
    }

}

char *poll()
{
    if(fgets(str, 100 , fp) != NULL) {
        return str;
    }
    return NULL;
}

void close() {
    fclose(fp);
}
