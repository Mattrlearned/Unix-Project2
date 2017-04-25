TRGTS = mycat mycp myls mysh
CFLAGS = -c -w -g -std=gnu99
all: $(TRGTS)

mycat: ./mycat_src/mycat.c
	cd ./mycat_src; make ; cp mycat ../.

mycp: ./mycp_src/mycp.c
	cd ./mycp_src; make ; cp mycp ../.

myls: ./myls_src/myls.c
	cd ./myls_src; make ; cp myls ../.

mysh.o: mysh.c textProc.h subshell.h
	gcc $(CFLAGS) mysh.c -o mysh.o

mysh: mysh.o
	gcc mysh.o -o mysh

clean:
	rm -f *.o $(TRGTS)
	cd ./mycat_src; make clean
	cd ./mycp_src; make clean
	cd ./myls_src; make clean
