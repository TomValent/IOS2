
C=gcc
CFLAGS=-std=gnu99 -Wall -Wextra -pedantic -pthread

proj2: proj2.o
	$(C) $(CFLAGS) proj2.o -o proj2

proj2.o: proj2.c
	$(C) $(CFLAGS) -c proj2.c -o proj2.o

clean:
	rm *.o proj2 -f

zip:
	zip xvalen27.zip *.c *.h Makefile