# ELF file injector  + injection detector

## Authors

 - [Nazar Pasternak](https://github.com/heeveG)
 - [Dan Klimenko]
## Idea
Create software, that will inject binary module into given ELF binary file, such that the injected code executes first and the original code after that. 

Also develop a detector for such injection.

## Prerequisites

 - **C++ compiler** - needs to support **C++17** standard
 - **CMake** 3.14+
 
## Installing

1. Clone the project.
    ```bash
    git clone https://github.com/heeveG/elf_injector-detector
    ```
