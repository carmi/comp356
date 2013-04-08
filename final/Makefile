# Professor Danner
# Computer Graphics 356
# Final Project
# Evan Carmi (WesID: 807137)
# ecarmi@wesleyan.edu
include comp356.mk

# OPTIONS FOR MORE are: chess | cube | sphere | spheres | wall
CPPFLAGS2=-DMORE=chess

EXECUTABLES=final

LIBS=-l356

FINAL_DEPENDENCIES=final.c surface.c surfaces_lights.c

SOLUTION_FILES=final.c surface.h surface.c surfaces_lights.h surfaces_lights.c color.h debug.h Makefile

walls : $(FINAL_DEPENDENCIES)
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) -DMORE=$@ $^ $(LDFLAGS) -l356
spheres : $(FINAL_DEPENDENCIES)
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) -DMORE=$@ $^ $(LDFLAGS) -l356
sphere : $(FINAL_DEPENDENCIES)
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) -DMORE=$@ $^ $(LDFLAGS) -l356
sphere3 : $(FINAL_DEPENDENCIES)
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) -DMORE=$@ $^ $(LDFLAGS) -l356
cube : $(FINAL_DEPENDENCIES)
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) -DMORE=$@ $^ $(LDFLAGS) -l356
chess : $(FINAL_DEPENDENCIES)
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) -DMORE=$@ $^ $(LDFLAGS) -l356
transcube : $(FINAL_DEPENDENCIES)
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) -DMORE=$@ $^ $(LDFLAGS) -l356
rg : $(FINAL_DEPENDENCIES)
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) -DMORE=$@ $^ $(LDFLAGS) -l356

final : $(FINAL_DEPENDENCIES)
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) $^ $(LDFLAGS) -l356

final : $(FINAL_DEPENDENCIES)
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) $^ $(LDFLAGS) -l356

clean :
	rm -f *.o $(EXECUTABLES)

solution : $(SOLUTION_FILES)
	tar czvf final.tar.gz $(SOLUTION_FILES)
