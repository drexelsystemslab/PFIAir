#!/bin/bash

for file in *.off
do
    base=$(basename $file .off)
    meshlabserver -i $file -o $base.obj
done
