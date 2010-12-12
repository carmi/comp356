include comp356.mk

LIBS=-l356

%.o : %.c %.h
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) $<

hw2bp1.o : hw2bp1.c surface.h surfaces_lights.h
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) -c hw2bp1.c

hw2bp1 : hw2bp1.o surface.o surfaces_lights.o 
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) $^ $(LDFLAGS) $(LIBS)

clean :
	rm -f *.o hw2bp1

