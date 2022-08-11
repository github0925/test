#!/bin/sh

i=100

while [ $i -gt 0 ]
do
echo "test $i begin"
echo "case 1"
rgx_triangle_test -tpf 1000 -f 1000 -b
echo "case 2"
rgx_triangle_test -ser 1 -f 1
echo "case 3"
rgx_triangle_test -ser 1 -f 100
echo "case 4"
rgx_triangle_test -ser 1 -f 1000
echo "case 5"
rgx_triangle_test -ser 1 -f 10000
echo "case 6"
rgx_triangle_test -f 1
echo "case 7"
rgx_triangle_test -f 100
echo "case 8"
rgx_triangle_test -f 1000
echo "case 9"
rgx_triangle_test -f 10000
echo "case 10"
rgx_triangle_test -gs 4 4 -f 1
echo "case 11"
rgx_triangle_test -gs 4 4 -f 100
echo "case 12"
rgx_triangle_test -gs 4 4 -f 1000
echo "case 13"
rgx_triangle_test -gs 4 4 -f 10000
echo "case 14"
rgx_triangle_test -b -f 1
echo "case 15"
rgx_triangle_test -b -f 100
echo "case 16"
rgx_triangle_test -b -f 1000
echo "case 17"
rgx_triangle_test -b -f 10000
echo "case 18"
rgx_triangle_test -gs 4 4 -b -f 1
echo "case 19"
rgx_triangle_test -gs 4 4 -b -f 100
echo "case 20"
rgx_triangle_test -gs 4 4 -b -f 1000
echo "case 21"
rgx_triangle_test -gs 4 4 -b -f 10000
echo "case 22"
rgx_triangle_test -tpf 1 -f 1000
echo "case 23"
rgx_triangle_test -tpf 10 -f 1000
echo "case 24"
rgx_triangle_test -tpf 100 -f 1000
echo "case 25"
rgx_triangle_test -tpf 1000 -f 1000
echo "case 26"
rgx_triangle_test -tpf 10000 -f 1000
echo "case 27"
rgx_triangle_test -gs 4 4 -tpf 1 -f 1000
echo "case 28"
rgx_triangle_test -gs 4 4 -tpf 10 -f 1000
echo "case 29"
rgx_triangle_test -gs 4 4 -tpf 100 -f 1000
echo "case 30"
rgx_triangle_test -gs 4 4 -tpf 1000 -f 1000
echo "case 31"
rgx_triangle_test -gs 4 4 -tpf 10000 -f 1000
echo "case 32"
rgx_triangle_test -pb 8192 -pbmax 8192 -tpf 1 -f 1000
echo "case 33"
rgx_triangle_test -pb 8192 -pbmax 8192 -tpf 10 -f 1000
echo "case 34"
rgx_triangle_test -pb 8192 -pbmax 8192 -tpf 100 -f 1000
echo "case 35"
rgx_triangle_test -pb 8192 -pbmax 8192 -tpf 1000 -f 1000
echo "case 36"
rgx_triangle_test -pb 32 -pbmax 32 -tpf 1 -f 1000
echo "case 37"
rgx_triangle_test -pb 32 -pbmax 32 -tpf 10 -f 1000
echo "case 38"
rgx_triangle_test -pb 32 -pbmax 32 -tpf 100 -f 1000
echo "case 39"
rgx_triangle_test -pb 32 -pbmax 32 -tpf 1000 -f 1000
echo "case 40"
rgx_triangle_test -pb 32 -tpf 1 -tpf2 10000 -f 1000
echo "case 41"
rgx_triangle_test -tpf 1 -tpf2 10000 -f 1000
echo "case 42"
rgx_triangle_test -tpfr -tpf 10000 -ts 5 -seed 81576 -f 10000
echo "case 43"
rgx_triangle_test -tpfr -tpf 10000 -ts 5 -seed 81576 -f 10000 -gs 4 4

 i=`expr $i - 1`
done
