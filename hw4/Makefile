# Professor Danner
# Computer Graphics 356
# Homework #4
# Evan Carmi (WesID: 807136)
# ecarmi@wesleyan.edu
include comp356.mk

EXECUTABLES=hw4

LIBS=-l356

HW4_DEPENDENCIES=hw4.o maze.o debug.h

SOLUTION_FILES=hw4.o maze.o debug.h


hw4 : $(HW4_DEPENDENCIES)
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) $^ $(LDFLAGS) -l356

hw3 : hw3.c debug.h
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) hw3.c $(LDFLAGS) $(LIBS)

show_maze2d : show_maze2d.c maze.c
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) show_maze2d.c maze.c $(LDFLAGS) $(LIBS)

clean :
	rm -f *.o $(EXECUTABLES)

solution : $(SOLUTION_FILES)
	tar czvf hw4.tar.gz $(SOLUTION_FILES)
