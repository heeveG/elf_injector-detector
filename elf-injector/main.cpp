#include <iostream>
#include "utils/utils.h"

int main(int argc, char **argv) {



    int fd = open(argv[1], O_RDWR);

    Elf64_Ehdr ehdr;
    read(fd, &ehdr, sizeof(ehdr));
    std::cout << "Old entry point: " << ehdr.e_entry << std::endl;

    // get code cave
    code_cave_t cc = findCodeCave(fd);
    std::cout << "Code cave start: " << cc.start << " Code cave end: " << cc.end << std::endl;
    // get image base
    int imgBase = getImageBase(fd);
    std::cout << "Image base: " << imgBase << std::endl;
    // find section of the code cave
    int secNum = findSectionNum(fd, cc.start, cc.end);
    std::cout << "Section number of the Code cave: " << secNum << std::endl;
    // set exec flag for the section
    setExecFlag(fd, secNum);

    // set entry point
    setEntryPoint(fd, imgBase + cc.start);

    lseek(fd, 0, SEEK_SET);
    read(fd, &ehdr, sizeof(ehdr));
    std::cout << "New entry point: " << ehdr.e_entry << std::endl;

    return 0;
}
