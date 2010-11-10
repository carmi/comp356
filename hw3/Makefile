# Professor Danner
# Computer Graphics 356
# Homework #3
# Evan Carmi (WesID: 807136)
# ecarmi@wesleyan.edu
include comp356.mk

SOLUTION_FILES = hw3.c Makefile debug.h hw3.pdf

LIBS=-l356

hw3 : hw3.c debug.h
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) hw3.c $(LDFLAGS) $(LIBS)

clean :
	rm -f *.o hw3
solution : $(SOLUTION_FILES)
	tar czvf hw3.tar.gz $(SOLUTION_FILES)
