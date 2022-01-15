myshell: main.c shell.c 
	gcc -Wall -Werror -pedantic -o myshell main.c shell.c -lreadline

clean:
	rm -f *.o myshell