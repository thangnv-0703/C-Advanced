//Pham Thanh Dat
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "../Libfdr/jval.h"

void output(Jval buf[],size_t size, int n){
    int i;
    for (i=0; i<n; i++)
        if (size == sizeof(int)){
            printf("%d ",buf[i].i);
        }
        else if (size == sizeof(long)){
            printf("%ld ",buf[i]);
        }
    printf("\n");
}

int search(
           Jval *buf,
           int size_of_item,
           int l, int r,
           Jval *item,
           int (*compare)(Jval*,Jval*)
           ){
    int mid,res;
    if (r<l) return -1;
    mid=(l+r)/2;
    res=compare(item,&buf[mid]);
    if (res==0)
        return mid;
    else if (res<0)
        return search(buf, size_of_item, l, mid-1, item, compare);
    else
        return search(buf, size_of_item, mid+1, r, item, compare);
}

int int_compare(Jval *x,Jval *y){
    Jval *m,*n;
    m=x;
    n=y;
    if (m->i==n->c) return 0;
    return m->i>n->i ? 1:-1;
}

void int_swap(Jval buf[],size_t size,int i,int j){
    Jval cmp = buf[i];
    buf[i]= buf[j];
    buf[j] = cmp;
}

void qsort_3way(Jval buf[], int l, int r, size_t size, int (*compare)(Jval*,Jval*), void (*swap)(Jval*, size_t, int, int)){
    if(l>=r) return;
    int i=l-1,j=r;
    int p=l-1,q=r;
    
    while(1)
    {
        while(compare( &buf[++i],&buf[r])==-1);
        while(compare(&buf[--j],&buf[r])==1)
        {
            if(j==l) break;
        }
        if(i>=j) break;
        swap(buf,size,i,j);
        if(compare(&buf[i],&buf[r])==0) swap(buf,size,++p,i);
        if(compare(&buf[j],&buf[r])==0) swap(buf,size,--q,j);
    }
    
    swap(buf,size,i,r);
    j=i-1;
    i=i+1;
    
    int k;
    for(k=l;k<=p;k++)
    {
        swap(buf,size,k,j--);
    }
    for(k=r-1;k>=q;k--)
    {
        swap(buf,size,k,i++);
    }
    
    qsort_3way(buf,l,j,size,compare,swap);
    qsort_3way(buf,i,r,size,compare,swap);
}

int main(){
    Jval a[10] = {102, 5, 100, 9, 10, 34, 56, 89, 8, 2};
    // Jval c,b;
    // c = new_jval_i(10);
    // b = new_jval_i(12);
    // printf("%d\n",int_compare(&c,&b));
    output(a, sizeof(long), 10);
    qsort_3way(a, 0, 9, sizeof(long), int_compare, int_swap);
    output(a, sizeof(long), 10);
    
    long res;
    Jval find = new_jval_l(100);
    printf("Finding element: %ld \n", find);
    res = search(a, sizeof(long), 0, 9, &find, int_compare);
    printf("Position found: %ld \n", res);
}
