20161622.out: 20161622.o assembler.o 
	gcc -o 20161622.out 20161622.o assembler.o
20161622.o: 20161622.c 20161622.h main.h assembler.h 
	gcc -Wall -c 20161622.c -o 20161622.o
assembler.o: assembler.c assembler.h main.h 20161622.h 
	gcc -Wall -c assembler.c -o assembler.o
clean: 
	rm 20161622.out 20161622.o assembler.o
