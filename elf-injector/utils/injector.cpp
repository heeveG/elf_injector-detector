//
// Created by heeve on 03.01.21.
//

#include "injector.h"


void injectCode(int fd, const code_cave_t &codeCave, int entryPoint, const Elf64_Ehdr &ehdr, int imgBase,
                const std::vector<unsigned char> &payload) {
    size_t reqLength = JMP_SIZE;
    size_t lenStolen = lenDisasm(fd, reqLength, entryPoint - imgBase);
    unsigned char stolen[lenStolen];

    lseek(fd, entryPoint - imgBase, SEEK_SET);
    read(fd, &stolen, lenStolen);

    unsigned char jmp = JMP, nop = NOP, call = CALL;

    const int ccOffset = (codeCave.start + payload.size()) - (entryPoint + JMP_SIZE);
    const int origOffset =
            (entryPoint + JMP_SIZE) -
            (codeCave.start + lenStolen + payload.size() + JMP_SIZE + CALL_SIZE);
    const int payloadOffset = codeCave.start - (codeCave.start + payload.size() + CALL_SIZE);

    std::stringstream sStreamCC, sStreamOrig, sStreamPayload;
    sStreamCC << std::hex << htonl(ccOffset);
    sStreamOrig << std::hex << htonl(origOffset);
    sStreamPayload << std::hex << htonl(payloadOffset);

    std::vector<char> jumpToCCBytes = hexToBytes(sStreamCC.str());
    std::vector<char> jumpToOrigBytes = hexToBytes(sStreamOrig.str());
    std::vector<char> jumpToPayloadBytes = hexToBytes(sStreamPayload.str());

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

void prepareSegment(int fd, const code_cave_t &cc, const Elf64_Ehdr &ehdr) {
    Elf64_Phdr phdr;

    lseek(fd, ehdr.e_phoff + cc.segment * sizeof(phdr), SEEK_SET);
    read(fd, &phdr, sizeof(phdr));
    phdr.p_filesz += cc.payloadSize;
    phdr.p_memsz += cc.payloadSize;
    phdr.p_flags |= 1;
    lseek(fd, ehdr.e_phoff + cc.segment * sizeof(phdr), SEEK_SET);
    write(fd, &phdr, sizeof(phdr));
}

void setSectionExecFlag(int fd, int secNum, const Elf64_Ehdr &ehdr) {
    Elf64_Shdr shdr;

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

