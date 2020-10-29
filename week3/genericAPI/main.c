#include<stdio.h>
#include<string.h>
#include <stdlib.h>
#include "data.h"
#include "function.h"

Entry makePhone(void *name,void *number){
    Entry newPhone;
    newPhone.key = strdup((char*)name);
    newPhone.value = (long *) malloc(sizeof(long));
    memcpy(newPhone.value,number,sizeof(long));
    return newPhone;
}

int compareEntry(void *key1,void *key2){
    return strcmp( (char*) key1,(char*) key2);
}



int main(){
    
    SymbolTable newBook = createSymbolTable(makePhone,compareEntry);
    char name[80];
    long number;
    FILE *input = fopen("text.txt","r");
    while(fscanf(input,"%s %ld\n",name,&number) == 2){
        addEntry(name,&number,&newBook);
    }
    printPhoneBook(newBook);
    char name1 [] = "Nam";
    Entry *found = getEntry(name1,newBook);

    if(found != NULL){
        printf("\n%s %ld\n",(char*)found->key,*(long*) found->value);
    }

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

