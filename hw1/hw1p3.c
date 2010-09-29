/** @file hw1p12.c
 *  @brief An application that provides a visual animation of merge sort and
 *  quick sort.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
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
int win_width, win_height;

// What color to draw.  Here we're doing something bad:  taking advantage
// of the fact that enumerated type elements are ints starting at 0.
typedef enum {RED, GREEN, BLUE} color_t;

// GLUT window IDs
int upper_win, lower_win;
typedef enum {PRESORT, SORTING, SORTED} state_t;
state_t state;

// A marginally-clever debugging function that expends to a no-op
// when NDEBUG is defined and otherwise prints its string formatting
// arguments to stderr.
#ifdef NDEBUG
#define debug(fmt, ...) 
#else
void debug(const char* fmt, ...) {
    va_list argptr ;
    va_start(argptr, fmt) ;

    char* fmt_newline ;
    asprintf(&fmt_newline, "%s\n", fmt) ;
    vfprintf(stderr, fmt_newline, argptr) ;
    free(fmt_newline) ;
}
#endif



// Stack Pointers
stack356_t* merge_stack;
stack356_t* quick_stack;

typedef struct Sublist {
    int start_index;
    int end_index;
} Sublist;

void draw_upper_rectangle(void) ;                 // Display callback.
void draw_lower_rectangle(void) ;                 // Display callback.
void create_menu() ;

void draw_black() ;
void swap(int *xs, int index_a, int index_b);
int partition(int *xs, int m, int n);
void draw_quick_sort();
void quick_sort_iterate(stack356_t* stack) ;
void draw_merge_sort(void);
void iterate(void);
void no_op(void);
void process_menu(int option);

//Global Lists
size_t list_size;
int* merge_list;
int* quick_list;

int main(int argc, char **argv) {
    state = PRESORT;

    // Make a list, list.
    list_size = DEFAULT_WIN_WIDTH/3;
    int* list = malloc(list_size * sizeof(int));
    for (int i=0; i<list_size; i++)
        list[i] = RAND(0, DEFAULT_WIN_HEIGHT/4);
    
    // Setup Merge Sort and Quick Sort
    merge_list = malloc(list_size * sizeof(int));
    cp_array(list, merge_list, list_size);
    // Setup the merge_stack for merge_sort.
    merge_stack = make_stack();

    int* initial_array = malloc(list_size * sizeof(int));
    cp_array(merge_list, initial_array, list_size);
    // Put xs in a node and on the stack; then enter the processing loop.
    Node* initial_node = make_node(initial_array, list_size, unsorted);
    push(merge_stack, initial_node);

    // Setup Quick Sort
    quick_list = malloc(list_size * sizeof(int));
    cp_array(list, quick_list, list_size);


    quick_stack = make_stack();
    Sublist *initial_list = malloc(sizeof(Sublist));
    initial_list->start_index = 0;
    initial_list->end_index = list_size - 1;
    push(quick_stack, initial_list); 

    // Initialize the drawing window.
    glutInitWindowSize(DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT) ;
    glutInitWindowPosition(0, 0) ;
    glutInit(&argc, argv) ;
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB) ;


    // Create the main window, set display callback to a no-op.
    int main_win = glutCreateWindow("Visual Animation of Merge Sort and Quick Sort.") ;
    glutDisplayFunc(draw_black) ;

    // Create a menu


    int win_width = glutGet(GLUT_WINDOW_WIDTH) ;
    int win_height = glutGet(GLUT_WINDOW_HEIGHT) ;

    // Create upper and lower subwindows, set appropriate display callbacks.
    upper_win = glutCreateSubWindow(main_win, 0, 0, win_width, win_height/2) ;
    glutDisplayFunc(draw_merge_sort) ;
    create_menu();
    lower_win = glutCreateSubWindow(main_win, 0, win_height/2, win_width,
            win_height-win_height/2) ;
    glutDisplayFunc(draw_quick_sort) ;
    create_menu();

    // Enter the main event loop.

    glutIdleFunc(iterate) ;
    glutMainLoop() ;
    return EXIT_SUCCESS ;
}

void swap(int *xs, int index_a, int index_b) {
    debug("swap");
    int tmp = xs[index_a];
    xs[index_a] = xs[index_b];
    xs[index_b] = tmp;
}

// partitions the sublist xs[m..n] around a pivot that's then returned
// the return value is with regards to xs not xs[m...n]
int partition(int *xs, int m, int n) {
    debug("partition");
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

void quick_sort_iterate(stack356_t* stack) {
    debug("quick_sort_iterate");
    if (!stk_is_empty(stack)) {
        debug("prepartition");
        Sublist *current_sublist = pop(stack);
        debug("postpartition");
        int start_index = current_sublist->start_index;
        int end_index = current_sublist->end_index;
        int pivot_index = partition(quick_list, start_index, end_index);
        // Create two sublists and push them on the stack.
        int left_start_index = start_index;
        int left_end_index = pivot_index - 1;
        if (left_end_index - left_start_index >= 1) {
            Sublist *left_list = malloc(sizeof(Sublist));
            left_list->start_index = left_start_index;
            left_list->end_index = left_end_index;
            push(stack, left_list);
        }
        int right_start_index = pivot_index + 1;
        int right_end_index = end_index;
        if (right_end_index - right_start_index >= 1) {
            Sublist *right_list = malloc(sizeof(Sublist));
            right_list->start_index = right_start_index;
            right_list->end_index = right_end_index;
            push(stack, right_list);
        }
        free(current_sublist);
    }
    debug("endif");
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

void merge_sort_iterate(stack356_t* stack, int(*compare)(int, int)) {
    debug("merge_sort_iterate");
    if (!stk_is_empty(stack)) {
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
                debug("Changing state to SORTED");
                state = SORTED;
                // Copy the array back to original array.
                cp_array(cur_node->array, merge_list, list_size);
            }
        } else {
            printf("Unrecognized state.");
        }
    }
}

/**
 * Draw a visualization of the sorting method. This is done by iterating through the call stack and displaying each Node.
 * Assume values of array are smaller than win_height/2
 */
