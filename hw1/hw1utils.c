/** @file hw1utils.c
 *  @brief This file contains some useful utility functions.
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

// Debugging Functions
void print_array(int *array, size_t length){
    printf("[");
    for (int i=0; i<length; i++){
        printf(" %d", array[i]);
        if ((i+1) < length) printf(",");
    };
    printf(" ]");
}
void print_node(Node* node, size_t counter) {
    printf("\n------------------------Node---------------------------\n     Array: ");
    print_array(node->array, node->size);
    printf("\n#%d   Size: %d", (int)counter, (int)node->size);
    if (node->state == sorted) printf("\n     State: sorted");
    if (node->state == unsorted) printf("\n     State: unsorted");
    printf("\n-------------------------------------------------------");
    return;
}
void print_stack(stack356_t* stack) {
    printf("\n*************************Printing Stack*******************\n");
    stack356_t* copy = make_stack();
    size_t counter = 1;
    while (!stk_is_empty(stack)) {
        // Move from stack to copy, printing each node.
        Node* cur_node = pop(stack);
        print_node(cur_node, counter);
        push(copy, cur_node);
        counter++;
    }
    while (!stk_is_empty(copy)) {
        // Move back to stack.
        Node* cur_node = pop(copy);
        push(stack, cur_node);
    }
    printf("\n**********************************************************\n");
    return;
}

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
void cp_array(int* src, int* dest, size_t size) {
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
    Node* new_node = malloc(sizeof(Node));
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
