include comp356.mk

LIBS=-l356

%.o : %.c %.h
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) $<

final.o : final.c surface.h surfaces_lights.h
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) -c final.c

chess : final.o surface.o chess.o 
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) $^ $(LDFLAGS) $(LIBS)

sphere : final.o surface.o sphere_on_table.o 
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) $^ $(LDFLAGS) $(LIBS)

cube : final.o surface.o cube_on_table.o 
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) $^ $(LDFLAGS) $(LIBS)

final : final.o surface.o surfaces_lights_final.o 
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) $^ $(LDFLAGS) $(LIBS)


clean :
	rm -f *.o final

