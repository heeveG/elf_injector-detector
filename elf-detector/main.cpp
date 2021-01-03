#include "utils.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Error. Usage: ./elf-detector <filepath>" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << argv[1] << " : injection detected - ";
    int fd = open(argv[1], O_RDWR);

    Elf64_Ehdr ehdr;
    read(fd, &ehdr, sizeof(ehdr));

    bool susJmp = checkJumps(fd, ehdr);
    bool susSize = checkSegmentsSize(fd);

    std::cout << (susJmp || susSize ? "true" : "false") << std::endl;
    return 0;
}
