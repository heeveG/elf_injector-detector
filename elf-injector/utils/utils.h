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
#include <fstream>
#include <iterator>
#include <unordered_map>
#include "ldisasm.h"
#include <iostream>

#define JMP 0xe9
#define NOP 0x90
#define CALL 0xe8
#define JMP_SIZE 0x5
#define CALL_SIZE 0x5

struct code_cave_t {
    size_t size;
    int start;
    int end;
    int segment;
    int payloadSize;
};

std::vector<char> hexToBytes(std::string hex);

size_t lenDisasm(int fd, size_t reqLength, int entryPoint);

bool verifyFile(int fd);

#endif //ELF_INJECTOR_UTILS_H
