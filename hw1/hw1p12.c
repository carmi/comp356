/** @file hw1p12.c
 *  @brief This file contains two programs: a recursive and iterative merge
 *  sort implementation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NDEBUG
#include <stdarg.h>
#endif

#ifdef __MACOSX__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#elif defined __LINUX__ || defined __CYGWIN__
// #define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

// Function Signatures
void merge_sort_rec(int *xs, size_t size, int(*compare)(int, int));
void merge_sort_itr(int *xs, size_t size, int(*compare)(int, int));
int* merge(int *first, size_t first_size, int *second, size_t second_size,
        int(*compare)(int, int));
int compare(int, int);
void half(int* initial, size_t initial_size, int* first, int* second);

// Debugger Functions

void print_array(int *array, size_t length);

/** Sort an array of integers according to a supplied comparison function. The
 * elements of the array will be rearranged into non-decreasing order according
 * to the comparison function.
 *
 * @param xs - the array to sort
 * @param size - the length of xs. Must have size > 1
 * @param compare - the comparison function. compare(x, y) returns
 * a number < 0, 0, or a number > 0 as per x < y, x = y, or x > y.
 */
void merge_sort_rec(int *xs, size_t size, int(*compare)(int, int)) {
    // Base case: if size is one or zero assume list is sorted and return.
    if (size == 1 || size == 0) return;
    // Recursive case: divide into two parts and recursively sort list.
    size_t middle = size/2;
    int* first = malloc(middle*sizeof(int));
    int* second = malloc((size-middle)*sizeof(int));
    // Split xs into first and second
    half(xs, size, first, second);

    // Recursively sort both parts of divided array.
    merge_sort_rec(first, middle, compare);
    merge_sort_rec(second, (size - middle), compare);
    
    // After the above recursive sorts return, we can assume that first and
    // second are each sorted in non-decreasing order. We then merge them into
    // xs.
    
    // Free the old contents of xs, and assign to the new array created by merge.
    free(xs);
    xs = merge(first, middle, second, (size - middle), compare);
    free(first);
    free(second);
}
/**
 * Merge - merge two integer arrays together and return a single array
 * consisting of their elements in non-decreasing order. It is a precondition
 * that each input array is already sorted in non-decreasing order.
 *
 * @param first - the first array; must be sorted in non-decreasing order.
 * @param first_size - the length of first.
 * @param second - the second array; must be sorted in non-decreasing order.
 * @param second_size - the length of second.
 * @param compare - the comparison function. compare(x, y) returns
 *          a number < 0, 0, or a number > 0 as per x < y, x = y, or x > y.
 *
 * @return int* - returns a pointer to the array of the two input lists merged together.
 */
int* merge(int *first, size_t first_size, int *second, size_t second_size,
        int(*compare)(int, int)) {
    // Create a new array to hold the result of the merge.
    int* result = malloc((first_size + second_size)*sizeof(int));
    // Create indices to keep track of position in arrays
    size_t result_index = 0;
    size_t first_index = 0;
    size_t second_index = 0;
    while ((first_index < first_size) || (second_index < second_size))
    {
        if ((first_index < first_size) && (second_index < second_size))
        {
            // Compare first items of each array, and place smaller in result.
            if (compare(first[first_index], second[second_index]) <= 0)
            {
                result[result_index] = first[first_index];
                result_index++;
                first_index++;
            }
            else
            {
                result[result_index] = second[second_index];
                result_index++;
                second_index++;
            }
        }
        else if (first_index < first_size)
        {
            result[result_index] = first[first_index];
            result_index++;
            first_index++;
        }
        else if (second_index < second_size)
        {
            result[result_index] = second[second_index];
            result_index++;
            second_index++;
        }
    }
    return result;
}

void merge_sort_itr(int *xs, size_t size, int(*compare)(int, int)) {
}

/**
 * compare - an integer comparison function. Returns a number < 0, 0, or a number > 0 as per x < y, x = y,
 *          or x > y. 

 *
 * @param x - the first integer
 * @param y - the second integer
 * 
 * @return int - returns -1, 0, or 1 as per x < y, x = y, or x > y. 
 */
int compare(int x, int y) {
    if (x < y) return -1;
    else if (x == y) return 0;
    else return 1;
}

/**
 * half - split a given array into two separate arrays.
 * @param initial - the array to be halved.
 * @param initial_size - the size of the array initial
 * @param first - the array to contain the first half of initial.
 * @param second - the array to contain the second half of initial.
 * Precondition: arrays first and second are already allocated and big enough
 * to contain all elements of initial.
 */
void half(int* initial, size_t initial_size, int* first, int* second) {
    size_t middle = initial_size/2;
    for (size_t i = 0; i < middle; ++i)
        first[i] = initial[i];
    for (size_t i = middle; i < initial_size; ++i)
        second[(i - middle)] = initial[i];
    return;
}
