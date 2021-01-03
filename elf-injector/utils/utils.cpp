//
// Created by heeve on 01.11.20.
//

#include "utils.h"

std::vector<char> hexToBytes(std::string hex) {
    std::vector<char> bytes;
    hex.insert(0, 8 - hex.size(), '0');
    for (unsigned int i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        char byte = (char) strtol(byteString.c_str(), NULL, 16);
        bytes.push_back(byte);
    }
    return bytes;
}

size_t lenDisasm(int fd, size_t reqLength, int entryPoint) {
    unsigned char buf[16];
    lseek(fd, entryPoint, SEEK_SET);
    read(fd, buf, sizeof(buf));
    size_t curLength = 0;
    while (curLength < reqLength) {
        size_t ln = ldisasm(buf + curLength, true);
        curLength += ln;
    }
    return curLength;
}

bool verifyFile(int fd) {
    char elf[4] = {0x7f, 'E', 'L', 'F'};
    char isELF[4];
    read(fd, isELF, 4);
    for (auto i = 0; i < sizeof(elf); ++i)
        if (elf[i] != isELF[i])
            return false;

    Elf64_Ehdr ehdr;
    lseek(fd, 0, SEEK_SET);
    read(fd, &ehdr, sizeof(ehdr));

    if (ehdr.e_machine != 0x3e)
        return false;

    lseek(fd, 0, SEEK_SET);
    return true;
}
