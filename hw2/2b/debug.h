/** @file debug.h
 *  @brief - Debug header file
 *  Professor Danner
 *  Computer Graphics 356
 *  Homework #2B
 *  Evan Carmi (WesID: 807136) and Carlo Francisco (WesID: 774066)
 *  ecarmi@wesleyan.edu and jfrancisco@wesleyan.edu
 *
 */#ifndef DEBUG_H
#define DEBUG_H

// A marginally-clever debugging function that expends to a no-op
// when NDEBUG is defined and otherwise prints its string formatting
// arguments to stderr.
#ifdef NDEBUG
#define debug(fmt, ...) 
#define debug_c(expr, fmt, ...)
#else

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

static void debug(const char* fmt, ...) {
    va_list argptr ;
    va_start(argptr, fmt) ;

    char* fmt_newline ;
    asprintf(&fmt_newline, "%s\n", fmt) ;
    vfprintf(stderr, fmt_newline, argptr) ;
    free(fmt_newline) ;
    fflush(stderr) ;
}
static void debug_c(bool expr, const char* fmt, ...) {
    if (expr) {
        va_list argptr ;
        va_start(argptr, fmt) ;

        char* fmt_newline ;
        asprintf(&fmt_newline, "%s\n", fmt) ;
        vfprintf(stderr, fmt_newline, argptr) ;
        free(fmt_newline) ;
        fflush(stderr) ;
    }
}
#endif

#endif
