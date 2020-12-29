#!/bin/bash

for filename in ~/Documents/os/os_project/test_bin/*; do
    isELF=$(file "$filename" | grep ELF | wc -l)
    
    if [[ $isELF > 0 ]]
    then
    	../elf-injector/cmake-build-debug/elf_injector "$filename" ../../test/payload
    fi
done
