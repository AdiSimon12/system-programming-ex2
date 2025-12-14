all: my_copy

my_copy: my_copy.c
	gcc -Wall my_copy.c -o my_copy

clean:
	rm -f my_copy