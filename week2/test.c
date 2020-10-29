#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "../Libfdr/jval.h"

void printArray(int arr[], int size);
#define SMALL_NUMBER 100
#define HUGE_NUMBER 10000000

void exch_generic(void *buf, int i, int j, int size)
{
    void *temp = malloc(sizeof(char) * size);
    memcpy(temp, buf + size * i, size);
    memcpy(buf + size * i, buf + size * j, size);
    memcpy(buf + size * j, temp, size);
}
void generic_sort(void *arr, int l, int r, int size, int (*compare)(const void *, const void *))
{
    if (r <= l)
        return;
    int i = l - 1, j = r;
    int p = l - 1, q = r;
    while (1)
    {
        //printArray(a,SMALL_NUMBER);
        while (compare(arr + size * (++i), arr + size * r) < 0);
        //printArray(a,SMALL_NUMBER);
        while (compare(arr + size * r, arr + size * (--j)) < 0)
            if (j == l)
                break;
        if (i >= j)
            break;
        exch_generic(arr, i, j, sizeof(int));
        if (compare(arr + size * i, arr + size * r) == 0)
        {
            p++;
            exch_generic(arr, p, i, sizeof(int));
        }
        if (compare(arr + size * j, arr + size * r) == 0)
        {
            q--;
            exch_generic(arr, q, j, sizeof(int));
        }
    }
    exch_generic(arr, i, r, sizeof(int));
    j = i - 1;
    i = i + 1;
    for (int k = l; k <= p; k++)
    {
        exch_generic(arr, k, j, sizeof(int));
        j--;
    }
    for (int k = r - 1; k >= q; k--)
    {
        exch_generic(arr, k, i, sizeof(int));
        i++;
    }
    generic_sort(arr, l, j, size, compare);
    generic_sort(arr, i, r, size, compare);
}

int *createArray(int size)
{
    int *res = (int *)malloc(sizeof(int) * size);
    srand(time(NULL));
    for (int i = 0; i < size; i++)
    {
        int j = (rand() % 100) + 1;
        res[i] = j;
    }
    return res;
}

void printArray(int arr[], int size)
{
    for (int i = 0; i < size; i++)
        printf("%ld ", arr[i]);
    printf("\n");
}

int *duplicateArray(int *a, int size)
{
    int *dupArray = (int *)malloc(sizeof(int) * size);
    for (int i = 0; i < size; i++)
        dupArray[i] = a[i];
    return dupArray;
}

int int_compare(void const *x, void const *y)
{
    int m, n;
    m = *((int *)x);
    n = *((int *)y);
    if (m == n)
        return 0;
    return m > n ? 1 : -1;
}

int double_compare(void const *x, void const *y)
{
    double m, n;
    m = *((double *)x);
    n = *((double *)y);
    if (m == n)
        return 0;
    return m > n ? 1 : -1;
}

int *dumpArray(int *a, int n)
{
    int *b = (int *)malloc(n * sizeof(int));
    memcpy(b, a, n * sizeof(int));
    return b;
}

int search(void *buf, int size, int l, int r, void *item, int (*compare)(const void *, const void *))
{
    int i, res;
    if (r < l)
        return -1;
    i = (l + r) / 2;
    res = compare(item, (char *)(buf + (size * i)));
    if (res == 0)
        return i;
    else if (res < 0)
        return search(buf, size, l, i - 1, item, compare);
    else
        return search(buf, size, i + 1, r, item, compare);
}

int main()
{
    int *a, *b;
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
    qsort(intArray, SMALL_NUMBER, sizeof(int), int_compare);
    generic_sort(intArray, 0, SMALL_NUMBER - 1, sizeof(int), int_compare);
    printArray(intArray, SMALL_NUMBER);
    int key;
    printf("Enter the key: ");
    scanf("%d", &key);
    printf("%d\n", search(intArray, sizeof(int), 0, SMALL_NUMBER - 1, &key, int_compare));
    free(intArray);
}