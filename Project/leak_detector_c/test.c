#include	<malloc.h>
#include	"leak_detector_c.h"
int main()
{
	char * ptr1; 
    int * ptr2; 
    float * ptr3;

    

    ptr1 = (char *) malloc (10); // allocating 10 bytes        
    ptr2 = (int *) calloc (10, sizeof(int)); 	// allocating 40 bytes 
					// let sizeof int =  4 bytes)
    ptr3 = (float *) calloc (15, sizeof(float)); // allocating 60 bytes
    free(ptr2);
	atexit(report_mem_leak);
    return 0;

}
