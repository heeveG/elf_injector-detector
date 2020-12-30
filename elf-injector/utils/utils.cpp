//
// Created by heeve on 01.11.20.
//

#include <iostream>
#include "utils.h"

// TODO: check if ImageBase is p_type 1
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

void makeSegExec(int fd, code_cave_t cc) {
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
        if (phdr.p_offset > cc.start) {
            lseek(fd, -2 * sizeof(phdr), SEEK_CUR);
            read(fd, &phdr, sizeof(phdr));
            phdr.p_flags |= 1;
            lseek(fd, -1 * sizeof(phdr), SEEK_CUR);
            write(fd, &phdr, sizeof(phdr));
        }
    }
}

void fillSectionMap(int fd, std::vector<std::pair<int, int>> &sectionMap) {
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
    shdr.sh_size += 0x14;
    // write back the modified section
    lseek(fd, sectionOffset, SEEK_SET);
    write(fd, &shdr, sizeof(shdr));
}

void setEntryPoint(int fd, long address) {
    Elf64_Ehdr ehdr;

    lseek(fd, 0, SEEK_SET);
    if (read(fd, &ehdr, sizeof(ehdr)) == -1) {
        std::cerr << "Error reading elf header" << std::endl;
        exit(EXIT_FAILURE);
    }  // read elf header

    lseek(fd, 0, SEEK_SET);
    ehdr.e_entry = address;

    if (write(fd, &ehdr, sizeof(ehdr)) == -1) {
        std::cerr << "Error writing elf header" << std::endl;
        exit(EXIT_FAILURE);
    }  // write updated elf header
}


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

//                        addrEnd = sectionMap[mapIndx].first;
                        break;
                    }
                }
                if (currentCave > codeCave.size && isExec) {
                    codeCave.start = addrStart;
                    codeCave.size = currentCave;
//                    codeCave.end = addrEnd;
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

size_t lenDisasm(int fd, size_t reqLength, int entryPoint)
{
    unsigned char buf[16];
    lseek(fd, entryPoint, SEEK_SET);
    read(fd, buf, sizeof(buf));
    size_t curLength = 0;
    while (curLength < reqLength){
        size_t ln = ldisasm(buf + curLength, true);
        curLength += ln;
    }
    return curLength;
}
void injectCode(int fd, const code_cave_t &codeCave, int entryPoint, const Elf64_Ehdr &ehdr, int imgBase,
                const std::vector<unsigned char> &payload) {
    size_t reqLength = 5; // size of rel jump
    size_t lenStolen = lenDisasm(fd, reqLength, entryPoint - imgBase);
    unsigned char stolen[lenStolen];

    lseek(fd, entryPoint - imgBase, SEEK_SET);
    read(fd, &stolen, lenStolen);

    unsigned char jmp = 0xe9, nop = 0x90, call = 0xe8;
    const int ccOffset = (codeCave.start + payload.size()) - (entryPoint + 0x5); // 0x5 - size of relative jump
    const int origOffset =
            (entryPoint + 0x5) -
            (codeCave.start + lenStolen + payload.size() + 0x5 + 0x5); // 0x5's for call and jmp
    const int payloadOffset = codeCave.start - (codeCave.start + payload.size() + 0x5);

    std::stringstream sStreamCC, sStreamOrig, sStreamPayload;
    sStreamCC << std::hex << htonl(ccOffset);
    sStreamOrig << std::hex << htonl(origOffset);
    sStreamPayload << std::hex << htonl(payloadOffset);

    std::vector<char> jumpToCCBytes = hexToBytes(sStreamCC.str());
    std::vector<char> jumpToOrigBytes = hexToBytes(sStreamOrig.str());
    std::vector<char> jumpToPayloadBytes = hexToBytes(sStreamPayload.str());

    std::cout << sStreamCC.str() << " " << sStreamOrig.str() << " " << sStreamPayload.str() << std::endl;

    lseek(fd, 0, SEEK_SET);

    // write jump to code cave
    lseek(fd, entryPoint - imgBase, SEEK_SET);
    write(fd, &jmp, sizeof(jmp));

    for (auto &j: jumpToCCBytes) {
        write(fd, &j, sizeof(j));
    }
    if (lenStolen > reqLength)
        for (int st = 0; st < lenStolen - reqLength; ++st)
            write(fd, &nop, sizeof(nop));

    // write stolen and jump to original code

    lseek(fd, codeCave.start - imgBase, SEEK_SET);
    write(fd, payload.data(), payload.size());

    write(fd, &call, sizeof(call));
    for (auto &j: jumpToPayloadBytes) {
        write(fd, &j, sizeof(j));
    }

    write(fd, &stolen, lenStolen);

    write(fd, &jmp, sizeof(jmp));
    for (auto &j: jumpToOrigBytes) {
        write(fd, &j, sizeof(j));
    }
}

code_cave_t findCodeCave2(int fd, int needSize) {
    Elf64_Ehdr ehdr;
    Elf64_Phdr phdr_1, phdr_2;
    char buf[128];
    int bytesRead;
    bool isEmpty;
    lseek(fd, 0, SEEK_SET);
    read(fd, &ehdr, sizeof(ehdr));
    lseek(fd, ehdr.e_phoff, SEEK_SET);
    code_cave_t codeCave{};
    int biggestCC;

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
            biggestCC = i;

        }
        lseek(fd, ehdr.e_phoff + (i + 1) * sizeof(phdr_2), SEEK_SET);
    }
    if (codeCave.size) {
        lseek(fd, ehdr.e_phoff + (biggestCC) * sizeof(phdr_1), SEEK_SET);
        read(fd, &phdr_1, sizeof(phdr_1));
        phdr_1.p_filesz += needSize;
        phdr_1.p_memsz += needSize;
        phdr_1.p_flags |= 1;
        lseek(fd, ehdr.e_phoff + (biggestCC) * sizeof(phdr_1), SEEK_SET);
        write(fd, &phdr_1, sizeof(phdr_1));
    }
    return codeCave;
}