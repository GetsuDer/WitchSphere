#include <cstring>
#include <iostream>

void 
render(int size) {
    if (size < 0) {
        std::cerr << "error: negative picture size" << std::endl;
    }
    return;
}

int
main(int argc, char **argv) {
    int i = 1;
    int size = 512;
    while (i < argc) {
        if (!strncmp(argv[i], "-w", 2)) {
            size = strtol(argv[i + 1], NULL, 10);
            std::cout << size << std::endl;
        }
    }
    
    render(size);
    return 0;
}
