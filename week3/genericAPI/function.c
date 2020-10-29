#ifndef __Function1__
#define __Function1__

#include "function.h"
#include "data.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


SymbolTable createSymbolTable(Entry (*makeNode)(void*,void*), int (*compare)(void*, void*)){
    SymbolTable newPhoneBook;
    newPhoneBook.total = 0;
    newPhoneBook.size = INITIAL_SIZE;
    newPhoneBook.makeNode = makeNode;
    newPhoneBook.compare = compare;
    newPhoneBook.entries = (Entry *) malloc(sizeof(Entry) * INITIAL_SIZE);
    return newPhoneBook;
}

void dropSymbolTable(SymbolTable *tab){
    for(int i = 0 ; i < tab->total ; i++){
        free((tab + i));
    }
    tab->total = 0;
    tab->size = INITIAL_SIZE;
}

int indexNewEntry(Entry *newEntry,SymbolTable *tab){

    for(int i = 0 ; i < tab->total ; i++){
        if( compareEntry(newEntry->key,(tab->entries + i)->key) < 0){
            return i;
        }
    }
    return tab->total;
}

void printPhoneBook(SymbolTable tab){

    for(int i = 0 ; i < tab.total ; i++){
        printf("%s %ld\n",(char*)(tab.entries[i]).key,*(long*)(tab.entries[i]).value);
    }
    printf("\n");
}

void addEntry(void *key,void *value, SymbolTable *tab){

    Entry *checkEntry = getEntry(key,*(tab));
    
    if(checkEntry != NULL){
       memcpy(checkEntry->value,value,sizeof(long));
       return;
    }

    Entry newEntry =  tab->makeNode(key,(long*)value);
    if(tab->total + 1 > tab->size){
        tab->entries = (Entry *) realloc(tab->entries,sizeof(Entry) * (tab->total + INCREMENTAL_SIZE));
        int newIndex = indexNewEntry(&newEntry,tab);
        for(int i = tab->total - 1 ; i >= newIndex ; i--){
            memcpy(tab->entries + i + 1,tab->entries + i,sizeof(Entry));
        }
        memcpy(tab->entries + newIndex,&newEntry,sizeof(Entry));
        tab->total += 1;
        tab->size += INCREMENTAL_SIZE;
    }else{
        int newIndex = indexNewEntry(&newEntry,tab);
        for(int i = tab->total-1 ; i >= newIndex ; i--){
            memcpy(tab->entries + i + 1,tab->entries + i,sizeof(Entry));
        }
        tab->entries[newIndex] = newEntry;
        tab->total +=1;
    }
}

Entry *getEntry(void *key, SymbolTable tab){
    for(int i = 0 ; i < tab.total ; i++){
        if(tab.compare(key,tab.entries[i].key) == 0){
            return &tab.entries[i];
        }
    }
    return NULL;
}

// PhoneEntry *getPhoneNumber(char *name,PhoneBook book){
    
// }

#endif