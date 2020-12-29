#!/bin/bash

for filename in /bin*; do
    if [[ -x "$filename" ]]
    then
    	cp $filename  ~/Documents/os/os_project/test_bin/${filename##*/}
fi
done
