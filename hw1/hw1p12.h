/** @file hw1p12.h
 *  @brief The header file for hw1p12.c.
 *
 *  Professor Danner
 *  Computer Graphics 356
 *  Homework #1
 *  Evan Carmi (WesID: 807136) and Carlo Francisco (WesID: 774066)
 *  ecarmi@wesleyan.edu and jfrancisco@wesleyan.edu
 */
#include <stdlib.h>

void merge_sort_rec(int *xs, size_t size, int(*compare)(int, int));
void merge_sort_itr(int *xs, size_t size, int(*compare)(int, int));
