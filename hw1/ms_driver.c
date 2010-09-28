/** @file ms_driver.c
 *  @brief A file that contains the main function and is used to run
 *  hw1p12.c.
 */

#include <stdlib.h>
#include <stdio.h>
#include "hw1utils.h"
#include "hw1p12.h"

int main(int argc, char **argv) {
    // Create an array of random integers.
    size_t size = 101;
    int* test_array = malloc(size * sizeof(int));
    for (int i = 0; i < size; i++)
        test_array[i] = RAND(-100, 100);
    printf("\n----Created new array---\n");
    print_array(test_array, size);
    merge_sort_itr(test_array, size, compare);
    printf("\n----Array Sorted. Printing array---\n");
    print_array(test_array, size);
    printf("\n");
    free(test_array);
}
