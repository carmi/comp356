/** @file hw1p3
 *  @brief This file contains two programs: a recursive and iterative quick
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

#include <stack356.h>
#include "hw1utils.h"

// Function Declarations
void quick_sort_rec(int *xs, size_t start, size_t end, int(*compare)(int, int));
size_t partition(int *array, size_t start, size_t end, int(*compare)(int, int));

int main(int argc, char **argv) {
    // Create an array of random integers.
    size_t size = 5;
    int* test_array = malloc(size*sizeof(int));
    for (int i=0; i<size; i++)
        test_array[i] = (rand() % 40);
    
    size_t starter = 0;
    printf("\n----Created new array---\n");
    print_array(test_array, size);
    quick_sort_rec(test_array, starter, size-1, compare);
    printf("\n----Array Sorted. Printing array---\n");
    print_array(test_array, size);
    printf("\n");
}

/** Sort an array of integers according to a supplied comparison function. The
 * elements of the array will be rearranged into non-decreasing order according
 * to the comparison function.
 *
 * @param xs - the array to sort
 * @param start - the index in xs to begin quick sorting.
 * @param end - the index in xs to stop quick sorting.
 * @param compare - the comparison function. compare(x, y) returns
 * a number < 0, 0, or a number > 0 as per x < y, x = y, or x > y.
 */
void quick_sort_rec(int *xs, size_t start, size_t end, int(*compare)(int, int)) {
    // Base case: if size is one or zero assume list is sorted and return.
    if ((start-end) == 1 || (start-end) == 0) return;
    // Recursive case: divide into two parts and recursively sort list.
    size_t pivot = partition(xs, start, end, compare);
    if (start < (pivot-1)) quick_sort_rec(xs, start, pivot-1, compare);
    if (pivot < end) quick_sort_rec(xs, pivot, end, compare);
    return;
}

/**
 * partition - A quick sort partition function.
 * @param array - the array to partition.
 * @param start -the starting index in array.
 * @param end -the ending index in array.
 * @param compare - the comparison function.
 */
size_t partition(int *array, size_t start, size_t end, int(*compare)(int, int)) {
    size_t a = start;
    size_t b = end;

    size_t pivot = (start + end)/2;

    while (a <= b) {
        while(compare(array[a], array[pivot]) <= 0) a++;
        while(compare(array[pivot], array[b]) <= 0) b--;
        if (a <= b) {
            // Swap numbers
            int temp = array[a];
            array[a] = array[b];
            array[b] = temp;
            a++;
            b--;
        }
    }
    return a;
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
/*
void merge_sort_itr(int *xs, size_t size, int(*compare)(int, int)) {
    stack356_t* stack = make_stack();

    // Put xs in a node and on the stack; then enter the processing loop.
    Node* initial_node = make_node(xs, size, unsorted);
    push(stack, initial_node);

    while(!stk_is_empty(stack)) {
        print_stack(stack);
        Node* cur_node = pop(stack);
        
        // cur_node's state is unsorted, fake a recursive merge_sort
        if (cur_node->state == unsorted) {
            // If length 0 or 1, change state to sorted and put back on stack.
            if ((cur_node->size == 1) || cur_node->size == 0) {
                cur_node->state = sorted;
                push(stack, cur_node);
            }
            else {
                // Split and place back on stack.
                // Make two new arrays, and fill them.
                size_t middle = ((cur_node->size)/2);
                int* first = malloc(middle*sizeof(int));
                int* second = malloc(((cur_node->size)-middle)*sizeof(int));
                half(cur_node->array, cur_node->size, first, second);

                // Make two new nodes with those arrays and push.
                Node* first_node = make_node(first, middle, unsorted);
                push(stack, first_node);

                Node* second_node = make_node(second, (cur_node->size-middle), unsorted);
                push(stack, second_node);

                // Free cur_node
                free_node(cur_node);
            }
        } else if (cur_node->state == sorted) {
            if (!stk_is_empty(stack)) {
                // If stack isn't empty, get next_node and merge or swap.
                Node* next_node = pop(stack);
                // If next node is sorted, merge them together; else switch them.
                if (next_node->state == sorted) {
                    // Merge them into a new node.
                    int* merged_array = malloc((cur_node->size+next_node->size)*sizeof(int));
                    merged_array = merge(cur_node->array, cur_node->size,
                                         next_node->array, next_node->size,
                                         compare);
                    Node* merged_node = make_node(merged_array, (cur_node->size+next_node->size), sorted);
                    push(stack, merged_node);
                    
                    //Free two old nodes
                    free_node(cur_node);
                    free_node(next_node);
                } else if (next_node->state == unsorted) {
                    // Swap position of nodes.
                    push(stack, cur_node);
                    push(stack, next_node);
                } else {
                    printf("Uh oh. We shouldn't be here");
                }
            } else {
                // If stack is empty, then we're done.
                // Copy final arrive to address pointed to by xs, because we can't modify the value of xs.
                cp_array(cur_node->array, xs, cur_node->size);
                free_node(cur_node);
                return;
            }
        } else {
            printf("Uh oh. We shouldn't be here.");
        }
    }
    return;
}
*/
