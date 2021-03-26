#include <cmath>
#include <set>

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
    Vec operator*(const float a) const {
        return Vec(x * a, y * a, z * a);
    }
}; 

float dot(Vec, Vec);
Vec cross(Vec, Vec);
Vec rotateAroundAxis(Vec, Vec, float);

struct Ray {
    Vec pos;
    Vec dir;
    Ray(Vec _pos, Vec _dir) : pos(_pos), dir(_dir) {}
};

struct Triangle {
    Vec a, b, c;
    Triangle() : a(), b(), c() {}
    Triangle(Vec _a, Vec _b, Vec _c) : a(_a), b(_b), c(_c) {}
};

float intersect(Triangle, Ray);

struct Object {
    int triangles_number;
    Triangle *triangles;
    Object() {
        triangles_number = 0;
        triangles = NULL;
    }
    float intersect(Ray r);
};

struct Rectangle : public Object {
    // from left down to right down
    Rectangle(Vec a, Vec b, Vec c, Vec d) {
        triangles_number = 2;
        triangles = new Triangle[2];
        triangles[0] = Triangle(a, b, c);
        triangles[1] = Triangle(a, c, d);
    }
};

struct Pentagon : public Object {
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
