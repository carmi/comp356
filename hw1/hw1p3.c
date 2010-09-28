/** @file hw1p12.c
 *  @brief An application that provides a visual animation of merge sort and
 *  quick sort.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifndef NDEBUG
    #include <stdarg.h>
#endif
#ifdef __MACOSX__
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
    #include <GLUT/glut.h>
#elif defined __LINUX__ || defined __CYGWIN__
    #define GL_GLEXT_PROTOTYPES
    #include <GL/gl.h>
    #include <GL/glu.h>
    #include <GL/glut.h>
#endif
#include "hw1utils.h"
#include <stack356.h>

#define RAND(MIN, MAX) (int) (rand() % (MAX - MIN + 1)) + MIN

// Defaults for the window.
#define DEFAULT_WIN_WIDTH 800
#define DEFAULT_WIN_HEIGHT 600
int win_width, win_height ;

// What color to draw.  Here we're doing something bad:  taking advantage
// of the fact that enumerated type elements are ints starting at 0.
typedef enum {RED, GREEN, BLUE} color_t ;

// GLUT window IDs
int upper_win, lower_win ;

// Stack Pointers
stack356_t* merge_stack;
//stack356_t* quick_stack;

typedef struct Sublist {
    int startIndex;
    int endIndex;
} Sublist;

void swap(int *xs, int index_a, int index_b);
int partition(int *xs, int m, int n);
void quick_sort_itr(int *xs, size_t size);
void merge_sort_itr(int *xs, size_t size, int(*compare)(int, int));
int* merge(int *first, size_t first_size, int *second, size_t second_size,
        int(*compare)(int, int));


int main(int argc, char **argv) {

    // Make a list.
    size_t list_size = 50;
    int* list = malloc(list_size*sizeof(int));
    for (int i=0; i<list_size; i++)
        list[i] = RAND(-100, 100);



    // Setup the merge_stack for merge_sort.
    merge_stack = make_stack();

    // Put xs in a node and on the stack; then enter the processing loop.
    Node* initial_node = make_node(xs, size, unsorted);
    push(merge_stack, initial_node);

    // Initialize the drawing window.
    glutInitWindowSize(DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT) ;
    glutInitWindowPosition(0, 0) ;
    glutInit(&argc, argv) ;
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB) ;

    // Create the main window, set display callback to a no-op.
    int main_win = glutCreateWindow("Visual Animation of Merge Sort and Quick Sort.") ;
    glutDisplayFunc(no_op) ;
    int win_width = glutGet(GLUT_WINDOW_WIDTH) ;
    int win_height = glutGet(GLUT_WINDOW_HEIGHT) ;

    // Create upper and lower subwindows, set appropriate display callbacks.
    upper_win = glutCreateSubWindow(main_win, 0, 0, win_width, win_height/2) ;
    glutDisplayFunc(draw_merge_sort) ;
    lower_win = glutCreateSubWindow(main_win, 0, win_height/2, win_width,
            win_height-win_height/2) ;
    glutDisplayFunc(draw_quick_sort) ;

    glutIdleFunc(iterate()) ;

    // Enter the main event loop.
    create_menu();
    glutMainLoop() ;

    return EXIT_SUCCESS ;

    
}

void swap(int *xs, int index_a, int index_b) {
    int tmp = xs[index_a];
    xs[index_a] = xs[index_b];
    xs[index_b] = tmp;
}

// partitions the sublist xs[m..n] around a pivot that's then returned
int partition(int *xs, int m, int n) {
    if (n - m < 1) return 0;
    else {
        int init_pivot_index = m;
        int pivot = xs[init_pivot_index];
        swap(xs, init_pivot_index, n);
        int final_pivot_index = m;
        for (int i = m; i < n; i++) {
            if (xs[i] <= pivot) {
                swap(xs, i, final_pivot_index);
                final_pivot_index++;
            }
        }
        swap(xs, final_pivot_index, n);
        return final_pivot_index;
    }
}

void quick_sort_itr(int *xs, size_t size) {
    if (size <= 1) {
        printf("Array too small (already sorted).\n");
    }
    else {
        stack356_t *callStack = make_stack();
        Sublist *initial_list = malloc(sizeof(Sublist));        
        initial_list->startIndex = 0;
        initial_list->endIndex = size - 1;
        push(callStack, initial_list);
        while (!stk_is_empty(callStack)) {
            Sublist *current_sublist = pop(callStack);
            int startIndex = current_sublist->startIndex;
            int endIndex = current_sublist->endIndex;
            // printf("Sorting list from index %d to index %d...\n", startIndex, endIndex);
            // printf("Unpartitioned array:\n");
            // printf("[");
            // for (int i = 0; i < size; i++) {
            //     if (i == 0) printf("%d", xs[0]);
            //     else printf(", %d", xs[i]);
            // }
            // printf("]\n");
            int pivot_index = partition(xs, startIndex, endIndex);
            // printf("Partitioned current sublist. Pivot at position %d...\n", pivot_index);
            // printf("Partitioned array:\n");
            // printf("[");
            // for (int i = 0; i < size; i++) {
            //      if (i == 0) printf("%d", xs[0]);
            //      else printf(", %d", xs[i]);
            // }
            // printf("]\n");
            int leftStartIndex = startIndex;
            int leftEndIndex = pivot_index - 1;
            if (leftEndIndex - leftStartIndex >= 1) {
                Sublist *left_list = malloc(sizeof(Sublist));
                left_list->startIndex = leftStartIndex;
                left_list->endIndex = leftEndIndex;
                push(callStack, left_list);
                // printf("    doing left\n");
            }
            int rightStartIndex = pivot_index + 1;
            int rightEndIndex = endIndex;
            if (rightEndIndex - rightStartIndex >= 1) {
                Sublist *right_list = malloc(sizeof(Sublist));
                right_list->startIndex = rightStartIndex;
                right_list->endIndex = rightEndIndex;
                push(callStack, right_list);
                // printf("     doing right\n");
            }
            free(current_sublist);
        }
    }
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
// Do one iteration on the stack stack of merge sort.
void merge_sort_iterate(stack356_t* stack, int(*compare)(int, int)) {

    if (!stk_is_empty(stack)) {
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

                // Free cur_node aslong as it's not the last node on the stack.
                // The last node on the stack should be freed by the caller.
                if (!stk_is_empty(stack))
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
                //TODO I think this needs to be removed or changed.
                //cp_array(cur_node->array, xs, cur_node->size);
                //free_node(cur_node);
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
 * Draw a visualization of the sorting method. This is done by iterating through the call stack and displaying each Node.
 * Assume values of array are smaller than win_height/2
 */
