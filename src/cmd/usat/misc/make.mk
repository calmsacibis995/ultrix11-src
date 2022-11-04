
# test file for the make command
all:	clean prog clean

prog:	prog.c
	cp prog.c prog2.c

prog.c: prog1.c

prog1.c:
	touch prog1.c
	touch prog.c
clean:
	rm -f prog prog.o prog.c prog1.c prog2.c
