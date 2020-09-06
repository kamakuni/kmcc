CFLAGS=-Wall -std=c11 -g -o0
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

kmcc: $(OBJS)
			$(CC) -o kmcc $(OBJS) $(LDFLAGS)

$(OBJS): kmcc.h

test: kmcc
		./kmcc -test
		./test.sh

clean:
		rm -f kmcc *.o *~ tmp*

debug: $(OBJS)
			$(CC) -g3 -O0 -o kmcc $(OBJS)