void draw_quick_sort() {
    debug("draw_quick_sort");
    win_width = glutGet(GLUT_WINDOW_WIDTH);
    win_height = glutGet(GLUT_WINDOW_HEIGHT);

    // Create the framebuffer, initialize to all zero (i.e., black).
    GLubyte fb[win_height][win_width][3] ;
    bzero(fb, (win_width*win_height*3)*sizeof(GLubyte)) ;

    // Because quick_sort is in_place, we can display the real list in the
    // middle of the sort without any problems.
    // Display list from global list.
    // Go through list
    debug("1hfkja");
    print_array(quick_list, list_size);
    debug("2hfkja");
    for (int elm = 0; elm < list_size; elm++) {
        int value = quick_list[elm];
        for (int height = 0; height < value; height++) {
            fb[height][3*elm][0] = 255;
        }
    }
    // Draw the pixels.  An alternative to WindowPos* is RasterPos*, but the
    // latter takes into account the current model-view transformation, so
    // doesn't seem appropriate.  We should probably always specify 
    // GL_UNPACK_ALIGNMENT to be certain that we read the "next" row 
    // of pixels correctly.
    glWindowPos2s(0, 0) ;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1) ;
    glDrawPixels(win_width, win_height, GL_RGB, GL_UNSIGNED_BYTE, fb) ;

    glFlush() ;
    glutSwapBuffers() ;
}

/**
 * Draw a visualization of the sorting method. This is done by iterating through the call stack and displaying each Node.
 * Assume values of array are smaller than win_height/2
 */
void draw_merge_sort() {
    debug("draw_merge_sort");
    win_width = glutGet(GLUT_WINDOW_WIDTH);
    win_height = glutGet(GLUT_WINDOW_HEIGHT);

    // Create the framebuffer, initialize to all zero (i.e., black).
    GLubyte fb[win_height][win_width][3] ;
    bzero(fb, (win_width*win_height*3)*sizeof(GLubyte)) ;

    if (state == PRESORT || state == SORTED) {
        // Display list from global list.
        // Go through list
        for (int elm = 0; elm < list_size; elm++) {
            int value = merge_list[elm];
            for (int height = 0; height < value; height++) {
                fb[height][3*elm][1] = 255;
            }
        }
    }
    else if (state == SORTING) {
        stack356_t* stack_copy = make_stack();
        // Iterate through the stack, for each Node, get the array, for each value in array, put a column on the screen of its value.
        int fb_col = 0;
        while (!stk_is_empty(merge_stack)) {
            Node* cur_node = pop(merge_stack);
            // Go through list
            for (int elm = 0; elm < cur_node->size; elm++) {
                int value = cur_node->array[elm];
                for (int height = 0; height < value; height++) {
                    fb[height][3*fb_col][1] = 255;
                }
                fb_col++;
            }
            push(stack_copy, cur_node);
        }
        // Put stack back together from copy.
        while (!stk_is_empty(stack_copy)) {
            push(merge_stack, pop(stack_copy));
        }
    }

    // Draw the pixels.  An alternative to WindowPos* is RasterPos*, but the
    // latter takes into account the current model-view transformation, so
    // doesn't seem appropriate.  We should probably always specify 
    // GL_UNPACK_ALIGNMENT to be certain that we read the "next" row 
    // of pixels correctly.
    glWindowPos2s(0, 0) ;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1) ;
    glDrawPixels(win_width, win_height, GL_RGB, GL_UNSIGNED_BYTE, fb) ;

    glFlush() ;
    glutSwapBuffers() ;
}


