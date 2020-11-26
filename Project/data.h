#ifndef __DATA__
#define __DATA__

struct Flight{
    int id;
    char name[30];
    int priceNormal;
    int priceEconomic;
};

struct Airport{
    int id;
    char name[30];
    struct Flight *flights;
};


#endif