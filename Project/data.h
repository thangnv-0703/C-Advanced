#ifndef __DATA__
#define __DATA__

#include "./Libfdr/jrb.h"
#include "./Libfdr/jval.h"

typedef struct{
    int id;
    char name[30];
    int cost;
}Airport;

typedef struct{
    JRB vertices;
    JRB edges;
} Graph;

#endif