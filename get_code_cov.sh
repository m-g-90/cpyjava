#!/bin/bash
for filename in `find . | egrep '\.c'`; 
do 
  gcov-5 -n -o .obj $filename > /dev/null; 
done
