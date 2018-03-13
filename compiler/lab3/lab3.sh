#!/bin/bash
yacc -d lab3.y
gcc -o lab3 y.tab.c y.tab.h -lm
./lab3