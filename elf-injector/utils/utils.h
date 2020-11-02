//
// Created by heeve on 02.11.20.
//

#ifndef ELF_INJECTOR_UTILS_H
#define ELF_INJECTOR_UTILS_H

#include "elf.h"
#include "unistd.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

struct code_cave_t {
    size_t size;
    int start;
    int end;

};

Elf64_Addr getImageBase(int fd);
int findSectionNum(int fd, int ccStart, int ccEnd);
void setExecFlag(int fd, int secNum);
void setEntryPoint(int fd, int address);
code_cave_t findCodeCave(int fd);


#endif //ELF_INJECTOR_UTILS_H
