#ifndef __Data__
#define __Data__

#define INITIAL_SIZE 10
#define INCREMENTAL_SIZE 5



typedef struct{
    char name[80];
    long number;
} PhoneEntry;

typedef struct{
    PhoneEntry *entries;
    int total;
    int size;
}PhoneBook;

#endif