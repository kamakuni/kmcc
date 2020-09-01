#! /bin/bash
try() {
    expected="$1"
    input="$2"

    ./kmcc "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$expected expected, but got $actual"
        exit 1
    fi

}

try 21 "int main(){ return 5+20-4;}"
try 41 "int main(){ return  12 + 34 - 5 ;}"
try 47 "int main(){ return 5+6*7;}"
try 15 "int main(){ return 5* 3;}"
try 7 "int main(){ return 10 /2 + 2;}"
try 15 "int main(){ return 5*(9-6);}"
try 4 "int main(){ return (3+5)/2;}"
try 5 "int main(){ return -10+15;}"
try 3 "int main(){ return -(5+2)+10;}"
try 10 "int main(){ return -10+20;}"
try 10 "int main(){ return - -10;}"
try 10 "int main(){ return - - +10;}"

try 0 "int main(){ return 1 < 1;}"
try 0 "int main(){ return 2 < 1;}"
try 1 "int main(){ return 1 < 2;}"
try 0 "int main(){ return 1 > 1;}"
try 1 "int main(){ return 2 > 1;}"
try 0 "int main(){ return 1 > 2;}"
try 1 "int main(){ return 1 <= 1;}"
try 0 "int main(){ return 2 <= 1;}"
try 1 "int main(){ return 1 <= 2;}"
try 1 "int main(){ return 1 >= 1;}"
try 1 "int main(){ return 2 >= 1;}"
try 0 "int main(){ return 1 >= 2;}"
try 1 "int main(){ return 5 == 2 + 3;}"
try 0 "int main(){ return 2 + 3 != 5;}"

try 30 "int main(){int a;a=30; return a;}"
try 24 "int main(){int a;a=9+15; return a;}"
try 2 "int main(){1; return 2;3;}"
try 3 "int main(){int a;int b;a=1;b=2; return a+b;}"
try 4 "int main(){int a;int b;int c;a=b=2;c=a+b; return c;}"

try 5 "int main(){return 5;}"
try 7 "int main(){int a; a=4+3; return a ;}"

try 6 "int main(){int foo; int bar;foo=1;bar=2+3;return foo + bar;}"

try 2 "int main(){if(1<10) return 2;}"
try 2 "int main(){if(1<10){return 2;}}"
try 2 "int main(){if(1<10){int a;a=2;return a;}}"
try 10 "int main(){int var; var=1;if(1<10) return var=10; return var;}"
try 1 "int main(){int var; var=1;if(1>10) return var=10; return var;}"

try 2 "int main(){if(1<10) return 2; else return 3;}"
try 3 "int main(){if(11<10) return 2; else return 3;}"
try 2 "int main(){if(1<10) {return 2;} else {return 3;}}"
try 3 "int main(){if(11<10) {return 2;} else {return 3;}}"

try 10 "int main(){int i; i=0;while(i<10)i=i+1; return i;}"
try 0 "int main(){int i; i=0;while(i<10)if(i==0) return i; return 11;}"
try 5 "int main(){int i; i=0;while(i<10)if(i<5) i=i+1; else return i; return 11;}"
try 10 "int main(){int i; i=0;while(i<10){i=i+1;} return i;}"

try 55 "int main() { int i=0; int j=0; for (i=0; i<=10; i=i+1) j=i+j; return j; }"
try 20 "int main(){int i;int a; a=10;for(i=0;i<10;i=i+1)a=a+1; return a;}"
try 11 "int main(){int i; i=0;for(;i<=10;)i=i+1;return i;}"
try 0 "int main(){int i; i=0;for(;i<=10;)if(i==0) return i; return 11;}"
try 5 "int main(){int i; i=0;for(;i<=10;)if(i<5) i=i+1; else return i; return 11;}"
try 5 "int main(){int i; i=0;for(;i<=10;)if(i<5) i=i+1; else return i;int ret; ret=11;}"

try 10 "int func(){return 10;}int main(){func();}"
try 10 "int func(){return 10;}int main(){func();}"
try 10 "int func(){return 10;}int main(){return func();}"
try 10 "int three(){return 3;}int seven(){return 7;}int main(){return three() + seven();}"

try 10 "int func(int a){a;}int main(){func(10);}"
try 10 "int func(int a,int b){b;}int main(){func(0,10);}"
try 10 "int func(int a,int b){a;}int main(){func(10,0);}"
try 10 "int func(int a){if(a==1) return 10; else return 11;}int main(){func(1);}"
try 10 "int func(int a){if(a==1) return 11; else return 10;}int main(){func(0);}"

try 10 "int func(int a,int b){a + b;}int main(){func(3,7);}"
try 10 "int func(int a,int b){return a + b;}int main(){func(3,7);}"
try 10 "int func(int a,int b){return a + b;}int main(){return func(3,7);}"
try 10 "int func(int a,int b){return a + b;}int main(){return func(2*2,2+4);}"
try 1  "int func(int a){if(a == 5) 1; else 0;}int main(){return func(5);}"
try 1  "int func(int a){return a = a + 1;}int main(){return func(0);}"

try 55 "int fib(int n){if(n==0){return 0;} if(n==1){return 1;} return fib(n-2) + fib(n-1);}int main(){return fib(10);}"

try 3 "int main(){int x;int *y;y = &x;*y = 3;return x;}"
try 3 "int main(){int x=3; return *&x;}"
try 3 "int main(){int x=3; int *y=&x; int **z=&y; return **z;}"
try 5 "int main(){int x=3; int y=5; return *(&x+1);}"
try 5 "int main(){int x=3; int y=5; return *(1+&x);}"
try 3 "int main(){int x=3; int y=5; return *(&y-1);}"
try 2 "int main(){int x=3; return (&x+2)-&x;}"
try 5 "int main(){int x=3; int y=5; int *z=&x; return *(z+1);}"
try 3 "int main(){int x=3; int y=5; int *z=&y; return *(z-1);}"
try 5 "int main(){int x=3; int *y=&x; *y=5; return x;}"
try 7 "int main(){int x=3; int y=5; *(&x+1)=7; return y;}"
try 7 "int main(){int x=3; int y=5; *(&y-1)=7; return x;}"
try 8 "int main(){int x=3; int y=5; return foo(&x,y);} int foo(int *x, int y){ return *x + y;}"

try 4 "int main(){int x=3; return sizeof(x);}"
try 4 "int main(){int x=3; return sizeof x;}"
try 8 "int main(){int x=3; return sizeof(&x);}"

try 3 "int main(){ int x[2]; int *y=&x; *y=3; return *x;}"
try 3 "int main(){ int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *p + *(p + 1);}"

try 3 "int main(){ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; return *x; }"
try 4 "int main(){ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; return *(x+1); }"
try 5 "int main(){ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; return *(x+2); }"

#try 3 "int main(){ int x[3]; x[0]=3; x[1]=4; x[2]=5; return x[0]; }"
#try 4 "int main(){ int x[3]; x[0]=3; x[1]=4; x[2]=5; return x[1]; }"
#try 5 "int main(){ int x[3]; x[0]=3; x[1]=4; x[2]=5; return x[2]; }"

echo OK
