#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>


long long randomz(long long minN, long long maxN){
    return minN + rand() % (maxN + 1 - minN);
}

char* myitoa(long long i, char b[]){
    char const digit[] = "0123456789";
    char* p = b;
    if(i<0){
        *p++ = '-';
        i *= -1;
    }
    int shifter = i;
    do{ //Move to where representation ends
        ++p;
        shifter = shifter/10;
    }while(shifter);
    *p = '\0';
    do{ //Move back, inserting digits as u go
        *--p = digit[i%10];
        i = i/10;
    }while(i);
    return b;
}
char *generateName(){
    char *rs = (char*) malloc(sizeof(char)*4);
    memset(rs,'\0',sizeof(char)*4);
    int num = (rand() % (90 - 65 + 1)) + 65;
    printf("%d\n",num);
    rs[0] = (char) num;
    do{
        num = (rand() % (90 -65 + 1)) +65;
    }while((int)rs[0] == (int) num);
    printf("%d\n",num);
    rs[1] = ' ';
    rs[2] = (char) num;
    return rs;
}
long long generatePhoneNum(){
    return randomz(100000,10000000000);
}

int main(){
    //char s[10][10]= {"M1","M2","M3","M4","M5","M6","M7","M8","M9","M10"};
    //char m[10][10] = {"S1","S2","S3","S4","S5","S6","S7","S8","S9","S10"};
    FILE *fout = fopen("text.txt","w+");
    srand((int)time(0));

    for(int i = 0; i < 50; ++i){
        fprintf(fout,"%s",generateName());
        if(i != 49) fputs("\n",fout);
    }
    fclose(fout);
}



