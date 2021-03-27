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
            buffer[x + y * size] = Color(0, 0, 0, 1.f);
        }
    }
    float size_f = size;
    Light lamp(Vec(0, 0, size), 1);
    
    Vec me(size_f / 2, size_f / 2, size_f * 2);
    Rectangle base(Vec(0, 0, 0), Vec(0, 0, size / 3), Vec(size / 3, 0, size / 3), Vec(size / 3, 0, 0));
    //Pentagon pent(Vec(size / 2, size / 2, 0), Vec(0, size / 2, 0), Vec(0, 0, size / 2));
    Cube cube(base, Vec(0, size / 3, 0));
    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            Vec dir = (Vec(x, y, size) - me).normalize();
            Ray ray = Ray(me, dir);
            float dist = cube.intersect(ray);
            if (dist > 0) {
                Vec intersect = me + (dir * dist);
                Vec light_dir = lamp.pos - intersect;
                Ray to_light(intersect, light_dir.normalize());
                if (cube.intersect(to_light) <= 0) {
                    std::cout << light_dir.len() << '\n';
                    buffer[x + y * size] = Color((light_dir.len() * light_dir.len()) / (size * size), 0, 0, 1);// / light_dir.len(), 1 / light_dir.len(), 0, 1);
                } else {
                    std::cout << cube.intersect(to_light) << '\n';
                   // buffer[x + y * size] = Color(0.5, 0.5, 0, 1);
                }
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
            i++;
            if (argc <= i) return 1;
            size = strtol(argv[i], NULL, 10);
            i++;
        } else {
            i++;
        }
    }
    
    render(size);
    return 0;
}
