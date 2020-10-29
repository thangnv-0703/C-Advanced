#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
void printArray(int arr[],int size);
#define SMALL_NUMBER 100
#define HUGE_NUMBER 10000000

    void exch(void *buf,void *i , void *j,size_t size){
        
    }
    void swap(void *a,void*b){
        int m = *((int*) a);
        *((int*) a) = *((int*) b);
        *((int*) b) = m;
    }
    void sort(int a[], int l, int r,int (*compare)(const void*,const void*)) {
        if (r <= l) return;
        int i = l-1, j = r;
        int p = l-1, q = r;
        while(1) {
            //printArray(a,SMALL_NUMBER);
            while (compare(&a[++i] , &a[r]) < 0){}
            //printArray(a,SMALL_NUMBER);
            while ( compare(&a[r] , &a[--j]) < 0 ) if (j == l) break;
            if (i >= j) break;
            exch(a, &i, &j,sizeof(int));
            if (compare(&a[i],&a[r]) == 0){
               p++; 
               exch(a, &p, &i,sizeof(int));
            }
            if (compare(&a[j] , &a[r]) == 0){
               q--;
               exch(a, &q, &j,sizeof(int));
            }
        }
        exch(a, &i, &r,sizeof(int));
        j = i - 1;
        i = i + 1;
        for (int k = l ; k <= p; k++){
           exch(a, &k, &j,sizeof(int));
           j--;
        }
        for (int k = r-1; k >= q; k--){
           exch(a, &k, &i,sizeof(int));
           i++;
        }
        sort(a, l, j,compare);
        sort(a, i, r,compare);
    }

int *createArray(int size){
    int *res = (int*) malloc(sizeof(int) * size);
    srand(time(NULL));
    for(int i  = 0  ; i < size ; i++){
        int j = (rand() % 100) + 1;
        res[i] = j;
        //a2[i] = j;
    }
    return res;
}

void printArray(int arr[],int size){
    for(int i = 0 ; i < size ; i++) printf("%ld ", arr[i]);
    printf("\n");
}

int *duplicateArray(int *a, int size){
    int *dupArray = (int *) malloc(sizeof(int) * size);
    for(int i = 0 ; i < size ; i++)
        dupArray[i] = a[i];
    return dupArray;
}

// int partition(int arr[],int l , int r){
//     int pivot = arr[r];
//     int i = (l - 1);
//     for(int j = l ; j <= r-1 ; j++){
//         if(arr[j] < pivot)
//             exch(arr,j,++i);
//     }
//     exch(arr,i+1,r);
//     return (i+1);
// }

// void two_ways_sort(int arr[],int l,int r){
//     if(l < r){
//         int pi = partition(arr,l,r);
//         two_ways_sort(arr,l,pi-1);
//         two_ways_sort(arr,pi+1,r);
//     }
// }

// void swap(int num1, int num2) {
//    int temp = intArray[num1];
//    intArray[num1] = intArray[num2];
//    intArray[num2] = temp;
// }

// int partition(int left, int right, int pivot) {
//    int leftPointer = left -1;
//    int rightPointer = right;

//    while(1) {
//       while(intArray[++leftPointer] < pivot) {
//          //do nothing
//       }
		
//       while(rightPointer > 0 && intArray[--rightPointer] > pivot) {
//          //do nothing
//       }

//       if(leftPointer >= rightPointer) {
//          break;
//       } else {
//          //printf(" item swapped :%d,%d\n", intArray[leftPointer],intArray[rightPointer]);
//          swap(leftPointer,rightPointer);
//       }
//    }
	
//    //printf(" pivot swapped :%d,%d\n", intArray[leftPointer],intArray[right]);
//    swap(leftPointer,right);
// //    printf("Updated Array: "); 
// //    display();
//    return leftPointer;
// }

// void quickSort(int left, int right) {
//    if(right-left <= 0) {
//       return;   
//    } else {
//       int pivot = intArray[right];
//       int partitionPoint = partition(left, right, pivot);
//       quickSort(left,partitionPoint-1);
//       quickSort(partitionPoint+1,right);
//    }        
// }

int int_compare(void const* x, void const*y){
   int m, n;
   m = *((int *) x);
   n = *((int *) y);
   if (m == n ) return 0;
   return m > n ? 1: -1;
}



int search(void *buf,int size , int l ,int r,void *item, int (*compare)(const void*,const void*)){
   int i, res;
   if ( r < l ) return -1;
   i = (l+r)/2;
   res = compare(item,(char*) (buf + (size*i)));
   if (res == 0) return i;
   else if ( res < 0) return search(buf,size,l,i-1,item,compare);
   else return search(buf,size,i+1,r,item,compare);
}
int main(){
    int *a,*b;
    int *intArray;
    intArray = createArray(SMALL_NUMBER);
    //printArray(intArray,SMALL_NUMBER);
    //a2 = (int*) malloc(sizeof(int) * SMALL_NUMBER);
    //a1 = createArray(a2,SMALL_NUMBER);
   //  a = duplicateArray(intArray,SMALL_NUMBER);
   //  b = duplicateArray(intArray,SMALL_NUMBER);
   //  clock_t t1,t2,t3;
   //  t1 = clock();
   //  sort(a,0,SMALL_NUMBER-1);
   //  t1 = clock() - t1;
   //  printf("The time wastes by 3-way-partition: %f\n", (double) t1/CLOCKS_PER_SEC);
   //  t2 = clock();
   //  quickSort(0,SMALL_NUMBER-1);
   //  t2 = clock() - t2;
   //  printf("The time wastes by 2-way-partition: %f\n", (double)t2/CLOCKS_PER_SEC);
   //  t3 = clock();
   //  qsort(b,SMALL_NUMBER,sizeof(int),int_compare);
   //  t3 = clock() - t3;
   //  printf("The time wastes by qsort: %f\n", (double)t3/CLOCKS_PER_SEC);
   //  free(a);
   //  free(b);
   //Sorting
   // qsort(intArray,SMALL_NUMBER,sizeof(int),int_compare);
   sort(intArray,0,SMALL_NUMBER-1,int_compare);
   printArray(intArray,SMALL_NUMBER);
   int key;
   printf("Enter the key: ");
   scanf("%d",&key);
   printf("%d\n",search(intArray,sizeof(int),0,SMALL_NUMBER-1,&key,int_compare));
   free(intArray);
}