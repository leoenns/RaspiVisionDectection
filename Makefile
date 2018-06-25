CFLAGS  += -I=/usr/include/cifx

all:
	$(CC) ethercat.c $(CFLAGS) -lcifx -lpthread -lrt -o ethercat 
clean:
	rm -f *.o ethercat
