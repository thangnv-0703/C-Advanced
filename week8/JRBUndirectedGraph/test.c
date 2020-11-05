#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int main(){
    char **s;
    s = (char**) malloc(sizeof(char)*10);
    for(int i = 0 ; i < 10 ; i++) *(s + i) = (char *) malloc(sizeof(char)*5);
    strcpy(*(s),"123");
    printf("%s\n",*(s+0));
}