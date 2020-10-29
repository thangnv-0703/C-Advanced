#ifndef __Function__
#define __Function__
#include "data.h"

PhoneBook createPhoneBook();
void dropPhoneBook(PhoneBook *book);
void addPhoneNumber(char *name,long number, PhoneBook *book);
void printPhoneBook(PhoneBook book);
PhoneEntry *getPhoneNumber(char *name,PhoneBook book);
#endif