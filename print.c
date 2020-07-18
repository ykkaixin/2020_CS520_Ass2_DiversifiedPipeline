#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "fileprocessor/fileProcess.h"

int main() {
    // char *buffer = (char*)malloc(100);
	// strcpy(buffer,"/home/kai/Desktop/ass1/test01.asm");
    // openFile(buffer);
    // char *d;
    // while (1)
    // {
    //     d = poll();
    //     printf("adssdaasd%s\n",d );
    // }
    

    char c[123] = "add";
    if (!strcmp(c, "1"))
    {
        printf("hello");
    } else
    {
        printf("no");
    }
    int result = strcmp(c, "add");
    printf("%d\n", result);
    
    
    // SimpleSet set;
    // set_init(&set);
    // int a = 30;
    // char str[10];
    // sprintf(str, "%d", a);
    // set_add(&set, str);
    // if(set_contains(&set,"30") == SET_TRUE) {
    //     printf("h");
    // }


    

    return 0;
}