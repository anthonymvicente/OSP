#!/bin/bash

for ((i=1; i<=100; i++))
do
    echo "TEST $i"
    ./scheduler -test -f0 rr

done
