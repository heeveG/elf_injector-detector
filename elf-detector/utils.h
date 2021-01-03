//
// Created by heeve on 27.12.20.
//

#ifndef ELF_DETECTOR_UTILS_H
#define ELF_DETECTOR_UTILS_H

#include <iostream>
#include "unistd.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include <arpa/inet.h>
#include <sstream>
#include <elf.h>

#define JMP 0xe9

bool checkSegmentsSize(int fd);

Elf64_Addr getImageBase(int fd, const Elf64_Ehdr &ehdr);

bool checkJumps(int fd, const Elf64_Ehdr &ehdr);

#endif //ELF_DETECTOR_UTILS_H
