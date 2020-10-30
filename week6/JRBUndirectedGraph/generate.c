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
char *generateName(int k,char first[][10],char middle[][10]){
    char *rs = (char *)malloc(50*sizeof(char));
    memset(rs,'\0',sizeof(char)*50);
    
    int arr[10];
    memset(arr,0,sizeof(int)*10);
    strcpy(rs,first[k]);
    strcat(rs," ");
    for(int i = 0 ; i < 10 ; i++){
        int m;
        m = rand() % 10;
        while(arr[m] == 1){
            m = rand() % 10;
        }
        strcat(rs,middle[m]);
        if (i != 9) strcat(rs," ");
        arr[m] = 1;
        for(int i = 0 ; i < 10 ; i++) printf("%d ",arr[i]);
        printf("\n");
    }
    //rs[strlen(rs)] = (char) randomz(65,65+7);
    return rs;
}
long long generatePhoneNum(){
    return randomz(100000,10000000000);
}

int main(){
    char s[10][10]= {"M1","M2","M3","M4","M5","M6","M7","M8","M9","M10"};
    char m[10][10] = {"S1","S2","S3","S4","S5","S6","S7","S8","S9","S10"};
    FILE *fout = fopen("station.txt","w+");
    srand((int)time(0));

    for(int i = 0; i < 10; ++i){
        char *name = generateName(i,s,m);
        if(i != 9) fprintf(fout,"%s\n",name);
        else fprintf(fout,"%s",name);
    }
    fclose(fout);
}