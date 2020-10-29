#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
void skipTab(){
    for(int i = 0 ; i < 7 ; i++)
        printf("\b");
}
int checkTabLine(char text[]){
    for(int i = 0 ; i < strlen(text) ; i++)
        if(text[i] == '\t')
            skipTab();
    return 0;
}
int main(void)
{
  
}