#include <cstring>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

#include "geom.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

Collision
find_hit(std::vector<Object *> *scene, Ray ray) {
    Collision hit, tmp;
    size_t len = scene->size();
    for (size_t i = 0; i < len; i++) {
        tmp = (*scene)[i]->intersect(ray);
        if (tmp.hit && (!hit.hit || hit.dist > tmp.dist)) {
            hit = tmp;
        }
    }
    return hit;
}

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
    std::vector<Light> lights = std::vector<Light>();
    lights.push_back(Light(Vec(0, 0, -size * 5), 0.8));
    //lights.push_back(Light(Vec(0, 0, 0), 1)); 
    std::vector<Object*> scene = std::vector<Object*>();

    float me_dist = size * 2;
    Vec me(0, 0, -me_dist);
    float side = size_f / 5;
    float small_side = side / 3;
    float big_side = size_f / 2;

    Vec shift(size / 5, size / 5, 0);
    
    Rectangle big(Vec(0, 0, size) + shift, Vec(0, big_side, size) + shift, Vec(big_side, big_side, size) + shift, Vec(big_side, 0, size) + shift);
    Rectangle base(Vec(0, 0, 0) + shift, Vec(0, side, 0) + shift, Vec(side, side, 0) + shift, Vec(side, 0, 0) + shift);
    Rectangle small(Vec(0, 0, -size) + shift, Vec(0, small_side, -size) + shift, Vec(small_side, small_side, -size) + shift, Vec(small_side, 0, -size) + shift);
    
    Cube cube(base);
    Cube small_cube(small);
    Cube big_cube(big); 
    
    scene.push_back(&cube);
    scene.push_back(&small_cube);
    scene.push_back(&big_cube);

    float base_light = 0.1;
    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            Vec dir = (Vec(x, y, size) - me).normalize();
            Ray ray = Ray(me, dir);
            Collision hit = find_hit(&scene, ray);
            
            if (hit.hit) {
                buffer[x + y * size] = Color(1, 1, 0, 1) * base_light;
                Vec intersect = me + (dir * hit.dist);
                for (size_t i = 0; i < lights.size(); i++) {
                    Vec light_dir = lights[i].pos - intersect;
                    Ray to_light(intersect, light_dir.normalize());
                    Collision intersected = find_hit(&scene, to_light);
                    if (!intersected.hit) {
                        float angle = abs(dot(light_dir.normalize(), hit.normal.normalize()));
                        float distance = light_dir.len();
                        Color add(1, 1, 0);
                        add = add * angle;
                        add = add * lights[i].intensity;
                        buffer[x + y * size] = buffer[x + y * size] + add;
                    } else {
                        //std::cout << "no light " << intersected.dist << '\n';
                    }
                    
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
