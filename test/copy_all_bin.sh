#!/bin/bash

for filename in /bin/*; do
    if [[ ! -L "$filename" && -x "$filename" ]]
    then
    	cp -r $filename  ~/Documents/os/os_project/test_bin/${filename##*/}
fi
done

