#!/usr/bin/env bash

rm victim
rm payload
rm payload.o

as payload.s -o payload.o
objcopy -O binary ./payload.o payload
gcc ./test.c -o victim
