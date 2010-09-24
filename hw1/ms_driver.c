/** @file ms_driver.c
 *  @brief A file that contains the main function and is used to run
 *  hw1p12.c.
 */

#include <stdlib.h>
#include <stdio.h>
#include "hw1p12.h"

void print_array(int *array, size_t length){
    printf("[ ");
    for (int i=0; i<length; i++)
        printf("%d, ", array[i]);
    printf(" ]\n");
}

int main(int argc, char **argv) {
    // Create an array of random integers.
    size_t size = 5;
    int* test_array = malloc(size*sizeof(int));
    size_t middle = size/2;
    test_array[0] = 4;
    test_array[1] = 53;
    test_array[2] = 3;
    test_array[3] = -1;
    test_array[4] = 4;

    //for (int i=0; i<size; i++)
    //    test_array[i] = (rand() % 30);

    printf("\n----Created new array---\n");
    print_array(test_array, size);
    merge_sort_rec(test_array, size, compare);
    print_array(test_array, size);
    free(test_array);

    // Test arrays and half
    /*
    int* first = malloc(middle*sizeof(int));
    int* second = malloc((size-middle)*sizeof(int));
    print_array(first, middle);
    print_array(second, size-middle);
    half(test_array, size, first, second);
    */

}
