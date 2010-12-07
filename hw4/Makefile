# Professor Danner
# Computer Graphics 356
# Homework #4
# Evan Carmi (WesID: 807136)
# ecarmi@wesleyan.edu
include comp356.mk

EXECUTABLES=hw4

LIBS=-l356

HW4_DEPENDENCIES=hw4.o maze.o debug.h

SOLUTION_FILES=hw4.c maze.c maze.h debug.h Makefile

hw4 : $(HW4_DEPENDENCIES)
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) $^ $(LDFLAGS) -l356

clean :
	rm -f *.o $(EXECUTABLES)

solution : $(SOLUTION_FILES)
	tar czvf hw4.tar.gz $(SOLUTION_FILES)
