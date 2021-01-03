//
// Created by heeve on 03.01.21.
//

#ifndef ELF_INJECTOR_PARSER_H
#define ELF_INJECTOR_PARSER_H

#include "utils.h"

Elf64_Addr getImageBase(int fd, const Elf64_Ehdr &ehdr);

int findSectionNum(int fd, int ccStart, int ccEnd);

void getSectionInfo(int fd, std::vector<std::pair<int, int>> &sectionMap);

code_cave_t findCodeCave(int fd, std::vector<std::pair<int, int>> &sectionMap, int needSize);

code_cave_t findCodeCaveSeg(int fd, int needSize);

#endif //ELF_INJECTOR_PARSER_H
