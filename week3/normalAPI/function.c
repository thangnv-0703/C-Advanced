#ifndef __Function1__
#define __Function1__

#include "function.h"
#include "data.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

PhoneBook createPhoneBook(){
    PhoneBook newPhoneBook;
    newPhoneBook.total = INITIAL_SIZE;
    newPhoneBook.size = 0;
    newPhoneBook.entries = (PhoneEntry *) malloc(sizeof(PhoneEntry) * newPhoneBook.total);
    return newPhoneBook;
}

void dropPhoneBook(PhoneBook *book){
    for(int i = 0 ; i < book->total ; i++){
        free((book->entries + i));
    }
    book->size = 0;
    book->total = INITIAL_SIZE;
}

void addPhoneNumber(char *name,long number, PhoneBook *book){
    PhoneEntry newPhone;
    strcpy(newPhone.name,name);
    newPhone.number = number;
    //printf("%s %d\n",newPhone.name,newPhone.number);
    if(book->size + 1 > book->total){
        book->entries = (PhoneEntry *) realloc(book->entries,sizeof(PhoneEntry) * (book->total + INCREMENTAL_SIZE));
        book->total += INCREMENTAL_SIZE;
        *(book->entries + book->size) = newPhone;
        book->size += 1;
    }else{
        *(book->entries + book->size) = newPhone;
        book->size += 1;
    }
}

void printPhoneBook(PhoneBook book){
    for(int i = 0 ; i < book.size ; i++){
        printf("%s %ld\n", (book.entries + i)->name, (book.entries + i)->number);
    }
}

PhoneEntry *getPhoneNumber(char *name,PhoneBook book){
    for(int i = 0 ; i < book.size ; i++){
        if(strcmp(name,(book.entries + i)->name) == 0)
            return (book.entries + i);
    }
    return NULL;
}

#endif