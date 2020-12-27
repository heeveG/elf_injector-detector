cmake_minimum_required(VERSION 3.17)
project(elf_detector)

set(CMAKE_CXX_STANDARD 14)

add_executable(elf_detector main.cpp utils.cpp utils.h)