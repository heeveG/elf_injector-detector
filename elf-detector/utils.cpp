//
// Created by heeve on 27.12.20.
//

#include "utils.h"

bool checkSegmentsSize(int fd)
{
    Elf64_Ehdr ehdr;
    Elf64_Phdr phdr_1, phdr_2;
    char buf[128];
    int bytesRead;
    bool hasVirus = false;
    lseek(fd, 0, SEEK_SET);
    read(fd, &ehdr, sizeof(ehdr));
    lseek(fd, ehdr.e_phoff, SEEK_SET);

    for (int i = 0; i < ehdr.e_phnum - 1 && !hasVirus; ++i)
    {
        read(fd, &phdr_1, sizeof(phdr_1));
        if (!(phdr_1.p_flags & 1))
            continue;
        read(fd, &phdr_2, sizeof(phdr_2));

        int toRead = phdr_2.p_offset - phdr_1.p_offset - phdr_1.p_filesz;
        lseek(fd, phdr_1.p_offset + phdr_1.p_filesz, SEEK_SET);

        while (toRead > 0 && !hasVirus)
        {
            bytesRead = read(fd, buf, sizeof(buf));
            for (int j = 0; j < (bytesRead > toRead ? toRead : bytesRead); ++j)
                if (buf[j] != 0)
                    hasVirus = true;
            toRead -= bytesRead;
        }
        lseek(fd, ehdr.e_phoff + (i + 1) * sizeof(phdr_2), SEEK_SET);

    }
    return hasVirus;
}