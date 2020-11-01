//
// Created by heeve on 01.11.20.
//

#include "elf.h"
#include "unistd.h"
#include <iostream>

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

int setExecFlag(int fd, int secNum) {
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

    return 0;
}

int setEntryPoint(int fd, int address) {
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
    return 0;
}