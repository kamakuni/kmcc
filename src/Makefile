CFLAGS=-Wall -std=c11 -g -o0
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

compiler: $(OBJS)
			$(CC) -o compiler $(OBJS) $(LDFLAGS)

$(OBJS): compiler.h

test: compiler
		./compiler -test
		./test.sh

clean:
		rm -f compiler *.o *~ tmp*
