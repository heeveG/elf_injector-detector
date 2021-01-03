//
// Created by heeve on 03.01.21.
//

#ifndef ELF_INJECTOR_INJECTOR_H
#define ELF_INJECTOR_INJECTOR_H

#include "utils.h"

void injectCode(int fd, const code_cave_t &codeCave, int entryPoint, const Elf64_Ehdr &ehdr, int imgBase,
                const std::vector<unsigned char> &);

void prepareSegment(int fd, const code_cave_t &cc, const Elf64_Ehdr &ehdr);

void setSectionExecFlag(int fd, int secNum, const Elf64_Ehdr &ehdr);

void setEntryPoint(int fd, long address);

#endif //ELF_INJECTOR_INJECTOR_H
