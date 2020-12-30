#include "utils.h"
#include "string.h"

int main(int argc, char **argv) {
    if (argc != 2)
    {
        std::cerr << "Error. Usage: ./elf-detector <filepath>" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << argv[1] << " contains virus: ";
    int fd = open(argv[1], O_RDWR);

    Elf64_Ehdr ehdr;
    read(fd, &ehdr, sizeof(ehdr));
    int imgBase = getImageBase(fd, ehdr);


    lseek(fd, ehdr.e_entry - imgBase, SEEK_SET);
    unsigned char entryOp;

    read(fd, &entryOp, 1);
    bool jmpEP = entryOp == 0xe9;
//    std::cout << "Entry point: " << ehdr.e_entry << std::endl;
//    std::cout << "Entry point OP: " << std::hex << (int)entryOp << std::endl;
//    if ((checkSegmentsSize(fd)))
    std::cout << (checkSegmentsSize(fd) || jmpEP ? "true" : "false") << std::endl;

    return 0;
}
