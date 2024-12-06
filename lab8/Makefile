.PHONY: all mtx run clean
all: run
mtx: mtx.c
	gcc mtx.c -lpthread -o mtx
	
run: mtx
	./mtx
	
clean:
	rm -f mtx
