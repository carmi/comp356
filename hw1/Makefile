include comp356.mk

ms_driver : ms_driver.o hw1p12.h hw1p12.o hw1utils.h hw1utils.o
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) $^ $(LDFLAGS) -l356

hw1p3 : hw1p3.o hw1utils.h hw1utils.o
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) $^ $(LDFLAGS) -l356