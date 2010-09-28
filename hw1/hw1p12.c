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

#include "hw1utils.h"
#include <stack356.h>

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
    size_t first_array_size = size/2;
    size_t second_array_size = size - first_array_size;
    int* first = malloc(first_array_size * sizeof(int));
    int* second = malloc(second_array_size * sizeof(int));
    
    half(xs, first, first_array_size, second, second_array_size);

    merge_sort_rec(first, first_array_size, compare);
    merge_sort_rec(second, second_array_size, compare);
    
    // After the above recursive sorts return, we can assume that first and
    // second are each sorted in non-decreasing order. We then merge them into
    // xs.
    
    merge(first, first_array_size, second, second_array_size, xs, compare);
    free(first);
    free(second);
}

/**
 * An iterative implementation of merge sort. Sort an array of integers
 * according to a supplied comparison function. The elements of the array will
 * be rearranged into non-decreasing order according to the comparison
 * function.
 *
 * @param xs - the array to sort
 * @param size - the length of xs. Must have size > 1
 * @param compare - the comparison function. compare(x, y) returns
 * a number < 0, 0, or a number > 0 as per x < y, x = y, or x > y.
 */

void merge_sort_itr(int *xs, size_t size, int(*compare)(int, int)) {
    stack356_t* stack = make_stack();

    // Copy xs to an array we can free and play with.
    int* initial_array = malloc(size*sizeof(int));
    cp_array(xs, initial_array, size);

    // Put initial_array in a node and on the stack; enter the processing loop.
    Node* initial_node = make_node(initial_array, size, unsorted);
    push(stack, initial_node);

    while (!stk_is_empty(stack)) {
        //print_stack(stack);
        Node* cur_node = pop(stack);
        State cur_state = cur_node->state;
        // cur_node's state is unsorted, fake a recursive merge_sort
        if (cur_state == unsorted) {
            size_t cur_size = cur_node->size;
            // If length 0 or 1, change state to sorted and put back on stack.
            if (cur_size == 1 || cur_size == 0) {
                cur_node->state = sorted;
                push(stack, cur_node);
            }
            else {
                // Split and place back on stack.
                // Make two new arrays, and fill them.
                size_t first_array_size = (cur_size / 2);
                size_t second_array_size = (cur_size - first_array_size);
                int* first = malloc(first_array_size * sizeof(int));
                int* second = malloc(second_array_size * sizeof(int));
                half(cur_node->array, first, first_array_size, second,
                    second_array_size);
                // Push new arrays on stack as nodes.
                Node* first_node = make_node(first, first_array_size,
                    unsorted);
                push(stack, first_node);

                Node* second_node = make_node(second, second_array_size,
                    unsorted);
                push(stack, second_node);

                free_node(cur_node);
            }
        } else if (cur_state == sorted) {
            if (!stk_is_empty(stack)) {
                // If stack isn't empty, get next_node and merge or swap.
                Node* next_node = pop(stack);
                // If next_node's sorted, merge them together; else switch them.
                if (next_node->state == sorted) {
                    // Merge them into a new node.
                    size_t first_half_size = cur_node->size;
                    size_t second_half_size = next_node->size;
                    size_t merged_size = first_half_size + second_half_size;
                    int* merged_array = malloc(merged_size * sizeof(int));
                    merge(cur_node->array, first_half_size, next_node->array,
                        second_half_size, merged_array, compare);
                    Node* merged_node = make_node(merged_array, merged_size,
                        sorted);
                    push(stack, merged_node);
                    
                    free_node(cur_node);
                    free_node(next_node);
                } else if (next_node->state == unsorted) {
                    // Swap position of nodes.
                    push(stack, cur_node);
                    push(stack, next_node);
                } else {
                    printf("Unrecognized state.");
                }
            } else {
                // If stack is empty, then we're done.
                // Copy final array to address pointed to by xs.
                cp_array(cur_node->array, xs, cur_node->size);
                free_node(cur_node);
            }
        } else {
            printf("Unrecognized state.");
        }
    }
}
