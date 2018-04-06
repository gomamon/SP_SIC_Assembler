20161622.out: 20161622.o SICShell.o #assembler.o
	gcc -o 20161622.out 20161622.o SICShell.o
20161622.o: 20161622.h 20161622.c
	gcc -Wall -c 20161622.c
SICShell.o: 20161622.h SICShell.c
	gcc -Wall -c SICShell.c
clean: 
	rm 20161622.out 20161622.o shell.o
