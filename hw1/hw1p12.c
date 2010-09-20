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

void merge_sort_rec(int *xs, size_t size, int(*compare)(int, int));
void merge_sort_itr(int *xs, size_t size, int(*compare)(int, int));

void merge_sort_rec(int *xs, size_t size, int(*compare)(int, int)) {
    
    
}

void merge_sort_itr(int *xs, size_t size, int(*compare)(int, int)) {
    
    
}