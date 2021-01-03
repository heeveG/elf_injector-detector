#!/bin/bash

rm ~/Documents/os/os_project/test_bin/*

for filename in /bin/*; do
    isELF=$(file "$filename" | grep ELF | wc -l)
    
    if [[ ! -L "$filename" && -x "$filename" && isELF > 0 ]]
    then
    	cp -r $filename  ~/Documents/os/os_project/test_bin/${filename##*/}
fi
done


