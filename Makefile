#this makefile will compile components of the project (i.e. mycat and myls)
#if a member is working on multiple components, it will compile all these components
#source code must be contained in the ./src folder for this to work

SOURCES = $(wildcard src/*.c)
OBJS = $(notdir $(patsubst %.c,%.o,$(wildcard ./src/*.c)))
PROGS = $(notdir $(basename $(SOURCES)))
CC= gcc

all: $(PROGS)

$(OBJS): %.o: ./src/%.c
	gcc -g -w -c $< -o $@

clean:
	rm -rf $(OBJS) $(PROGS)
