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
