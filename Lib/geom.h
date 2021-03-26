#include <cmath>

struct Color {
    float r;
    float g;
    float b;
    float a;
    Color(float _r = 0, float _g = 0, float _b = 0, float _a = 0) : r(_r), g(_g), b(_b), a(_a) {}
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
}; 

float dot(Vec, Vec);
Vec cross(Vec, Vec);

struct Ray {
    Vec pos;
    Vec dir;
    Ray(Vec _pos, Vec _dir) : pos(_pos), dir(_dir) {}
};

struct Triangle {
    Vec a, b, c;
    Triangle(Vec _a, Vec _b, Vec _c) : a(_a), b(_b), c(_c) {}
};

float intersect(Triangle, Ray);