/**
 * Iterate once through both algorithms.
 */
void iterate() {
    if (state == PRESORT) {
        debug("iterate: state = %d", state);
    }else if (state == SORTING) {
        debug("iterate: state = %d", state);
        merge_sort_iterate(merge_stack, compare);
        quick_sort_iterate(quick_stack);

        glutSetWindow(upper_win) ;
        glutPostRedisplay() ;
        glutSetWindow(lower_win) ;
        glutPostRedisplay() ;
    }
    else {
        debug("iterate: state = %d", state);
    }
}

/**
 * make_sublist - creates a new Sublist and returns a pointer to it.
 * @param startIndex - the index of the original list at which sublist starts
 * @param endIndex - the index of the original list at which the sublist ends
 * @returns - returns a pointer to the newly created Sublist
 */
Sublist* make_sublist(int start_index, int end_index) {
    debug("make_sublist"); 
    Sublist* new_sublist = malloc(sizeof(Sublist));
    new_sublist->start_index = start_index;
    new_sublist->end_index = end_index;
    return new_sublist;
}

void no_op() {
    debug("no_op");
}

void process_menu(int option) {
    debug("process_menu with option: %d", option);
    switch (option) {
        case 1 :
            // Begin sorting. Set state to 1.
            state = SORTING;
            debug("Changed state to %d", state);
            break;
        case 0 :
            // Exit program.
            debug("Exiting from menu", state);
            exit(0);
            break;
    }
}
void draw_black() {
    debug("draw_black") ;
    win_width = glutGet(GLUT_WINDOW_WIDTH) ;
    win_height = glutGet(GLUT_WINDOW_HEIGHT) ;

    // Create the framebuffer, initialize to all zero (i.e., black).
    GLubyte fb[win_height][win_width][3] ;
    bzero(fb, (win_width*win_height*3)*sizeof(GLubyte)) ;

    // Draw the pixels.  An alternative to WindowPos* is RasterPos*, but the
    // latter takes into account the current model-view transformation, so
    // doesn't seem appropriate.  We should probably always specify 
    // GL_UNPACK_ALIGNMENT to be certain that we read the "next" row 
    // of pixels correctly.
    glWindowPos2s(0, 0) ;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1) ;
    glDrawPixels(win_width, win_height, GL_RGB, GL_UNSIGNED_BYTE, fb) ;

    glFlush() ;

    glutSwapBuffers() ;

}


void draw_rectangle(color_t color) {
    debug("draw_rectangle(%d)", color) ;
    win_width = glutGet(GLUT_WINDOW_WIDTH) ;
    win_height = glutGet(GLUT_WINDOW_HEIGHT) ;

    // Create the framebuffer, initialize to all zero (i.e., black).
    GLubyte fb[win_height][win_width][3] ;
    bzero(fb, (win_width*win_height*3)*sizeof(GLubyte)) ;

    // Fill the framebuffer with the rectangle pixels.
    for (int h=win_height/3; h<2*win_height/3; ++h) {
        for (int w=0; w<win_width/3; ++w) {
            fb[h][(int)(win_width)+w][color] = 255 ;
        }
    }

    // Draw the pixels.  An alternative to WindowPos* is RasterPos*, but the
    // latter takes into account the current model-view transformation, so
    // doesn't seem appropriate.  We should probably always specify 
    // GL_UNPACK_ALIGNMENT to be certain that we read the "next" row 
    // of pixels correctly.
    glWindowPos2s(0, 0) ;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1) ;
    glDrawPixels(win_width, win_height, GL_RGB, GL_UNSIGNED_BYTE, fb) ;

    glFlush() ;

    glutSwapBuffers() ;

}

/** Handle display events for the upper window.  Draw a red rectangle
 *  using draw_rectangle.
 */
void draw_upper_rectangle() {
    draw_rectangle(RED) ;
}

/** Handle display events for the upper window.  Draw a green rectangle
 *  using draw_rectangle.
 */
void draw_lower_rectangle() {
    draw_rectangle(GREEN) ;
}

void create_menu() {
    debug("create_menu");
    glutCreateMenu(process_menu);
    glutAddMenuEntry("Animate sorting.", 1);
    glutAddMenuEntry("Quit!", 0);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

