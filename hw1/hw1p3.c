/** @file hw1p3.c
 *  @brief An application that provides a visual animation of merge sort and
 *  quick sort.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
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
#include "stack356.h"

#define MAX_STRING_LENGTH 256
#define DEFAULT_WIN_WIDTH 800
#define DEFAULT_WIN_HEIGHT 600

// Window dimensions.
int win_width, win_height;
int upper_win_height, lower_win_height;
// Window identifiers.
int main_win, upper_win, lower_win;

bool in_intro;

// Lists that will be displayed, and their two properties.
int *xs_mergesort;
int *xs_quicksort;
int xs_highest;
int xs_size;

// Counters (for text output).
int mergesort_counter;
int quicksort_counter;

// Stacks our algorithms will use.
stack356_t *quicksort_stack;
stack356_t *mergesort_stack;

// Our Sublist struct, for use in our quicksort algorithm.
typedef struct Sublist {
    int start_index;
    int end_index;
} Sublist;

// Function Declarations
Sublist* make_sublist(int start_index, int end_index);
void swap(int *xs, int index_a, int index_b);
int partition(int *xs, int m, int n, int(*compare)(int, int));
void mymenu(int value);
void handle_reshape(int w, int h);
void recreate_subwindows();
void draw_main_window(void);
void draw_mergesort_window(void);
void draw_quicksort_window(void);
void update_lists(void);
void draw_string(char *s, void* font);
int* generate_random_list();

int main(int argc, char **argv) {
    in_intro = true;
    glutInitWindowSize(DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT);
    glutInitWindowPosition(0, 0);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    
    main_win = glutCreateWindow("Sort animations");
    glutDisplayFunc(draw_main_window);
    glutReshapeFunc(handle_reshape);

    // create menu
    glutCreateMenu(mymenu);
    glutAddMenuEntry("Animate Sorting", 1);
    glutAddMenuEntry("Exit", 2);
    glutAttachMenu(GLUT_LEFT_BUTTON);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    glutAttachMenu(GLUT_MIDDLE_BUTTON);
        
    glutMainLoop();
    
    return EXIT_SUCCESS;
}

/**
 * Creates a new Sublist struct for the use of quicksort. A Sublist represents
 * a part of the original list that's being sorted. In terms of state it only
 * has a start index and end index (since our quicksort algorithm sorts in
 * place).
 *
 * @param start_index - the first index of the original list that the sublist
 * represents.
 * @param end_index - the last index of the original list that the sublist
 * represents.
 */
Sublist* make_sublist(int start_index, int end_index) {
    Sublist* new_sublist = malloc(sizeof(Sublist));
    new_sublist->start_index = start_index;
    new_sublist->end_index = end_index;
    return new_sublist;
}

/**
 * Swaps two elements in an array, identified by two indices. Swap operation
 * is done in place.
 *
 * @param index_a - the first index.
 * @param index_b - the second index.
 */
void swap(int *xs, int index_a, int index_b) {
    int tmp = xs[index_a];
    xs[index_a] = xs[index_b];
    xs[index_b] = tmp;
}

/**
 * Partitions the sublist xs[m...n] around a pivot whose index is then
 * returned. The index is the pivots index in xs, NOT xs[m...n].
 *
 * @param xs - the array whose sublist we are going to partition.
 * @param m - the start index.
 * @param n - the end index.
 * @param compare(int, int) - the comparison function.
 */
int partition(int *xs, int m, int n, int(*compare)(int, int)) {
    if (n - m < 1) return 0;
    else {
        int init_pivot_index = m;
        int pivot = xs[init_pivot_index];
        swap(xs, init_pivot_index, n);
        int final_pivot_index = m;
        for (int i = m; i < n; i++) {
            if (compare(xs[i], pivot) <= 0) {
                swap(xs, i, final_pivot_index);
                final_pivot_index++;
            }
        }
        swap(xs, final_pivot_index, n);
        return final_pivot_index;
    }
}

/**
 * Menu function that handles the event when someone clicks "Animate sorting"
 * or "Exit." In the case of the former, we will set up the subwindows.
 *
 * @param value - the menu item clicked.
 */
void mymenu(int value) {
    if (value == 1) {
        mergesort_counter = 0; quicksort_counter = 0;
        xs_mergesort = generate_random_list();
        xs_quicksort = malloc(xs_size * sizeof(int));
        cp_array(xs_mergesort, xs_quicksort, xs_size);
        
        // push onto the mergesort stack
        mergesort_stack = make_stack();
        int *initial_array = malloc(xs_size * sizeof(int));
        cp_array(xs_mergesort, initial_array, xs_size);
        Node *initial_node = make_node(initial_array, xs_size, unsorted);
        initial_node->start_index = 0;
        initial_node->end_index = xs_size - 1;
        push(mergesort_stack, initial_node);

        // push onto the quicksort stack
        quicksort_stack = make_stack();
        Sublist *initial_list = make_sublist(0, xs_size - 1);        
        push(quicksort_stack, initial_list);

        recreate_subwindows();
        
        glutIdleFunc(update_lists);
        
        in_intro = false;
    }
    if (value == 2)
        exit(0);
}

