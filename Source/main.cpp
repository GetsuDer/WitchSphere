#include <cstring>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

#include "geom.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


void 
render(int size) {
    if (size < 0) {
        std::cerr << "error: negative picture size" << std::endl;
        return;
    }
    std::vector<Color> buffer(size * size);
    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            buffer[x + y * size] = Color(0, 0, 1, 1.f);
        }
    }
    float size_f = size;
    Vec me(size_f / 2, size_f / 2, size_f * 2);
    Triangle tr(Vec(0, 0, 0), Vec(0, size_f / 2, size_f / 3), Vec(size_f / 2, 0, 0));
    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            Ray ray = Ray(me, (Vec(x, y, size) - me).normalize());
            if (intersect(tr, ray) > 0) {
                buffer[x + y * size] = Color(1, 1, 0, 1);
            }
        }
    }

    unsigned char *data = new unsigned char[size * size * 3];
    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            data[0 + x * 3 + y * size * 3] = (unsigned char)(255 * std::max(0.f, std::min(1.f, buffer[x + y * size].r)));
            data[1 + x * 3 + y * size * 3] = (unsigned char)(255 * std::max(0.f, std::min(1.f, buffer[x + y * size].g)));
            data[2 + x * 3 + y * size * 3] = (unsigned char)(255 * std::max(0.f, std::min(1.f, buffer[x + y * size].b)));
        }
    }

    stbi_write_png("328_derevyanko_v4v5.png", size, size, 3, data, size * 3);
    delete[] data;
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
