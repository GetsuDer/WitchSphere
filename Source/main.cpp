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
    float EPS = 1e-3;
    for (size_t i = 0; i < len; i++) {
        tmp = (*scene)[i]->intersect(ray);
        if (tmp.hit && (!hit.hit || hit.dist - tmp.dist > EPS)) {
            hit = tmp;
        }
    }
    return hit;
}

float
find_light(std::vector<Object *> *scene, std::vector<Light> *lights, Vec intersect, Collision hit) {
    size_t light_len = lights->size();
    float add = 0;
    for (size_t i = 0; i < light_len; i++) {
        Vec light_dir = (*lights)[i].pos - intersect;
        Ray to_light(intersect, light_dir.normalize());
        Collision intersected = find_hit(scene, to_light);
        if (!intersected.hit) {
            float angle = abs(dot(light_dir.normalize(), hit.normal.normalize()));
            float distance = light_dir.len();
            add += angle * ((*lights)[i].intensity / (distance * distance));
        }
    }
    return add;
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
    std::vector<Color> buffer(size * size);
    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            buffer[x + y * size] = Color(0, 0, 0, 1.f);
        }
    }
    std::vector<Light> lights = std::vector<Light>();
    lights.push_back(Light(Vec(size * 3, 0, -size * 3), 40 * size * size));
    lights.push_back(Light(Vec(size / 3, -size * 1.1, 0), 8 * size * size));
    
    std::vector<Object*> scene = std::vector<Object*>();
    std::vector<Object*> sphere = std::vector<Object*>();
    std::vector<Object*> empty = std::vector<Object*>();

    float me_dist = size * 2;
    Vec me(0, 0, -me_dist);
    
    Vec d_center(size / 3, size / 3, 0);
    Vec d_normal(0, 1, 0);
    float d_size = size / 4.5;
    Dodekaedr d(d_center, d_normal, d_size, Color(1, 0, 0, 0.7), 1.05);
   
    float a = d_size;
    float b = a / sqrt(2 - 2 * cos(2 * M_PI / 5));
    float r = a * (1 + sqrt(5)) * sqrt(3) / 4;
    float k = sqrt(r * r - b * b);

    float side = size / 5; 
    Vec shift(size / 4, size / 4, size / 3);
    Rectangle base(Vec(0, 0, 0) + shift, Vec(0, side, 0) + shift, Vec(side, side, 0) + shift, Vec(side, 0, 0) + shift);
    Cube cube(base, Color(0, 1, 1, 1), 1);

    sphere.push_back(&d);
    scene.push_back(&cube);

    // podstavka

    float p_side = size / 4;
    Vec p_shift = d_center + (d_normal * k);
    Rectangle* p_base = new Rectangle(Vec(0, 0, 0) + p_shift, Vec(0, p_side, 0) + p_shift, Vec(p_side, p_side, 0) + p_shift, Vec(p_side, 0, 0) + p_shift);
    Cube* p_cube = new Cube(*p_base, Color(0, 1, 0, 1), 1);
    sphere.push_back(p_cube);

    p_shift.z += p_side;
    p_base = new Rectangle(Vec(0, 0, 0) + p_shift, Vec(0, p_side, 0) + p_shift, Vec(p_side, p_side, 0) + p_shift, Vec(p_side, 0, 0) + p_shift);
    p_cube = new Cube(*p_base, Color(0, 1, 0, 1), 1);
    sphere.push_back(p_cube);

    p_shift.z -= p_side;
    p_shift.x -= p_side;
    p_base = new Rectangle(Vec(0, 0, 0) + p_shift, Vec(0, p_side, 0) + p_shift, Vec(p_side, p_side, 0) + p_shift, Vec(p_side, 0, 0) + p_shift);
    p_cube = new Cube(*p_base, Color(0, 1, 0, 1), 1);
    sphere.push_back(p_cube);

    p_shift.z += p_side;
    p_base = new Rectangle(Vec(0, 0, 0) + p_shift, Vec(0, p_side, 0) + p_shift, Vec(p_side, p_side, 0) + p_shift, Vec(p_side, 0, 0) + p_shift);
    p_cube = new Cube(*p_base, Color(0, 1, 0, 1), 1);
    sphere.push_back(p_cube);

    float base_sphere_light = 0.2;
    float base_scene_light = 0.4;
    float EPS = 5;

    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            Vec dir = (Vec(x, y, size) - me).normalize();
            Ray ray = Ray(me, dir);
            Collision hit = find_hit(&sphere, ray);
            //find_hit(&scene, ray);
            
            if (hit.hit) {
                // dodekaedr color
                Vec intersect_sphere = ray.pos + (ray.dir * hit.dist);
                float d_add = find_light(&sphere, &lights, intersect_sphere, hit);
                buffer[x + y * size] = hit.color * (base_sphere_light * d_add);
                
                
                Ray inside_ray = Ray(intersect_sphere, reflect_vec(ray.dir, hit.normal, hit.reflect));
                Collision scene_hit = find_hit(&scene, inside_ray);
                if (scene_hit.hit) {
                    Vec intersect = intersect_sphere + (ray.dir * scene_hit.dist);
                    float add = find_light(&scene, &lights, intersect, scene_hit);
                    buffer[x + y * size] = buffer[x + y * size] + scene_hit.color * (base_scene_light * add);
                    
                } else {
                    inside_ray.pos = inside_ray.pos + inside_ray.dir * EPS;
                    hit = find_hit(&sphere, inside_ray);
                    if (hit.hit) {
                        intersect_sphere = inside_ray.pos + (inside_ray.dir * hit.dist);
                        d_add = find_light(&empty, &lights, intersect_sphere, hit);
                        buffer[x + y * size] = buffer[x + y * size] + hit.color * (base_sphere_light * d_add);
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
