OBJS = mycat.o
CC = gcc
CFLAGS = -c -g -w

all : mycat

mycat: $(OBJS)
	$(CC) -g $(OBJS) -o mycat

mycat.o: mycat.c
	$(CC) $(CFLAGS) mycat.c

clean:
	rm -f mycat $(OBJS)
