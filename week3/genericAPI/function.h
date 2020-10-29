#ifndef __Function__
#define __Function__
#include "data.h"

SymbolTable createSymbolTable(Entry (*makeNode)(void*,void*),int (*compare)(void*, void*));
void dropSymbolTable(SymbolTable *tab);
void addEntry(void *key,void *value, SymbolTable *tab);
Entry *getEntry(void *key, SymbolTable tab);
Entry makePhone(void *key,void *value);
int compareEntry(void *key1,void *key2);
void printPhoneBook(SymbolTable tab);
//PhoneEntry *getPhoneNumber(char *name,PhoneBook book);

#endif