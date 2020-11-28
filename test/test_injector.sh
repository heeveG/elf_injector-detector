#!/usr/bin/env bash

rm -rf ../elf-injector/build
mkdir ../elf-injector/build
cmake -S ../elf-injector -B ../elf-injector/build
make -C ../elf-injector/build

../elf-injector/build/elf_injector victim payload