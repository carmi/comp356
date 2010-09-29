/** @file hw1utils.h
 *  @brief This is the header file for hw1utils.c.
 */
#include <stdlib.h>
#include <stack356.h>

#define RAND(MIN, MAX) (int) (rand() % (MAX - MIN + 1)) + MIN

/* Create an enum State to help us keep track of whether a Node on the
 *   stack is returning from a recursive call and is sorted or not.
 */
typedef enum _State {
    sorted,
    unsorted
} State;

typedef struct _Node {
    int* array;
    size_t size;
    State state;
} Node;

int compare(int, int);
void half(int *initial, int *first, size_t first_size, int *second,
    size_t second_size);
void cp_array(int* src, int* dest, size_t size);
Node* make_node(int *array, size_t size, State state);
void free_node(Node *node);
void print_array(int *array, size_t length);
void print_node(Node* node, size_t counter);
void print_stack(stack356_t* stack);
void merge(int *first, size_t first_size, int *second, size_t second_size,
        int* result, int(*compare)(int, int));