#include <cstring>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

#include "geom.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

float SIZE = 1;

Vec refract_vec(Vec, Vec, float, float i_r = 1);
Vec reflect_vec(Vec, Vec);

float 
count_lights(std::vector<Object *> *scene, std::vector<Light> *lights, Ray ray, Collision hit) {
    float EPS = 1e-5;
    size_t len = scene->size();
    size_t lights_len = lights->size();
    Vec intersect(ray.pos + (ray.dir * hit.dist)  + (hit.normal.normalize() * EPS));
    float add = 0;
    //float add = 0.2 * std::max(0.f, dot(ray.dir.normalize(), hit.normal.normalize()));
   // float add = 0.2 * abs(dot(ray.dir.normalize(), hit.normal.normalize()));
    for (size_t i = 0; i < lights_len; i++) {
        Vec light_dir = (*lights)[i].pos - intersect;
        Ray to_light(intersect, light_dir.normalize());
        float way_coef = 0;
        bool result = true;
        float in_a = 0;
        std::vector<size_t> in_objects = std::vector<size_t>();
        while (true) {
            Collision intersected, tmp;
            size_t object = -1;
            for (size_t j = 0; j < len; j++) {
                tmp = (*scene)[j]->intersect(to_light);
                if (tmp.hit && tmp.real) {
                    if (!intersected.hit || (intersected.dist - tmp.dist > EPS)) {
                        intersected = tmp;
                        object = j;
                    }
                }
            }
            if (!intersected.hit) {
                float dist = ((*lights)[0].pos - to_light.pos).len();
                way_coef += (1 + in_a) * dist;
                break;
            } else { // walked into something
               if (intersected.color.a < 1) {
                   way_coef += (1 + in_a) * intersected.dist * ((intersected.absorbtion == 0) ? 1 : exp(-intersected.absorbtion * intersected.dist));
                   to_light.pos = to_light.pos + (to_light.dir * intersected.dist);
                    
                   if (in_objects.size() > 0 && in_objects[in_objects.size() - 1] == object) { // out
                        in_objects.pop_back();
                        if (in_objects.empty()) {
                            in_a = 0;
                        } else {
                            in_a = (*scene)[in_objects.back()]->color.a;
                        }
                    } else {
                        in_objects.push_back(object);
                        in_a = intersected.color.a;
                    }
                } else {
                    result = false;
                    break;
                }
             }
        }
        if (result) {
           // float angle = std::max(0.f, dot(light_dir.normalize(), hit.normal.normalize()));
            float angle = abs(dot(light_dir.normalize(), hit.normal.normalize()));
            float light = angle * ((*lights)[i].intensity / (way_coef * way_coef));
            add += light;
        }
    }
    return add;
}

