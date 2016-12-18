hw4: c.c main.c 
	gcc -O0 -g c.c main.c -o hw4 --std=gnu99

debug: c.c debug_main.c
	gcc -O0 -g c.c debug_main.c -o hw4 --std=gnu99
