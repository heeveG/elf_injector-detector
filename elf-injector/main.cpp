#include <iostream>
#include "utils/utils.h"

int main(int argc, char **argv) {

    int fd = open(argv[1], O_RDWR);

    // get code cave
    code_cave_t cc = findCodeCave(fd);

    // get image base
    int imgBase = getImageBase(fd);

    // find section of the code cave
    int secNum = findSectionNum(fd, cc.start, cc.end);

    // set exec flag for the section
    setExecFlag(fd, secNum);

    // set entry point
    setEntryPoint(fd, imgBase + cc.start);


    return 0;
}
