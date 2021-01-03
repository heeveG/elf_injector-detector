//
// Created by heeve on 03.01.21.
//

#include "parser.h"

Elf64_Addr getImageBase(int fd, const Elf64_Ehdr &ehdr) {
    Elf64_Phdr phdr;

    lseek(fd, ehdr.e_phoff, SEEK_SET);  // go to program header offset

    // read all program headers to find the LOAD type header
    for (int i = 0; i < ehdr.e_phnum; ++i) {
        if (read(fd, &phdr, sizeof(phdr)) == -1) {
            std::cerr << "Error reading program header" << std::endl;
            exit(EXIT_FAILURE);
        }
        if (phdr.p_type == 1) {
            return phdr.p_vaddr;
        }
    }

    std::cerr << "Could not find image base" << std::endl;
    exit(EXIT_FAILURE);
}

void getSectionInfo(int fd, std::vector<std::pair<int, int>> &sectionMap) {
    Elf64_Ehdr ehdr;
    Elf64_Shdr shdr;

    lseek(fd, 0, SEEK_SET);
    read(fd, &ehdr, sizeof(ehdr));
    lseek(fd, ehdr.e_shoff, SEEK_SET);
    for (int i = 0; i < ehdr.e_shnum; ++i) {
        read(fd, &shdr, sizeof(shdr));

        sectionMap.emplace_back(std::make_pair(shdr.sh_offset, shdr.sh_flags));
    }
    lseek(fd, 0, SEEK_SET);
}

int findSectionNum(int fd, int ccStart, int ccEnd) {
    Elf64_Ehdr ehdr;
    Elf64_Shdr shdr;

    lseek(fd, 0, SEEK_SET);
    read(fd, &ehdr, sizeof(ehdr));

    for (int i = 0; i < ehdr.e_shnum; ++i) {
        lseek(fd, ehdr.e_shoff + (ehdr.e_shentsize * i), SEEK_SET);
        read(fd, &shdr, sizeof(shdr));
        if (ccStart < shdr.sh_offset) {
            return i - 1;
        }
    }
    std::cerr << "Could not find suitable section" << std::endl;
    exit(EXIT_FAILURE);
}


// find code cave by iterating binary and searching for needed amount of zeros
code_cave_t findCodeCave(int fd, std::vector<std::pair<int, int>> &sectionMap, int needSize) {
    char buf[128];
    code_cave_t codeCave{};
    int bytesRead, currentCave = 0;
    int bytesProcessed = 0;
    bool start = true;
    int prevOffset = 0;
    int addrStart = 0, addrEnd = 0;

    codeCave.size = 0;
    lseek(fd, 0, SEEK_SET);

    bytesRead = read(fd, buf, sizeof(buf));

    while (bytesRead > 0) {
        for (int i = 0; i < bytesRead; i++) {
            bytesProcessed++;
            if (buf[i] == 0) {
                if (start) {
                    addrStart = prevOffset + bytesProcessed;
                    start = false;
                }
                currentCave++;
            } else {
                bool isExec = false;
                for (int mapIndx = 0; mapIndx < sectionMap.size(); ++mapIndx) {
                    if (addrStart < sectionMap[mapIndx].first) {
                        isExec = sectionMap[mapIndx - 1].second & 0x4;
                        break;
                    }
                }
                if (currentCave > codeCave.size && isExec) {
                    codeCave.start = addrStart;
                    codeCave.size = currentCave;
                    codeCave.end = prevOffset + bytesProcessed;
                }
                if (codeCave.size > needSize)
                    return codeCave;
                currentCave = 0;
                start = true;
            }
        }
        prevOffset += bytesProcessed;
        bytesProcessed = 0;
        bytesRead = read(fd, buf, sizeof(buf));
    }
    return codeCave;
}

// find code cave by iterating ELF binary segments and finding needed amount of zeros in segment's padding
code_cave_t findCodeCaveSeg(int fd, int needSize) {
    Elf64_Ehdr ehdr;
    Elf64_Phdr phdr_1, phdr_2;
    char buf[128];
    int bytesRead;
    bool isEmpty;
    lseek(fd, 0, SEEK_SET);
    read(fd, &ehdr, sizeof(ehdr));
    lseek(fd, ehdr.e_phoff, SEEK_SET);
    code_cave_t codeCave{};

    for (int i = 0; i < ehdr.e_phnum - 1; ++i) {
        isEmpty = true;
        read(fd, &phdr_1, sizeof(phdr_1));
        read(fd, &phdr_2, sizeof(phdr_2));

        int toRead = phdr_2.p_offset - phdr_1.p_offset - phdr_1.p_filesz;
        if (toRead < needSize || toRead < codeCave.size)
            isEmpty = false;

        lseek(fd, phdr_1.p_offset + phdr_1.p_filesz, SEEK_SET);

        while (toRead > 0 && isEmpty) {
            bytesRead = read(fd, buf, sizeof(buf));
            for (int j = 0; j < (bytesRead > toRead ? toRead : bytesRead) && isEmpty; ++j)
                if (buf[j] != 0)
                    isEmpty = false;
            toRead -= bytesRead;
        }
        if (isEmpty) {
            codeCave.start = phdr_1.p_offset + phdr_1.p_filesz;
            codeCave.end = phdr_2.p_offset;
            codeCave.size = codeCave.end - codeCave.start;
            codeCave.segment = i;
            codeCave.payloadSize = needSize;

        }
        lseek(fd, ehdr.e_phoff + (i + 1) * sizeof(phdr_2), SEEK_SET);
    }
    return codeCave;
}