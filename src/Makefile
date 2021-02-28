CFLAGS=-std=c11 -g -o0
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

kmcc: $(OBJS)
			$(CC) -no-pie -o kmcc $(OBJS) $(LDFLAGS)

$(OBJS): kmcc.h

test: kmcc
		./kmcc tests > tmp.s
		gcc -static -o tmp tmp.s
		./tmp

clean:
		rm -f kmcc *.o *~ tmp*

debug: $(OBJS)
			$(CC) -g3 -O0 -o kmcc $(OBJS)
