#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SMALL_NUMBER 10000000
void exch(long buf[],long i , long j){
    long temp = buf[i];
    buf[i] = buf[j];
    buf[j] = temp;
}
long *createArray(long size){
    long *res = (long*) malloc(sizeof(long) * size);
    srand(time(NULL));
    for(long i  = 0  ; i < size ; i++){
        long j = (rand() % 100) + 1;
        res[i] = j;
    }
    return res;
}

void printArray(long arr[],long size){
    for(long i = 0 ; i < size ; i++) printf("%d ", arr[i]);
    printf("\n");
}

long partition(long arr[],long l , long r){
    long pivot = arr[r];
    long i = (l - 1);
    for(long j = l ; j <= r-1 ; j++){
        if(arr[j] < pivot)
            exch(arr,j,++i);
    }
    exch(arr,i+1,r);
    return (i+1);
}

void sort(long arr[],long l,long r){
    if(l < r){
        long pi = partition(arr,l,r);
        sort(arr,l,pi-1);
        sort(arr,pi+1,r);
    }
}

long main(){
    long *a1 = createArray(SMALL_NUMBER);
    sort(a1,0,SMALL_NUMBER-1);
    printArray(a1,SMALL_NUMBER);
}