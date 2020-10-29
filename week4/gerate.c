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
char *generateName(char first[][10],char middle[][10]){
    char *rs = (char *)malloc(20*sizeof(char));
    memset(rs,'\0',sizeof(char)*20);
    int f,m;
    f = rand() % 12;
    m = rand() % 12;
    strcpy(rs,first[f]);
    strcat(rs,middle[m]);
    rs[strlen(rs)] = (char) randomz(65,65+7);
    return rs;
}
long long generatePhoneNum(){
    return randomz(100000,10000000000);
}

int main(){
    char first_name[12][10]= {"Tran_","Nguyen_","Trinh_","Ta_","Dao_","Hoang_","Le_","Ly_","Vu_","Phan_","Dang_","Ngo_"};
    char mid[12][10] = {"Van_","Ha_","Nguyet_","Nhat_","Bao_","Bach_","Bich_","Chi_","Diem_","Dieu_","Gia_","Ha_"};
    FILE *fout = fopen("phonebook.txt","w+");
    srand((int)time(0));
    long long r;
    for(int i = 0; i < 1000; ++i){
        char str[13];
        memset(str,'\0',sizeof(char)*13);
        r = randomz(90000000,100000000-1);
        //printf("%lld ",r);
        char sdt[13];
        memset(sdt,'\0',sizeof(char)*13);
        myitoa(r,str);
        strcpy( sdt,"+849");
        strcat( sdt,str );
        char *name = generateName(first_name,mid);
        //printf("%s \n",name);
        //printf("%lld \n",balance);
        fprintf(fout,"%s %s\n",name,sdt);
    }
    fclose(fout);
}