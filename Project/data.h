#ifndef __DATA__
#define __DATA__

#include "./Libfdr/jrb.h"
#include "./Libfdr/jval.h"

#define MAXSIZE 14200

typedef struct{
    int id;
    char name[100];
    float cost;
}Airport;

typedef struct{
    JRB vertices;
    JRB edges;
} Graph;

#endif
