.PHONY: all myfork run clean
all: run
myfork: main.c
	gcc main.c -o myfork
run: myfork
	./myfork
clean:
	rm -f myfork
