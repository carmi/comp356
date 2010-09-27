/** @file hw1utils.h
 *  @brief This is the header file for hw1utils.c.
 */
#include <stdlib.h>
#include <stack356.h>
#include "hw1p12.h"

/* Create an enum State to help us keep track of whether a Node on the
 *   stack is returning from a recursive call and is sorted or not.
 */
enum State {
    sorted,
    unsorted
};

typedef struct _Node {
    int* array;
    size_t size;
    enum State state;
} Node;

int compare(int, int);
void half(int *initial, size_t initial_size, int *first, int *second);
void cp_array(int* src, int* dest, size_t size);
Node* make_node(int *array, size_t size, enum State state);
void free_node(Node *node);
void print_array(int *array, size_t length);
void print_node(Node* node, size_t counter);
void print_stack(stack356_t* stack);