void draw_merge_sort(void) {
    win_height = glutGet(GLUT_WINDOW_HEIGHT);

    // Create the framebuffer, initialize to all zero (i.e., black).
    GLubyte fb[win_height][win_width][3] ;
    bzero(fb, (win_width*win_height*3)*sizeof(GLubyte)) ;

    int fb_index = 0;
    // Iterate through the stack, for each Node, get the array, for each value in array, put a column on the screen of its value.
    while (!stk_is_empty(merge_stack)) {
        Node* cur_node = pop(stack)
            for (int i=0; i<cur_node->size; i++) {
                make_column(fb, fb_index, cur_node->array[i]);
            }
        push(stack_copy, cur_node);
    }
    // Put stack back together from copy.
    while (!stk_is_empty(stack_copy)) {
        push(stack, pop(stack_copy));
    }


    // Fill the framebuffer with the values of the stack pixels.
    for (int h=win_height/3; h<2*win_height/3; ++h) {
        for (int w=0; w<win_width/3; ++w) {
            fb[h][(int)(win_width*left_pos)+w][color] = 255 ;
        }
    }

/**
 * Make a column in the framebuffer fb to represent the value of the element at fb_index.
 */
void make_column(fb, fb_index, value) {
    for (int h=0; h < value; h++) {
        fb[h][fb_index][0] = 255;
    }
}

/**
 * Iterate once through both algorithms.
 */
void iterate() {
    merge_sort_iterate(merge_stack, compare);
    //quick_sort_iterate(quick_stack, compare);

    glutSetWindow(upper_win) ;
    glutPostRedisplay() ;
    glutSetWindow(lower_win) ;
    glutPostRedisplay() ;
}
