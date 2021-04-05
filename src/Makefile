CFLAGS=-std=c11 -g -o0
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

kmcc: $(OBJS)
			$(CC) -no-pie -o kmcc $(OBJS) $(LDFLAGS)

$(OBJS): kmcc.h

kmcc-gen2: kmcc $(SRCS) kmcc.h
	./self.sh

extern.o: tests-extern
	gcc -xc -c -o extern.o tests-extern

test: kmcc extern.o
		./kmcc tests > tmp.s
		gcc -static -o tmp tmp.s extern.o
		./tmp

test-gen2: kmcc-gen2 extern.o
	./kmcc-gen2 tests > tmp.s
	gcc -static -o tmp tmp.s extern.o
	./tmp

clean:
		rm -rf kmcc kmcc-gen* *.o *~ tmp*

.PHONY: test clean