CFLAGS=-std=c11 -g -o0
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

kmcc: $(OBJS)
			$(CC) -no-pie -o kmcc $(OBJS) $(LDFLAGS)

$(OBJS): kmcc.h

test: kmcc
		./kmcc tests > tmp.s
		echo 'int ext1; int *ext2; int char_fn() { return 257; }' \
          'int static_fn() { return 5; }' | \
		  gcc -xc -c -o tmp2.o -
		gcc -static -o tmp tmp.s tmp2.o
		./tmp

clean:
		rm -f kmcc *.o *~ tmp*

debug: $(OBJS)
			$(CC) -g3 -O0 -o kmcc $(OBJS)
