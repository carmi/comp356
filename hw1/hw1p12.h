/** @file hw1p12.h
 *  @brief This header file defines the following functions used in
 *  hw1p12.h.
 */
#include <stdlib.h>

void merge_sort_rec(int *xs, size_t size, int(*compare)(int, int));
void merge_sort_itr(int *xs, size_t size, int(*compare)(int, int));
int* merge(int *first, size_t first_size, int *second, size_t second_size,
        int(*compare)(int, int));
int compare(int, int);
void half(int* initial, size_t initial_size, int* first, int* second);
