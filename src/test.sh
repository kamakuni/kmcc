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

try 21 "5+20-4;"
try 41 "  12 + 34 - 5 ;"
try 47 "5+6*7;"
try 15 "5* 3;"
try 7 "10 /2 + 2;"
try 15 "5*(9-6);"
try 4 "(3+5)/2;"
try 5 "-10+15;"
try 3 "-(5+2)+10;"

try 0 "1 < 1;"
try 0 "2 < 1;"
try 1 "1 < 2;"
try 0 "1 > 1;"
try 1 "2 > 1;"
try 0 "1 > 2;"
try 1 "1 <= 1;"
try 0 "2 <= 1;"
try 1 "1 <= 2;"
try 1 "1 >= 1;"
try 1 "2 >= 1;"
try 0 "1 >= 2;"
try 1 "5 == 2 + 3;"
try 0 "2 + 3 != 5;"

try 30 "a=30;"
try 24 "a=9+15;"
try 3 "a=1;b=2;a+b;"
try 4 "a=b=2;c=a+b;c;"

try 5 "return 5;"
try 7 "a=4+3; return a;"

try 6 "foo=1;bar=2+3;return foo + bar;"

try 2 "if(1<10)2;"
try 10 "var=1;if(1<10)var=10;var;"
try 1 "var=1;if(1>10)var=10;var;"

try 2 "if(1<10) 2; else 3;"
try 3 "if(11<10) 2; else 3;"

try 10 "i=0;while(i<10)i=i+1; return i;"
try 0 "i=0;while(i<10)if(i==0) return i; return 11;"
try 5 "i=0;while(i<10)if(i<5) i=i+1; else return i; return 11;"

try 20 "a=10;for(i=0;i<10;i=i+1)a=a+1; return a;"
try 11 "i=0;for(;i<=10;)i=i+1;return i;"
try 5 "i=0;for(;i<=10;)if(i<5) i=i+1; else return i; return 11;"

echo OK
