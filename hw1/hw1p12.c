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

void merge_sort_rec(int *xs, size_t size, int(*compare)(int, int));
void merge_sort_itr(int *xs, size_t size, int(*compare)(int, int));
void merge(int *output, int *first, int first_size, int *second, int second_size,
        int(*compare)(int, int));
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
    if (size <= 1) return;

    // Recursive case: divide into two parts and recursively sort list.
    int middle = size/2;

    // Create new arrays to hold each part of xs and populate them from xs.
    int first[middle]; // First half of array xs
    int second[(size - middle)]; // Second half of array xs
    for (int i = 0; i < middle; ++i)
        first[i] = xs[i];
    for (int i = middle; i < size; ++i)
        second[i] = xs[i];

    // Recursively sort both parts of divided array.
    merge_sort_rec(first, middle, compare);
    merge_sort_rec(second, middle, compare);
    
    // After the above recursive sorts return, we can assume that first and
    // second are each sorted in non-decreasing order. We then merge them into
    // xs.
    merge(xs, first, middle, second, (size - middle), compare);
}
/**
 * Merge - merge two integer arrays together into a single array in
 * non-decreasing order. It is a precondition that each input array is already
 * sorted in non-decreasing order.
 *
 * Effect: the output array will contain all elements from first and second and
 *          be in non-decreasing order.
 *
 * @param output - the output array; must be an array of length (first_size +
 *          second_size)
 * @param first - the first array; must be sorted in non-decreasing order.
 * @param first_size - the length of first.
 * @param second - the second array; must be sorted in non-decreasing order.
 * @param second_size - the length of second.
 * @param compare - the comparison function. compare(x, y) returns
 *          a number < 0, 0, or a number > 0 as per x < y, x = y, or x > y.
 */
void merge(int *output, int *first, int first_size, int *second, int second_size,
        int(*compare)(int, int)) {
    // Create indices to keep track of position in arrays
    int output_index = 0;
    int first_index = 0;
    int second_index = 0;
    while ((first_index < first_size) || (second_index < second_size))
    {
        if ((first_index < first_size) && (second_index < second_size))
        {
            // Compare first items of each array, and place smaller in output.
            if (first[first_index] <= second[second_index])
            {
                output[output_index] = first[first_index];
                output_index++;
                first_index++;
            }
            else
            {
                output[output_index] = second[second_index];
                output_index++;
                second_index++;
            }
        }
        else if (first_index < first_size)
        {
            output[output_index] = first[first_index];
            output_index++;
            first_index++;
        }
        else if (second_index < second_size)
        {
            output[output_index] = second[second_index];
            output_index++;
            second_index++;
        }
    }
    return;
}

void merge_sort_itr(int *xs, size_t size, int(*compare)(int, int)) {
}