Collision
trace_ray(std::vector<Object *> *scene, std::vector<Light> *lights, Ray ray, int deep, int reality) {
    Collision hit, tmp;
    if (deep <= 0) {
        return hit;
    }

    size_t len = scene->size();
    float EPS = 1e-5;
    int obj_ind = -1;
    for (size_t i = 0; i < len; i++) {
        tmp = (*scene)[i]->intersect(ray);
        if (tmp.hit && (!hit.hit || hit.dist - tmp.dist > EPS)) {
            if ((reality == -1) || tmp.real || !reality) {
                hit = tmp;
                obj_ind = i;
                hit.object = i;
            }
        }
    }
    if (hit.hit) {
        
        if (hit.color.a < 1 && hit.reflection < 1) { // through object
            Vec refracted;
            if (ray.objects.size() && ray.objects.back() == obj_ind) {
                float out = (*scene)[ray.objects.back()]->refraction;
                float inner = (ray.objects.size() > 1) ? (*scene)[ray.objects.at(ray.objects.size() - 2)]->refraction : 1;
                refracted = refract_vec(ray.dir, hit.normal, out, inner);
            } else {
                refracted = refract_vec(ray.dir, hit.normal, hit.refraction, (ray.objects.size() ? (*scene)[ray.objects.back()]->refraction : 1));
            }
            if (refracted.len() == 0) {
                // full inner reflection
                hit.color = (hit.color * hit.color.a);
            } else {
                Ray through_ray(ray.pos + (ray.dir * (hit.dist + EPS)), refracted);
                through_ray.objects = ray.objects;
                if (through_ray.objects.empty() || through_ray.objects.back() != obj_ind) {
                    through_ray.objects.push_back(obj_ind);
                } else {
                    through_ray.objects.pop_back();
                }
                Collision res = trace_ray(scene, lights, through_ray, deep - 1, (!obj_ind && reality == -1) ? false : hit.real);          

                if (res.hit) {
                    hit.color = (hit.color * hit.color.a) + (res.color * (1 - hit.color.a));
                }
            }
        }

        if (hit.reflection > 0 && dot(ray.dir, hit.normal) <= 0) { // other ray
            
            Ray reflected_ray(ray.pos + (ray.dir * hit.dist) + (hit.normal * 0.0001), reflect_vec(ray.dir, hit.normal));
            Collision reflected = trace_ray(scene, lights, reflected_ray, deep - 1, hit.real);
            if (reflected.hit) {
                hit.color = (hit.color * (1 - hit.reflection)) + (reflected.color * hit.reflection);
            } else {
                hit.color = hit.color * (1 - hit.reflection);
            }
            
            hit.color = hit.color * count_lights(scene, lights, ray, hit);
            float light_add = 0;
            for (size_t i = 0; i < lights->size(); i++) {
                Vec light_dir = (*lights)[i].pos - reflected_ray.pos;
                if (!trace_ray(scene, lights, Ray(reflected_ray.pos, light_dir), 1, reality).hit) {
                    float angle = abs(dot(light_dir.normalize(), hit.normal.normalize()));
                    int N = 1500;
                    light_add += pow(angle, N) * (*lights)[i].intensity / (light_dir.len() * light_dir.len());
                }
            }
            
            hit.color = hit.color + (Color(1, 1, 1, 1) *  (light_add * hit.reflection));
        } else {
            hit.color = hit.color * count_lights(scene, lights, ray, hit);
        }
        if (hit.object > 0 && hit.object < scene->size() - 1) {
            Ray x_plus_ray = Ray(Vec(ray.pos.x + SIZE / 60, ray.pos.y, ray.pos.z), ray.dir);
            Ray x_minus_ray = Ray(Vec(ray.pos.x - SIZE / 60, ray.pos.y, ray.pos.z), ray.dir);
            Ray y_plus_ray = Ray(Vec(ray.pos.x, ray.pos.y + SIZE / 60, ray.pos.z), ray.dir);
            Ray y_minus_ray = Ray(Vec(ray.pos.x, ray.pos.y - SIZE / 60, ray.pos.z), ray.dir);
            Collision x_plus_col = trace_ray(scene, lights, x_plus_ray, std::min(2, deep - 1), reality);
            Collision x_minus_col = trace_ray(scene, lights, x_minus_ray, std::min(2, deep - 1), reality);
            Collision y_plus_col = trace_ray(scene, lights, y_plus_ray, std::min(2, deep - 1), reality);
            Collision y_minus_col = trace_ray(scene, lights, y_minus_ray, std::min(2, deep - 1), reality);
            hit.color = (hit.color * 0.6) + (x_plus_col.color * 0.1) + (x_minus_col.color * 0.1) + (y_plus_col.color * 0.1) + (y_minus_col.color * 0.1);
        }    
    }
    
    return hit;
}

Vec
refract_vec(Vec v, Vec normal, float refract, float init_refract) {
    float alpha = acos(dot(v.normalize(), normal.normalize() * (-1)));
    float beta = asin(sin(alpha) * init_refract / refract);
    if (beta >= (M_PI / 2)) {
        return Vec(0, 0, 0);
    }
    Vec axis = cross(v, normal);
    return rotateAroundAxis(v, axis, beta - alpha);
}

