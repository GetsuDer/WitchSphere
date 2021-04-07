#include <cmath>
#include <set>
#include <iostream>
#include <algorithm>

#include "stb_image.h"

struct Color {
    float r;
    float g;
    float b;
    float a;
    Color(float _r = 0, float _g = 0, float _b = 0, float _a = 0) : r(_r), g(_g), b(_b), a(_a) {}
    const Color operator*(float f) {
        return Color(r * f, g * f, b * f, a);
    }

    const Color operator/(float f) {
        return Color(r / f, g / f, b / f, a);
    }

    Color operator+(const Color c) const {
        return Color(r + c.r, g + c.g, b + c.b, a);
    }
};

struct Vec {
    float x, y, z;
    Vec(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}
    Vec normalize();
    Vec operator-(const Vec& v) const {
       return Vec(x - v.x, y - v.y, z - v.z);
    } 
    Vec operator+(const Vec& v) const {
        return Vec(x + v.x, y + v.y, z + v.z);
    }
    Vec operator*(const float a) const {
        return Vec(x * a, y * a, z * a);
    }
    Vec operator/(const float a) const {
        return Vec(x / a, y / a, z / a);
    }

    float len();
}; 

float dot(Vec, Vec);
Vec cross(Vec, Vec);
Vec rotateAroundAxis(Vec, Vec, float);

struct Ray {
    Vec pos;
    Vec dir;
    float init_refraction;
    Ray(Vec _pos, Vec _dir, float ref = 1) : pos(_pos), dir(_dir), init_refraction(ref) {}
};

struct Collision {
    bool hit;
    float dist;
    Vec normal;
    Color color;
    float refraction;
    float reflection;
    float absorbtion;
    bool real;
    Collision() : real(true), hit(false), dist(), normal(), color() {}        
};

struct Triangle {
    Vec a, b, c;
    Triangle() : a(), b(), c() {}
    Triangle(Vec _a, Vec _b, Vec _c) : a(_a), b(_b), c(_c) {}
};

Collision intersect(Triangle, Ray);

struct Object {
    bool real;
    int triangles_number;
    Triangle *triangles;
    Color color;
    float refraction; // преломление
    float reflection; // отражение 
    float absorbtion; // полгощение
    Object() {
        triangles_number = 0;
        triangles = NULL;
        color = Color(0, 0, 0, 1);
        reflection = 0;
        refraction = 1;
        absorbtion = 0;
    }
    virtual Collision intersect(Ray r);
};

struct Rectangle : public Object {
    // from left down to right down
    Rectangle() {}
    Rectangle(Vec a, Vec b, Vec c, Vec d) {
        triangles_number = 2;
        triangles = new Triangle[2];
        triangles[0] = Triangle(a, b, c);
        triangles[1] = Triangle(a, c, d);
    }

    const Rectangle operator+(Vec v) {
        return Rectangle(triangles[0].a + v, triangles[0].b + v, triangles[0].c + v, triangles[1].c + v);
    }

    const Rectangle operator-(Vec v) {
        return Rectangle(triangles[0].a - v, triangles[0].b - v, triangles[0].c - v, triangles[1].c - v);
    }
    
};

struct Cube : public Object {
    int rectangles_number;
    Rectangle *rectangles;
    unsigned char *texture;
    int h, w;
    Cube(const Cube &cube) {
        texture = cube.texture;
        rectangles_number = cube.rectangles_number;
        refraction = cube.refraction;
        reflection = cube.reflection;
        absorbtion = cube.absorbtion;
        color = cube.color;
        h = cube.h;
        w = cube.w;
        real = cube.real;
        rectangles = new Rectangle[rectangles_number];
        for (int i = 0; i < rectangles_number; i++) {
            rectangles[i] = *(new Rectangle(cube.rectangles[i]));
        }
    }

    Cube(Rectangle base, Color col, float refract = 1, float reflect = 0, float absorb = 0, bool is_real = true) {
        texture = NULL;
        h = 256;
        w = 256;
        refraction = refract;
        reflection = reflect;
        absorbtion = absorb;

        real = is_real;

        rectangles_number = 6;
        rectangles = new Rectangle[rectangles_number];
        rectangles[0] = Rectangle(base.triangles[1].c, base.triangles[0].c, base.triangles[0].b, base.triangles[0].a);
        
        Vec e1 = base.triangles[0].b - base.triangles[0].a;
        Vec e2 = base.triangles[0].c - base.triangles[0].b;
        float side = e1.len();
        Vec shift = cross(e1, e2).normalize() * side;
        rectangles[1] = base + shift;
    
        Vec a(base.triangles[0].a);
        Vec b = a + shift;
        Vec c = b + e1;
        Vec d = a + e1;
        
        rectangles[2] = Rectangle(d, c, b, a);
        rectangles[3] = Rectangle(a, b, c, d) + e2;
        
        c = b + e2;
        d = a + e2;
        rectangles[4] = Rectangle(a, b, c, d);
        rectangles[5] = Rectangle(d, c, b, a) + e1;
        color = col;
    }
    Cube operator+(const Vec &vec) const {
        Cube *res = new Cube(*this);
        for (int i = 0; i < rectangles_number; i++) {
            res->rectangles[i] = rectangles[i] + vec;
        }
        return *res;
    }

    Cube operator-(const Vec &vec) const {
        Cube* res = new Cube(*this);
        for (int i = 0; i < rectangles_number; i++) {
            res->rectangles[i] = rectangles[i] + vec;
        }
        return *res;
    }

