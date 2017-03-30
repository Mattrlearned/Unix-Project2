#this makefile will compile components of the project (i.e. mycat and myls)
#if a member is working on multiple components, it will compile all these components
#source code must be contained in the ./src folder for this to work

SOURCES = $(wildcard src/*.c)
OBJS = $(notdir $(patsubst %.c,%.o,$(wildcard ./src/*.c)))
PROGS = $(notdir $(basename $(SOURCES)))

#mysh : mysh.c
#	gcc mysh.c -o mysh

$(PROGS) : $(OBJS)
	gcc -o $@ $^

$(OBJS): %.o: ./src/%.c
	gcc -g -c $< -o $@

clean:
	rm -rf $(OBJS) $(PROGS)
