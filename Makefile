process: sigtrap.c
	gcc -o process sigtrap.c

a2: a2.c
	gcc -o a2 pcb.c a2.c -g

clean:
	rm a2
	rm process