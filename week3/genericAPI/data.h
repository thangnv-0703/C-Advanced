#ifndef __Data__
#define __Data__

#define INITIAL_SIZE 100
#define INCREMENTAL_SIZE 10



typedef struct{
    void *key;
    void *value;
} Entry;

typedef struct{
    Entry *entries;
    int total;
    int size;
    Entry (*makeNode)(void*,void*);
    int (*compare)(void*, void*);
} SymbolTable;

#endif