Vec
reflect_vec(Vec v, Vec normal) {
   v = v.normalize();
   normal = normal.normalize();
   return (normal * (abs(dot(v, normal)) * 2) - v).normalize();
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
    float me_dist = size * 3;
    Vec me(size / 5, 0, -me_dist);
    
    std::vector<Light> lights = std::vector<Light>();
    lights.push_back(Light(Vec(size, -size / 2, -me_dist), 16 * size * size));
    lights.push_back(Light(Vec(size / 2, -size, 0), 5 * size * size));
    
    std::vector<Object*> scene = std::vector<Object*>();
    std::vector<Object*> sphere = std::vector<Object*>();
    std::vector<Object*> empty = std::vector<Object*>();

    
    Vec d_center(size / 2, size / 2, 0);
    Vec d_normal(0, 1, 0);
    float d_size = size / 3;
    Color d_color(158.f/255, 95.f/255, 189.f/255, 0.7);
    Dodekaedr d(d_center, d_normal, d_size, d_color, 1.6, 0.2, 0.01, true);
       
    scene.push_back(&d);
    float a = d_size;
    float b = a / sqrt(2 - 2 * cos(2 * M_PI / 5));
    float r = a * (1 + sqrt(5)) * sqrt(3) / 4;
    float k = sqrt(r * r - b * b);

    float side = size / 10; 
    Vec shift(d_center - Vec(side / 2, side / 2, 0));
    Rectangle base(Vec(0, 0, 0) + shift, Vec(0, side, 0) + shift, Vec(side, side, 0) + shift, Vec(side, 0, 0) + shift);
    Cube cube(Cube(base, Color(0, 1, 0, 0.1), 1.6, 0, 0, false));
    int w, h;
    unsigned char *bedrock = stbi_load("Resources/bedrock.jpg", &w, &h, NULL, 3);
    unsigned char *ender_stone = stbi_load("Resources/ender_stone.jpg", &w, &h, NULL, 3);
    cube.texture = ender_stone;
   // scene.push_back(&cube);
    

    Cube *other_cube = new Cube(cube + Vec(side, 0, 0));
    scene.push_back(other_cube);
    other_cube = new Cube(cube + Vec(-side, 0, 0));
    scene.push_back(other_cube);
    other_cube = new Cube(cube + Vec(0, 0, side));
    scene.push_back(other_cube);
    other_cube = new Cube(cube + Vec(0, 0, -side));
    scene.push_back(other_cube);
    other_cube = new Cube(cube + Vec(0, side, 0));
    scene.push_back(other_cube);
    other_cube = new Cube(cube + Vec(0, -side, 0));
    other_cube->texture = bedrock;
    scene.push_back(other_cube);
  
  
    // podstavka

    Color p_color(101.f/255, 131.f/255, 50.f/255, 1);
 // MAGIC SHIFTS 
    float p_side = size / 1.5;
    Vec p_shift = d_center + (d_normal * k);
    p_shift.x -= p_side / 2;
    p_shift.z += p_side / 2;
    p_shift.y += 10;
    Rectangle* p_base = new Rectangle(Vec(0, 0, 0) + p_shift, Vec(0, p_side, 0) + p_shift, Vec(p_side, p_side, 0) + p_shift, Vec(p_side, 0, 0) + p_shift);
    Cube* p_cube = new Cube(*p_base, p_color, 1, 0, 0, true);
    scene.push_back(p_cube);

    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            Vec dir = (Vec(x, y, 0) - me).normalize();
            Ray ray = Ray(me, dir);
            Collision hit = trace_ray(&scene, &lights, ray, 6, -1);
            
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
    stbi_image_free(bedrock);
    stbi_image_free(ender_stone);
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
