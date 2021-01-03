cmake_minimum_required(VERSION 3.14)
project(elf_injector)

set(CMAKE_CXX_STANDARD 14)

add_executable(elf_injector main.cpp utils/utils.cpp utils/ldisasm.cpp utils/parser.cpp utils/parser.h utils/injector.cpp utils/injector.h)