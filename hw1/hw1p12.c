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

#include <stack356.h>

/* Create an enum State to help us keep track of whether a Node on the
 *   stack is returning from a recursive call and is sorted or not.
 */
enum State { sorted, unsorted };

typedef struct _Node {
    int* array;
    size_t size;
    enum State state;
} Node;

// Function Signatures
void merge_sort_rec(int *xs, size_t size, int(*compare)(int, int));
void merge_sort_itr(int *xs, size_t size, int(*compare)(int, int));
int* merge(int *first, size_t first_size, int *second, size_t second_size,
        int(*compare)(int, int));
int compare(int, int);

void half(int *initial, size_t initial_size, int *first, int *second);
void cp_array(int* src, int* dest, size_t size);
Node* make_node(int *array, size_t size, enum State state);
void free_node(Node *node);

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
    printf("\n#%d   Size: %d", counter, node->size);
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
    return;
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
 * @param initial_size - the size of the array initial
 * @param first - the array to contain the first half of initial.
 * @param second - the array to contain the second half of initial.
 * Precondition: arrays first and second are already allocated and big enough
 * to contain all elements of initial.
 */
void half(int *initial, size_t initial_size, int *first, int *second) {
    size_t middle = initial_size/2;
    cp_array(initial, first, middle);
    // initial + middle, is the subarray we want. same as &initial[2]
    cp_array(initial+middle, second, initial_size);
    return;
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
Node* make_node(int *array, size_t size, enum State state) {
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
