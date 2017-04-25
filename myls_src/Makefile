OBJS = myls.o
CC = gcc
CFLAGS = -c -w -g -std=gnu99

all: myls

myls: $(OBJS)
	$(CC) -g $(OBJS) -o myls

myls.o: myls.c
	$(CC) $(CFLAGS) myls.c

clean:
	rm -f myls $(OBJS)
