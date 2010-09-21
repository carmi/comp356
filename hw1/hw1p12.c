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
    // Base case: return if size is less than one.
    if (size <= 1)
        return *xs;
    
    // Recursive case: separate into two parts and recursively sort list.
    int middle = size/2;
    
    int first[middle]; // First half of array xs
    int second[(size - middle)]; // Second half of array xs
    
    for (int i = 0; i < middle; ++i)
        first[i] = xs[i];
    for (int i = middle; i < size; ++i)
        second[i] = xs[i];
    
    first = merge_sort_rec(first, middle, compare);
    second = merge_sort_rec(second, middle, compare);
    
    return merge(first, middle, second, (size - middle), compare);

/**
 * Merge - merge two integer arrays together into a single array in
 * non-descending order. It is a precondition that each input array is already
 * sorted in non-descending order.
 *
 * @param first - the first array; must be sorted in non-descending order.
 * @param first_size - the length of first.
 * @param second - the second array; must be sorted in non-descending order.
 * @param second_size - the length of second.
 * @param compare - the comparison function. compare(x, y) returns
 * a number < 0, 0, or a number > 0 as per x < y, x = y, or x > y.
 */
void merge(int *first, int first_size, int *second, int second_size,
        int(*compare)(int, int)) {
    // Create a result array and populate it.
    int result[(first_size + second_size)];
    int result_index = 0;
    
    int first_index = 0;
    int second_index = 0;
    while ((first_index < first_size) || (second_index < second_size))
    {
        if ((first_index < first_size) && (second_index < second_size))
        {
            // Compare first items of each array, and place smaller in result.
            if (first[first_index] <= second[second_index])
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






    

}






void merge_sort_itr(int *xs, size_t size, int(*compare)(int, int)) {
}

