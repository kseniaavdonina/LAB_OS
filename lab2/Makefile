.PHONY: all mycat mygrep run clean
all: run
mycat: mycat.c
	gcc mycat.c -o mycat
mygrep: mygrep.c
	gcc mygrep.c -o mygrep
run: mycat mygrep
	./mycat 
	./mygrep
clean:
	rm -f mycat mygrep
