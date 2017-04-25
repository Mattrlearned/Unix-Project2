OBJS = mycp.o
CC = gcc
CFLAGS = -c -w -g -std=gnu99

all: mycp

mycp: $(OBJS)
	$(CC) -g $(OBJS) -o mycp

mycp.o: mycp.c
	$(CC) $(CFLAGS) mycp.c

clean:
	rm -f mycp $(OBJS)

