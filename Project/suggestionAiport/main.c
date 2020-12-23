#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
int main(){
    FILE *ptr = fopen("Book1.csv","r+");
    FILE *out = fopen("out.dat","w+");
    while(!feof(ptr)){
        int c = fgetc(ptr);
        if(isprint(c) || c =='\n') fputc(c,out);
    }
    fclose(ptr);
    fclose(out);
}