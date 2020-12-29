#!/bin/bash

for filename in ~/Documents/os/os_project/test_bin/*; do
    isELF=$(file "$filename" | grep ELF | wc -l)
    
    if [[ $isELF > 0 ]]
    then
    	../elf-detector/cmake-build-debug/elf_detector "$filename" 
    fi
done
