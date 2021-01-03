//
// Created by heeve on 27.12.20.
//

#include "utils.h"


Elf64_Addr getImageBase(int fd, const Elf64_Ehdr &ehdr) {
    Elf64_Phdr phdr;

    lseek(fd, ehdr.e_phoff, SEEK_SET);

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

bool checkSegmentsSize(int fd) {
    Elf64_Ehdr ehdr;
    Elf64_Phdr phdr_1, phdr_2;
    char buf[128];
    int bytesRead;
    bool hasVirus = false;
    lseek(fd, 0, SEEK_SET);
    read(fd, &ehdr, sizeof(ehdr));
    lseek(fd, ehdr.e_phoff, SEEK_SET);

    for (int i = 0; i < ehdr.e_phnum - 1 && !hasVirus; ++i) {
        read(fd, &phdr_1, sizeof(phdr_1));
        if (!(phdr_1.p_flags & 1))
            continue;
        read(fd, &phdr_2, sizeof(phdr_2));

        int toRead = phdr_2.p_offset - phdr_1.p_offset - phdr_1.p_filesz;
        lseek(fd, phdr_1.p_offset + phdr_1.p_filesz, SEEK_SET);

        while (toRead > 0 && !hasVirus) {
            bytesRead = read(fd, buf, sizeof(buf));
            for (int j = 0; j < (bytesRead > toRead ? toRead : bytesRead); ++j)
                if (buf[j] != 0)
                    hasVirus = true;
            toRead -= bytesRead;
        }
        lseek(fd, ehdr.e_phoff + (i + 1) * sizeof(phdr_2), SEEK_SET);

    }
    return hasVirus;
}

bool checkJumps(int fd, const Elf64_Ehdr &ehdr) {
    int imgBase = getImageBase(fd, ehdr);
    lseek(fd, ehdr.e_entry - imgBase, SEEK_SET);
    unsigned char entryOp;

    read(fd, &entryOp, 1);
    return entryOp == JMP;
}