    Collision intersect(Ray ray) {
        Collision res, tmp;
        int rect_n = -1;
        for (int i = 0; i < rectangles_number; i++) {
            tmp = rectangles[i].intersect(ray);
            if (tmp.hit) {
                if (!res.hit || res.dist > tmp.dist) {
                    res = tmp;
                    rect_n = i;
                }
            }
        }
        if (!res.hit) {
            return res;
        }
        if (!texture) {
            res.color = color;
        } else {
            Vec y_vec = rectangles[rect_n].triangles[0].b - rectangles[rect_n].triangles[0].a;
            Vec x_vec = rectangles[rect_n].triangles[0].c - rectangles[rect_n].triangles[0].b;
            Vec intersect = (ray.pos + (ray.dir * res.dist)) - rectangles[rect_n].triangles[0].a;
            float y_f = abs(intersect.y / (y_vec.len() / h));
            float x_f = abs(intersect.x / (x_vec.len() / w));
            int x = floor(x_f);
            int y = floor(y_f);
            double x_ratio = x_f - x;
            double y_ratio = y_f - y;
            double x_opposite = 1 - x_ratio;
            double y_opposite = 1 - y_ratio;
            float red = (texture[(x + y * w) * 3] * x_opposite + texture[(x + 1 + y * w) * 3] * x_ratio) * y_opposite + (texture[(x + (y + 1) * w) * 3] * x_opposite + texture[(x + 1 + (y + 1) * w) * 3] * x_ratio) * y_ratio;
            float green = (texture[(x + y * w) * 3 + 1] * x_opposite + texture[(x + 1 + y * w) * 3 + 1] * x_ratio) * y_opposite + (texture[(x + (y + 1) * w) * 3 + 1] * x_opposite + texture[(x + 1 + (y + 1) * w) * 3 + 1] * x_ratio) * y_ratio;
            float blue = (texture[(x + y * w) * 3 + 2] * x_opposite + texture[(x + 1 + y * w) * 3 + 2] * x_ratio) * y_opposite + (texture[(x + (y + 1) * w) * 3 + 2] * x_opposite + texture[(x + 1 + (y + 1) * w) * 3 + 2] * x_ratio) * y_ratio;
            
            res.color = Color(red / 255, green / 255, blue / 255, 1);
        }
        res.refraction = refraction;
        res.reflection = reflection;
        res.absorbtion = absorbtion;
        res.real = real;
        return res;
    }
};

struct Pentagon : public Object {
    Pentagon() {}
    Pentagon(Vec center, Vec vec, Vec normal) {
        Vec a(center + vec);
        float angle = (2 * M_PI) / 5;
        Vec b(center + rotateAroundAxis(vec, normal, angle));
        Vec c(center + rotateAroundAxis(vec, normal, angle * 2));
        Vec d(center + rotateAroundAxis(vec, normal, angle * 3));
        Vec e(center + rotateAroundAxis(vec, normal, angle * 4));
        triangles_number = 3;
        triangles = new Triangle[3];
        triangles[0] = Triangle(a, b, c);
        triangles[1] = Triangle(a, c, e);
        triangles[2] = Triangle(c, d, e);
    }

};

struct Dodekaedr : public Object {
    int pentagon_number;
    Pentagon *pentagons;

    Dodekaedr(Vec center, Vec v, float a, Color col, float refract = 1, float reflect = 0, float absorb = 0, bool is_real = true) {
        refraction = refract;
        reflection = reflect;
        absorbtion = absorb;
        
        real = is_real;
        
        pentagon_number = 12;
        pentagons = new Pentagon[pentagon_number];
        
        float b = a / sqrt(2 - 2 * cos(2 * M_PI / 5));
        float r = a * (1 + sqrt(5)) * sqrt(3) / 4;
        float d = sqrt(r * r - b * b);
        float c = sqrt(b * b - (a / 2) * (a / 2));

        v = v.normalize() * (-1);
        Vec side = cross(v, v + Vec(1, 0, 0)).normalize();
        pentagons[0] = Pentagon(center + (v * d), side * b, v);
        for (int i = 0; i < 5; i++) {
            Vec tmp1 = rotateAroundAxis(side, v, M_PI / 5 + i * (2 * M_PI / 5));   
            Vec tmp2 = rotateAroundAxis(tmp1, cross(v, tmp1), acos(1 / sqrt(5)));
        
            Vec other_center = center + (v * d) + (tmp1 * c) + (tmp2 * c);
            pentagons[i + 1] = Pentagon(other_center, tmp2 * (b), (other_center - center) * (1));
        }

        v = v * (-1);
        side = rotateAroundAxis(side, v, (2 * M_PI) / 10);
        pentagons[11] = Pentagon(center + (v * d), side * (b), v);
        for (int i = 0; i < 5; i++) {
            Vec tmp1 = rotateAroundAxis(side, v, M_PI / 5 + i * (2 * M_PI / 5));   
            Vec tmp2 = rotateAroundAxis(tmp1, cross(v, tmp1), acos(1 / sqrt(5)));
            Vec other_center = center + (v * d) + (tmp1 * c) + (tmp2 * c);
            pentagons[i + 6] = Pentagon(other_center, tmp2 * (b), (other_center - center) * (1));
        }
        
        color = col;
    }

        
    Collision intersect(Ray r) {
        Collision tmp, res;
        for (int i = 0; i < pentagon_number; i++) {
            tmp = pentagons[i].intersect(r);
            if (tmp.hit && (!res.hit || res.dist > tmp.dist)) {
                res = tmp;
            }
        }
        res.color = color;
        res.refraction = refraction;
        res.reflection = reflection;
        res.absorbtion = absorbtion;
        res.real = real;
        return res;
    }
    
};

struct Light {
    Vec pos;
    float intensity;
    Light() {}
    Light(Vec _pos, float _intensity) : pos(_pos), intensity(_intensity) {}
};
