#include<stdio.h>
#include<string.h>
#include "data.h"
#include "function.h"

int main(){
    PhoneBook newBook = createPhoneBook();
    char name[80];
    long number;
    FILE *input = fopen("text.txt","r");
    while(fscanf(input,"%s %d\n",name,&number) == 2){
        addPhoneNumber(name,number,&newBook);
    }
    printPhoneBook(newBook);


    // Search Entry
    // printf("Enter the name u want to search\n");
    // scanf("%s",name);
    // PhoneEntry *foundEntry = getPhoneNumber(name,newBook);
    // if(foundEntry != NULL){
    //     printf("%s %ld\n",foundEntry->name,foundEntry->number);
    // }else{
    //     printf("Not Found\n");
    // }
}

