.PHONY: all mypipe myfifo clean
all: mypipe myfifo
mypipe: pipe.c
	gcc pipe.c -o mypipe
myfifo: fifo.c
	gcc fifo.c -o myfifo
clean:
	rm -rf *.o mypipe myfifo
