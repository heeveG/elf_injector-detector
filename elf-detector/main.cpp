#include "utils.h"


int main(int argc, char **argv) {
    if (argc != 2)
    {
        std::cerr << "Error. Usage: ./elf-detector <filepath>" << std::endl;
        exit(EXIT_FAILURE);
    }

    int fd = open(argv[1], O_RDWR);

    Elf64_Ehdr ehdr;
    read(fd, &ehdr, sizeof(ehdr));
    lseek(fd, ehdr.e_entry, SEEK_SET);
    unsigned char entryOp = 0xe9;
    std::cout << "Entry point OP: " << std::hex << entryOp << std::endl;

    read(fd, &entryOp, 1);

    std::cout << "Entry point: " << ehdr.e_entry << std::endl;
    std::cout << "Entry point OP: " << std::hex << (int)entryOp << std::endl;

    std::cout << "Segments' sizes are changed: " << (checkSegmentsSize(fd) ? "true" : "false") << std::endl;

    return 0;
}
