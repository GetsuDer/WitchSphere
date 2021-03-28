#include <cmath>
#include <set>
#include <iostream>
#include <algorithm>

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

    Color operator-(const Color c) const {
        return Color(std::max(0.f, r - c.r), std::max(0.f, g - c.g), std::max(0.f, b - c.b), a);
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
    float len();
}; 

float dot(Vec, Vec);
Vec cross(Vec, Vec);
Vec rotateAroundAxis(Vec, Vec, float);

struct Ray {
    Vec pos;
    Vec dir;
    Ray(Vec _pos, Vec _dir) : pos(_pos), dir(_dir) {}
};

struct Collision {
    bool hit;
    float dist;
    Vec normal;
    Collision() : hit(false), dist(), normal() {}        
};

struct Triangle {
    Vec a, b, c;
    Triangle() : a(), b(), c() {}
    Triangle(Vec _a, Vec _b, Vec _c) : a(_a), b(_b), c(_c) {}
};

Collision intersect(Triangle, Ray);

struct Object {
    int triangles_number;
    Triangle *triangles;
    Object() {
        triangles_number = 0;
        triangles = NULL;
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

    Cube(Rectangle base) {
        rectangles_number = 6;
        rectangles = new Rectangle[rectangles_number];
        rectangles[0] = base;
        
        Vec e1 = base.triangles[0].b - base.triangles[0].a;
        Vec e2 = base.triangles[0].c - base.triangles[0].b;
        float side = e1.len();
        Vec shift = cross(e1, e2).normalize() * side;
        rectangles[1] = rectangles[0] + shift;
    
        Vec a(rectangles[0].triangles[0].a);
        Vec b = a + shift;
        Vec c = b + e1;
        Vec d = a + e1;
        
        rectangles[2] = Rectangle(a, b, c, d);
        rectangles[3] = rectangles[2] + e2;
        
        c = b + e2;
        d = a + e2;
        rectangles[4] = Rectangle(a, b, c, d);
        rectangles[5] = rectangles[4] + e1;
    }

    Collision intersect(Ray ray) {
        Collision res, tmp;
        for (int i = 0; i < rectangles_number; i++) {
            tmp = rectangles[i].intersect(ray);
            if (tmp.hit) {
                if (!res.hit || res.dist > tmp.dist) {
                    res = tmp;
                }
            }
        }
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

    Dodekaedr(Vec center, Vec v, float a) {
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
            pentagons[i + 1] = Pentagon(other_center, tmp2 * b, other_center - center);
        }

        v = v * (-1);
        side = rotateAroundAxis(side, v, (2 * M_PI) / 10);
        pentagons[11] = Pentagon(center + (v * d), side * b, v);
        for (int i = 0; i < 5; i++) {
            Vec tmp1 = rotateAroundAxis(side, v, M_PI / 5 + i * (2 * M_PI / 5));   
            Vec tmp2 = rotateAroundAxis(tmp1, cross(v, tmp1), acos(1 / sqrt(5)));
            Vec other_center = center + (v * d) + (tmp1 * c) + (tmp2 * c);
            pentagons[i + 6] = Pentagon(other_center, tmp2 * b, other_center - center);
        }
    }

        
    Collision intersect(Ray r) {
        Collision tmp, res;
        for (int i = 0; i < pentagon_number; i++) {
            tmp = pentagons[i].intersect(r);
            if (tmp.hit && (!res.hit || res.dist > tmp.dist)) {
                res = tmp;
            }
        }
        return res;
    }
    
};

struct Light {
    Vec pos;
    float intensity;
    Light() {}
    Light(Vec _pos, float _intensity) : pos(_pos), intensity(_intensity) {}
};
