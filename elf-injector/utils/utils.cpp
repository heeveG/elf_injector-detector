//
// Created by heeve on 01.11.20.
//

#include <iostream>
#include "utils.h"

// TODO: check if ImageBase is p_type 1
Elf64_Addr getImageBase(int fd) {
    Elf64_Ehdr ehdr;
    Elf64_Phdr phdr;

    lseek(fd, 0, SEEK_SET);
    if (read(fd, &ehdr, sizeof(ehdr)) == -1) {
        std::cerr << "Error reading elf header" << std::endl;
        exit(EXIT_FAILURE);
    }  // read elf header

    lseek(fd, ehdr.e_phoff, SEEK_SET);  // go to program header offset

    // read all program headers to find the LOAD type header
    for (int i = 0; i < ehdr.e_phnum; ++i) {
        if (read(fd, &phdr, sizeof(phdr)) == -1) {
            std::cerr << "Error reading program header" << std::endl;
            exit(EXIT_FAILURE);
        }
        if (phdr.p_type == 1)
            return phdr.p_vaddr;
    }

    std::cerr << "Could not find image base" << std::endl;
    exit(EXIT_FAILURE);
}

int findSectionNum(int fd, int ccStart, int ccEnd) {
    Elf64_Ehdr ehdr;
    Elf64_Shdr shdr;

    lseek(fd, 0, SEEK_SET);
    read(fd, &ehdr, sizeof(ehdr));

    for (int i = 0; i < ehdr.e_shnum; ++i) {
        lseek(fd, ehdr.e_shoff + (ehdr.e_shentsize * i), SEEK_SET);
        read(fd, &shdr, sizeof(shdr));

        if (ccStart >= shdr.sh_offset && ccEnd <= (shdr.sh_offset + shdr.sh_entsize)) {
            return i;
        }
    }
    std::cerr << "Could not find suitable section" << std::endl;
    exit(EXIT_FAILURE);
}

void setExecFlag(int fd, int secNum) {
    Elf64_Ehdr ehdr;
    Elf64_Shdr shdr;

    // TODO: add read and write error checks
    lseek(fd, 0, SEEK_SET);
    read(fd, &ehdr, sizeof(ehdr));

    // find offset of the given section and read it
    int sectionOffset = lseek(fd, ehdr.e_shoff + (ehdr.e_shentsize * secNum), SEEK_SET);
    read(fd, &shdr, sizeof(shdr));

    shdr.sh_flags |= 0x4;  // set the executable flag

    // write back the modified section
    lseek(fd, sectionOffset, SEEK_SET);
    write(fd, &shdr, sizeof(shdr));
}

void setEntryPoint(int fd, int address) {
    Elf64_Ehdr ehdr;

    lseek(fd, 0, SEEK_SET);
    if (read(fd, &ehdr, sizeof(ehdr)) == -1) {
        std::cerr << "Error reading elf header" << std::endl;
        exit(EXIT_FAILURE);
    }  // read elf header

    ehdr.e_entry = address;

    lseek(fd, 0, SEEK_SET);
    if (write(fd, &ehdr, sizeof(ehdr)) == -1) {
        std::cerr << "Error writing elf header" << std::endl;
        exit(EXIT_FAILURE);
    }  // write updated elf header
}


code_cave_t findCodeCave(int fd) {
    char buf[128];
    code_cave_t codeCave{};
    struct stat file_info{};
    int bytesRead, currentCave = 0;
    int bytesProcessed = 0;
    int start = 1;
    int prevOffset = 0;
    int addrStart = 0;

    codeCave.size = 0;
    lseek(fd, 0, SEEK_SET);
    if (fstat(fd, &file_info)) {
        std::cout << "Error while executing fstat()" << std::endl;
        exit(EXIT_FAILURE);
    }
    if ((bytesRead = read(fd, buf, sizeof(buf)) == -1)) {
        std::cout << "Error while reading the file" << std::endl;
        exit(EXIT_FAILURE);
    }

    while (bytesRead > 0) {
        for (int i = 0; i < bytesRead; i++) {
            bytesProcessed++;
            if (buf[i] == 0) {
                if (start) {
                    addrStart = prevOffset + bytesProcessed;
                    start--;
                }
                currentCave++;
            } else {
                if (currentCave > codeCave.size) {
                    codeCave.start = addrStart;
                    codeCave.size = currentCave;
                    codeCave.end = prevOffset + bytesProcessed;
                }
                currentCave = 0;
                start = 1;
            }
        }
        prevOffset += bytesProcessed;
        bytesProcessed = 0;
        if ((bytesRead = read(fd, buf, sizeof(buf)) == -1)) {
            std::cout << "Error, while reading file" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    return codeCave;
}