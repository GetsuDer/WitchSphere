#include <cstring>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

#include "geom.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
float SIZE = 1;

Vec reflect_vec(Vec, Vec, float);

Collision
trace_ray(std::vector<Object *> *scene, std::vector<Light> *lights, Ray ray, int deep) {
    Collision hit, tmp;
    if (deep <= 0) {
        return hit;
    }

    size_t len = scene->size();
    float EPS = 1e-3;
    for (size_t i = 0; i < len; i++) {
        tmp = (*scene)[i]->intersect(ray);
        if (tmp.hit && (!hit.hit || hit.dist - tmp.dist > EPS)) {
            hit = tmp;
        }
    }
    if (hit.hit) {
        Vec intersect(ray.pos + (ray.dir * hit.dist));
        // light for THIS point
        size_t lights_len = lights->size();
        float add = 0;
        for (size_t i = 0; i < lights_len; i++) {
            Vec light_dir = (*lights)[i].pos - intersect;
            // for each light do, while not wall
            Ray to_light(intersect, light_dir.normalize());
            float way_coef = 0;
            float distance = light_dir.len();
            
            bool result = true;
            float in_a = 1;
            while (true) {
                Collision intersected, tmp;
                for (size_t j = 0; j < len; j++) {
                    tmp = (*scene)[j]->intersect(to_light);
                    if (!intersected.hit || (intersected.dist - tmp.dist > EPS)) {
                        intersected = tmp;
                    }
                }
                if (!intersected.hit) {
                    way_coef += in_a * ((*lights)[i].pos - to_light.pos).len();
                    break;
                } else { // walked into something
                    if (intersected.color.a < 1) {
                        way_coef += in_a * intersected.dist;
                        // here all is bad, because i dont know, inside or outside of object we went
                        if (in_a < 1) {
                            in_a = 1; // kostyl`
                        } else {
                            in_a = intersected.color.a;
                        }
                    } else {
                        result = false;
                        break;
                    }
                }
            }
            if (result) {
                float angle = abs(dot(light_dir.normalize(), hit.normal.normalize()));
                float light = angle * ((*lights)[i].intensity / (way_coef * way_coef));
                add += light;
            }        
        }  
        hit.color = hit.color * add;
        float pogl = exp(-(hit.absorbtion * tmp.dist * 4 / SIZE));
    }
    
    return hit;
}

Vec
reflect_vec(Vec v, Vec normal, float reflect) {
    float alpha = acos(dot(v.normalize(), normal.normalize() * (-1)));
    float beta = asin(sin(alpha) / reflect);
    Vec axis = cross(v, normal);
    return rotateAroundAxis(v, axis, beta - alpha);
}

void 
render(int size) {
    if (size < 0) {
        std::cerr << "error: negative picture size" << std::endl;
        return;
    }
    SIZE = size;
    std::vector<Color> buffer(size * size);
    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            buffer[x + y * size] = Color(0, 0, 0, 1.f);
        }
    }
    std::vector<Light> lights = std::vector<Light>();
    lights.push_back(Light(Vec(0, 0, -size * 1.5), 1.5 * size * size));
    lights.push_back(Light(Vec(-size * 3, size, 0), 5 * size * size));
    
    std::vector<Object*> scene = std::vector<Object*>();
    std::vector<Object*> sphere = std::vector<Object*>();
    std::vector<Object*> empty = std::vector<Object*>();

    float me_dist = size * 2;
    Vec me(0, 0, -me_dist);
    
    Vec d_center(size / 3, size / 3, 0);
    Vec d_normal(0, 1, 0);
    float d_size = size / 4.5;
    Dodekaedr d(d_center, d_normal, d_size, Color(1, 0, 0, 0.5), 1.02, 0, 1.3, true);
   
    float a = d_size;
    float b = a / sqrt(2 - 2 * cos(2 * M_PI / 5));
    float r = a * (1 + sqrt(5)) * sqrt(3) / 4;
    float k = sqrt(r * r - b * b);

    float side = size / 5; 
    Vec shift(size / 4, size / 4, size / 3);
    Rectangle base(Vec(0, 0, 0) + shift, Vec(0, side, 0) + shift, Vec(side, side, 0) + shift, Vec(side, 0, 0) + shift);
    Cube cube(base, Color(0, 0, 1, 1), 1, 0, 0, false);

    scene.push_back(&cube);
    scene.push_back(&d);
    // podstavka

    float p_side = size / 4;
    Vec p_shift = d_center + (d_normal * k);
    Rectangle* p_base = new Rectangle(Vec(0, 0, 0) + p_shift, Vec(0, p_side, 0) + p_shift, Vec(p_side, p_side, 0) + p_shift, Vec(p_side, 0, 0) + p_shift);
    Cube* p_cube = new Cube(*p_base, Color(0, 1, 0, 1), 1, 0, 0, true);
    scene.push_back(p_cube);

    p_shift.z += p_side;
    p_base = new Rectangle(Vec(0, 0, 0) + p_shift, Vec(0, p_side, 0) + p_shift, Vec(p_side, p_side, 0) + p_shift, Vec(p_side, 0, 0) + p_shift);
    p_cube = new Cube(*p_base, Color(0, 1, 0, 1), 1, 0, 0, true);
    scene.push_back(p_cube);

    p_shift.z -= p_side;
    p_shift.x -= p_side;
    p_base = new Rectangle(Vec(0, 0, 0) + p_shift, Vec(0, p_side, 0) + p_shift, Vec(p_side, p_side, 0) + p_shift, Vec(p_side, 0, 0) + p_shift);
    p_cube = new Cube(*p_base, Color(0, 1, 0, 1), 1, 0, 0, true);
    scene.push_back(p_cube);

    p_shift.z += p_side;
    p_base = new Rectangle(Vec(0, 0, 0) + p_shift, Vec(0, p_side, 0) + p_shift, Vec(p_side, p_side, 0) + p_shift, Vec(p_side, 0, 0) + p_shift);
    p_cube = new Cube(*p_base, Color(0, 1, 0, 1), 1, 0, 0, true);
    scene.push_back(p_cube);

    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            Vec dir = (Vec(x, y, size) - me).normalize();
            Ray ray = Ray(me, dir);
            Collision hit = trace_ray(&scene, &lights, ray, 2);
            
            if (hit.hit) {
                buffer[x + y * size] = hit.color;
            } else {
                buffer[x + y * size] = Color(cos((float)y / size + 0.5), cos((float)y / size + 0.5), cos((float)y / size + 0.5), 1);
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