// Reshape event handler.
void handle_reshape(int w, int h) {
    win_width = w; 
    win_height = h;
    if (!in_intro) {
        // We recreate the subwindows so we can change their size.
        glutDestroyWindow(upper_win);
        glutDestroyWindow(lower_win);
        recreate_subwindows();
    }
}

// Sets the window height globals and creates the subinwdows in glut.
void recreate_subwindows() {
    upper_win_height = win_height / 2;
    lower_win_height = win_height - upper_win_height;
    upper_win = glutCreateSubWindow(main_win, 0, 0, win_width,
        upper_win_height);
    glutDisplayFunc(draw_mergesort_window);
    lower_win = glutCreateSubWindow(main_win, 0, win_height / 2, win_width,
        lower_win_height);
    glutDisplayFunc(draw_quicksort_window);
}

// Draws the main window (black background, appropriate text).
void draw_main_window(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    if (in_intro) {
        glColor3f(1.0f, 1.0f, 1.0f);
        glRasterPos2f(-0.9f, -0.85f);
        char msg[MAX_STRING_LENGTH] = "Welcome. Click anywhere to begin.";
        draw_string(msg, GLUT_BITMAP_HELVETICA_18);
    }
    
    glutSwapBuffers();
}

// Draws the mergesort subwindow.
void draw_mergesort_window(void) {
    GLubyte fb[upper_win_height][win_width][3];
    bzero(fb, (win_width * upper_win_height * 3) * sizeof(GLubyte));
    
    /* We'll increase the column we're placing the pixel at by a factor of
     * (window_width / size of the array).
     */
    double factor = (double)win_width / xs_size;
    for (double w = 0.0; w < win_width; w = w + factor) {
        double pos = w / factor; // will always be an integer
        int col_height = upper_win_height * (xs_mergesort[(int)pos] /
            (double)xs_highest); 
        for (int h = 0; h < col_height; h++) {
            fb[h][(int)w][0] = 255;
        }
    }
    glWindowPos2s(0, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glDrawPixels(win_width, upper_win_height, GL_RGB, GL_UNSIGNED_BYTE, fb);
    
    glFlush();
    
    // text
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(-0.95f, 0.75f);
    char label[MAX_STRING_LENGTH] = "Mergesort";
    draw_string(label, GLUT_BITMAP_HELVETICA_18);
    
    glRasterPos2f(-0.95f, -0.75f);
    char counter[MAX_STRING_LENGTH];
    sprintf(counter, "Merges: %d", mergesort_counter);
    draw_string(counter, GLUT_BITMAP_8_BY_13);
    
    glutSwapBuffers();
}

// Draws the quicksort subwindow.
void draw_quicksort_window(void) {
    GLubyte fb[lower_win_height][win_width][3];
    bzero(fb, (win_width * lower_win_height * 3) * sizeof(GLubyte));
    
    double factor = (double)win_width / xs_size;
    for (double w = 0.0; w < win_width; w = w + factor) {
        double pos = w / factor; // will always be an integer
        int col_height = lower_win_height * (xs_quicksort[(int)pos] /
         (double)xs_highest);
        for (int h = 0; h < col_height; h++) {
            fb[h][(int)w][2] = 255;
        }
    }
    glWindowPos2s(0, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glDrawPixels(win_width, lower_win_height, GL_RGB, GL_UNSIGNED_BYTE, fb);
    
    glFlush();
    
    // text
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(-0.95f, 0.75f);
    char label[MAX_STRING_LENGTH] = "Quicksort";
    draw_string(label, GLUT_BITMAP_HELVETICA_18);
    
    glRasterPos2f(-0.95f, -0.75f);
    char counter[MAX_STRING_LENGTH];
    sprintf(counter, "Partitions: %d", quicksort_counter);
    draw_string(counter, GLUT_BITMAP_8_BY_13);
    
    glutSwapBuffers();
}

/*
 * Our idle function. Does a step of mergesort and quicksort, and calls the
 * display functions.
 *
 */
void update_lists(void) {
    // Mergesort part.
    // we only want to display if we've performed a merge.
    bool performed_a_merge = false;
    // if mergesort isn't finished, perform a step of it.
    if (!stk_is_empty(mergesort_stack)) {
        Node *cur_node = pop(mergesort_stack);
        State cur_state = cur_node->state;
        if (cur_state == unsorted) {
            size_t cur_size = cur_node->size;
            // If length 0 or 1, change state to sorted and put back on stack.
            if (cur_size == 1 || cur_size == 0) {
                cur_node->state = sorted;
                push(mergesort_stack, cur_node);
            }
            else {
                // Split and place back on stack.
                // Make two new arrays, and fill them.
                size_t first_array_size = (cur_size / 2);
                size_t second_array_size = (cur_size - first_array_size);
                int *first = malloc(first_array_size * sizeof(int));
                int *second = malloc(second_array_size * sizeof(int));
                half(cur_node->array, first, first_array_size, second,
                    second_array_size);
                // Push new arrays on stack as nodes.
                Node *first_node = make_node(first, first_array_size,
                    unsorted);
                int cur_start_index = cur_node->start_index;
                first_node->start_index = cur_start_index;
                first_node->end_index = cur_start_index + first_array_size - 1;
                push(mergesort_stack, first_node);
                Node *second_node = make_node(second, second_array_size,
                    unsorted);
                second_node->start_index = cur_start_index + first_array_size;
                second_node->end_index = cur_node->end_index;
                push(mergesort_stack, second_node);

                free_node(cur_node);
            }
        } else if (cur_state == sorted) {
            if (!stk_is_empty(mergesort_stack)) {
                // If stack isn't empty, get next_node and merge or swap.
                Node *next_node = pop(mergesort_stack);
                // If next_node's sorted, merge them together; else switch
                // them.
                if (next_node->state == sorted) {
                    // Merge them into a new node.
                    size_t first_half_size = cur_node->size;
                    size_t second_half_size = next_node->size;
                    size_t merged_size = first_half_size + second_half_size;
                    int *merged_array = malloc(merged_size * sizeof(int));
                    merge(cur_node->array, first_half_size, next_node->array,
                        second_half_size, merged_array, compare);
                    Node *merged_node = make_node(merged_array, merged_size,
                        sorted);
                    merged_node->start_index = cur_node->start_index;
                    merged_node->end_index = next_node->end_index;
                    push(mergesort_stack, merged_node);
                    free_node(cur_node);
                    free_node(next_node);
                    performed_a_merge = true;
                } else if (next_node->state == unsorted) {
                    // Swap position of nodes.
                    push(mergesort_stack, cur_node);
                    push(mergesort_stack, next_node);
                } else {
                    printf("Unrecognized state.");
                }
            } else {
                free_node(cur_node);
            }
        } else {
            printf("Unrecognized state.");
        }
        // If we performed a merge, the newly merged element should be at the
        // top of the stack.
        if (performed_a_merge) {
            mergesort_counter++;
            Node *mergedNode = peek(mergesort_stack);
            int *cur_array = mergedNode->array;
            int start_index = mergedNode->start_index;
            int end_index = mergedNode->end_index;
            // Put the merged array into the appropriate place in xs_mergesort.
            for (int i = start_index; i <= end_index; i++) {
                xs_mergesort[i] = cur_array[i - start_index];
            }
            glutSetWindow(upper_win);
            glutPostRedisplay();
        }
    }
    /* We want to run and display quicksort if either mergesort is done or if
     * we just performed a merge. We could also run quicksort regardless of
     * what the mergesort algorithm did, but this way we can compare how the
     * display iterations of the algorithms look since they're running at
     * the same time.
     */
    if (performed_a_merge || stk_is_empty(mergesort_stack)) {
    // if (true) {
        if (!stk_is_empty(quicksort_stack)) {
            Sublist *current_sublist = pop(quicksort_stack);
            int start_index = current_sublist->start_index;
            int end_index = current_sublist->end_index;
            int pivot_index = partition(xs_quicksort, start_index, end_index,
                compare);
            // Create two sublists and push them on the stack.
            int left_start_index = start_index;
            int left_end_index = pivot_index - 1;
            if (left_end_index - left_start_index >= 1) {
                Sublist *left_list = malloc(sizeof(Sublist));
                left_list->start_index = left_start_index;
                left_list->end_index = left_end_index;
                push(quicksort_stack, left_list);
            }
            int right_start_index = pivot_index + 1;
            int right_end_index = end_index;
            if (right_end_index - right_start_index >= 1) {
                Sublist *right_list = malloc(sizeof(Sublist));
                right_list->start_index = right_start_index;
                right_list->end_index = right_end_index;
                push(quicksort_stack, right_list);
            }
            free(current_sublist);
            quicksort_counter++;
        }
        // Partition happens at every step, so we'll always display.
        glutSetWindow(lower_win);
        glutPostRedisplay();
    }
}

// Draws the string specified in char array s, using font.
void draw_string(char *s, void *font) {
    for (int i = 0; i < strlen(s); i++)
        glutBitmapCharacter(font, s[i]);
}

// Generates a malloc'd random list and returns it.
int* generate_random_list() {
    xs_size = win_width;
    int *xs = malloc(xs_size * sizeof(int));
    xs_highest = 0;
    for (int i = 0; i < xs_size; i++) {
        int random = RAND(0, 1000);
        if (random > xs_highest) xs_highest = random;
        xs[i] = random;
    }
    return xs;
}
