/** @file hw1utils.c
 *  @brief This file contains some useful utility functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hw1utils.h"

// Utility Functions
/**
 * compare - an integer comparison function. Returns a number < 0, 0, or a
 * number > 0 as per x < y, x = y, or x > y. 
 * @param x - the first integer
 * @param y - the second integer
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
 * @param first - the array to contain the first half of initial.
 * @param first_size - the size of first.
 * @param second - the array to contain the second half of initial.
 * @param second_size - the size of second.
 * Precondition: initial has size first_size + second_size and all arrays
 * have been allocated.
 */
void half(int *initial, int *first, size_t first_size, int *second,
        size_t second_size) {
    cp_array(initial, first, first_size);
    // initial + middle is the subarray we want.
    cp_array(initial + first_size, second, second_size);
}

/**
 * cp_array - copy size elements of array src to array dest.
 * @param src - the source array
 * @param dest - the destination array
 * @param size - the number of elements to be copied, must not be greater than
 * the size of src or dest.
 */
void cp_array(int *src, int *dest, size_t size) {
    for (size_t i = 0; i < size; i++) {
        dest[i] = src[i];
    }
}

/**
 * make_node - creates a new Node and returns a pointer to it.
 * @param array - the node's array.
 * @param size - the node's array's size.
 * @param state - the node's state.
 * @returns - returns a pointer to the newly created Node.
 */
Node* make_node(int *array, size_t size, State state) {
    Node *new_node = malloc(sizeof(Node));
    new_node->array = array;
    new_node->size = size;
    new_node->state = state;
    return new_node;
}

/**
 * free_node - deeply free the data structures of a node.
 * @param node - pointer to node to be freed.
 * Effect: the node's array is free()'d, and then the node is free()'d.
 */
void free_node(Node *node) {
    free(node->array);
    free(node);
}

/**
 * Merge - merge two integer arrays together into a single array
 * consisting of their elements in non-decreasing order. It is a precondition
 * that each input array is already sorted in non-decreasing order.
 *
 * @param first - the first array; must be sorted in non-decreasing order.
 * @param first_size - the length of first.
 * @param second - the second array; must be sorted in non-decreasing order.
 * @param second_size - the length of second.
 * @param result - int array to merge first and second into.
 * @param compare - the comparison function. compare(x, y) returns
 *          a number < 0, 0, or a number > 0 as per x < y, x = y, or x > y.
 */
void merge(int *first, size_t first_size, int *second, size_t second_size,
        int *result, int(*compare)(int, int)) {
    size_t result_size = first_size + second_size;
    size_t first_index = 0;
    size_t second_index = 0;
    for (int result_index = 0; result_index < result_size; result_index++) {
        if ((first_index < first_size) && (second_index < second_size)) {
            // Compare first items of each array, and place smaller in result.
            if (compare(first[first_index], second[second_index]) <= 0) {
                result[result_index] = first[first_index];
                first_index++;
            }
            else {
                result[result_index] = second[second_index];
                second_index++;
            }
        }
        else if (first_index < first_size) {
            result[result_index] = first[first_index];
            first_index++;
        }
        else if (second_index < second_size) {
            result[result_index] = second[second_index];
            second_index++;
        }
    }
}
