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
#include <vector>
#include <arpa/inet.h>
#include <sstream>

struct code_cave_t {
    size_t size;
    int start;
    int end;
};

Elf64_Addr getImageBase(int fd, const Elf64_Ehdr& ehdr);
int findSectionNum(int fd, int ccStart, int ccEnd);
void setExecFlag(int fd, int secNum);
void setEntryPoint(int fd, long address);
code_cave_t findCodeCave(int fd);
std::vector<char> hexToBytes(std::string hex);
void makeAllExec(int fd);
void injectCode(int fd, const code_cave_t &codeCave, int entryPoint, const Elf64_Ehdr &ehdr, int imgBase);

#endif //ELF_INJECTOR_UTILS_H
