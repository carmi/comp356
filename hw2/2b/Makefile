# Professor Danner
# Computer Graphics 356
# Homework #1
# Evan Carmi (WesID: 807136) and Carlo Francisco (WesID: 774066)
# ecarmi@wesleyan.edu and jfrancisco@wesleyan.edu
include comp356.mk

EXECUTABLES=hw2bp1
OBJS=ms_driver.o hw1p12.o hw1p3.o

SOLUTION_FILES=Makefile hw2bp1.c

HW2BP1_DEPENDENCIES=hw2bp1.o color.h debug.h Makefile surface.c surface.h \
		   surfaces_lights.c surfaces_lights.h

hw2bp1 : $(HW2BP1_DEPENDENCIES)
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) $^ $(LDFLAGS) -l356

solution : $(SOLUTION_FILES)
	tar cvf hw2b.tar.gz $(SOLUTION_FILES)

clean :
	rm -rf $(EXECUTABLES)
	rm -rf *.o
