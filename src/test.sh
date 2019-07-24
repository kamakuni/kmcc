#! /bin/bash
try() {
    expected="$1"
    input="$2"

    ./compiler "$input" > tmp.s
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

try 21 "main(){ 5+20-4;}"
try 41 "main(){   12 + 34 - 5 ;}"
try 47 "main(){ 5+6*7;}"
try 15 "main(){ 5* 3;}"
try 7 "main(){ 10 /2 + 2;}"
try 15 "main(){ 5*(9-6);}"
try 4 "main(){ (3+5)/2;}"
try 5 "main(){ -10+15;}"
try 3 "main(){ -(5+2)+10;}"

try 0 "main(){ 1 < 1;}"
try 0 "main(){2 < 1;}"
try 1 "main(){1 < 2;}"
try 0 "main(){1 > 1;}"
try 1 "main(){2 > 1;}"
try 0 "main(){1 > 2;}"
try 1 "main(){1 <= 1;}"
try 0 "main(){2 <= 1;}"
try 1 "main(){1 <= 2;}"
try 1 "main(){1 >= 1;}"
try 1 "main(){2 >= 1;}"
try 0 "main(){1 >= 2;}"
try 1 "main(){5 == 2 + 3;}"
try 0 "main(){2 + 3 != 5;}"

try 30 "main(){a=30;}"
try 24 "main(){a=9+15;}"
try 3 "main(){1;2;3;}"
try 3 "main(){a=1;b=2;a+b;}"
try 4 "main(){a=b=2;c=a+b;c;}"

try 5 "main(){return 5;}"
try 7 "main(){a=4+3; return a;}"

try 6 "main(){foo=1;bar=2+3;return foo + bar;}"

try 2 "main(){if(1<10)2;}"
try 10 "main(){var=1;if(1<10)var=10;var;}"
try 1 "main(){var=1;if(1>10)var=10;var;}"

try 2 "main(){if(1<10) 2; else 3;}"
try 3 "main(){if(11<10) 2; else 3;}"

try 10 "main(){i=0;while(i<10)i=i+1; return i;}"
try 0 "main(){i=0;while(i<10)if(i==0) return i; return 11;}"
try 5 "main(){i=0;while(i<10)if(i<5) i=i+1; else return i; return 11;}"

try 20 "main(){a=10;for(i=0;i<10;i=i+1)a=a+1; return a;}"
try 11 "main(){i=0;for(;i<=10;)i=i+1;return i;}"
try 0 "main(){i=0;for(;i<=10;)if(i==0) return i; return 11;}"
try 5 "main(){i=0;for(;i<=10;)if(i<5) i=i+1; else return i; return 11;}"
try 5 "main(){i=0;for(;i<=10;)if(i<5) i=i+1; else return i; ret=11;}"

try 10 "func(){10;}main(){func();}"
try 10 "func(){return 10;}main(){func();}"
try 10 "func(){return 10;}main(){return func();}"
try 10 "three(){return 3;}seven(){return 7;}main(){return three() + seven();}"

try 10 "func(a){a;}main(){func(10);}"
try 10 "func(a,b){b;}main(){func(0,10);}"
try 10 "func(a,b){a;}main(){func(10,0);}"
try 10 "func(a){if(a==1) 10; else 11;}main(){func(1);}"
try 10 "func(a){if(a==1) 11; else 10;}main(){func(0);}"

try 10 "func(a,b){a + b;}main(){func(3,7);}"
try 10 "func(a,b){return a + b;}main(){func(3,7);}"
try 10 "func(a,b){return a + b;}main(){return func(3,7);}"
try 10 "func(a,b){return a + b;}main(){return func(2*2,2+4);}"
try 1  "func(a){if(a == 5) 1; else 0;} main(){return func(5);}"
try 1  "func(a){return a = a + 1;} main(){return func(0);}"

echo OK
