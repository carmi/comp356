# Professor Danner
# Computer Graphics 356
# Final Project
# Evan Carmi (WesID: 807136)
# ecarmi@wesleyan.edu
include comp356.mk

EXECUTABLES=final

LIBS=-l356

FINAL_DEPENDENCIES=final.c surface.c surfaces_lights.c debug.h

SOLUTION_FILES=final.c surface.h surface.c surfaces_lights.h surfaces_lights.c debug.h Makefile

final : $(FINAL_DEPENDENCIES)
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) $^ $(LDFLAGS) -l356

clean :
	rm -f *.o $(EXECUTABLES)

solution : $(SOLUTION_FILES)
	tar czvf hw4.tar.gz $(SOLUTION_FILES)
