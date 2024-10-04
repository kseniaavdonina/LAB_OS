.PHONY: all myls run clean
all: run
myls: main.c
	gcc main.c -o myls
	
run: myls
	./myls
	
clean:
	rm -f myls
