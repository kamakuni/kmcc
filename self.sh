#!/bin/bash -x
TMP=tmp-self

mkdir -p $TMP

expand() {
    file=$1
    cat <<EOF > $TMP/$1
typedef struct FILE FILE;
extern FILE *stdout;
extern FILE *stderr;

void *malloc(long size);
void *calloc(long nmemb, long size);
int *__errno_location();
char *strerror(int errnum);
FILE *fopen(char *pathname, char *mode);
long fread(void *ptr, lonf size, long nmemb, FILE *stream);
int feof(FILE *stream);
static void assert() {}
int strcmp(char *s1, *s2);
EOF

    
}