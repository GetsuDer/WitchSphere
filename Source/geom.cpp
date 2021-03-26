#include "geom.h"
#include <iostream>

float 
dot(Vec v1, Vec v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Vec 
cross(Vec v1, Vec v2) {
    float x = v1.y * v2.z - v1.z * v2.y;
    float y = v1.z * v2.x - v1.x * v2.z;
    float z = v1.x * v2.y - v1.y * v2.x;
    return Vec(x, y, z);
}

Vec 
Vec::normalize() {
        float len = sqrt( x * x + y * y + z * z);
        return Vec(x / len, y / len, z / len);
}

float
intersect(Triangle t, Ray r) {
    Vec e1 = t.b - t.a;
    Vec e2 = t.c - t.a;
    // normal vector
    Vec pvec = cross(r.dir, e2);
    float det = dot(e1, pvec);

    // || ploskosti
    if (det < 1e-8 && det > -1e-8) {
        return 0;
    }

    float inv_det = 1 / det;
    Vec tvec = r.pos - t.a;
    float u = dot(tvec, pvec) * inv_det;
    if (u < 0 || u > 1) {
        return 0;
    }

    Vec qvec = cross(tvec, e1);
    float v = dot(r.dir, qvec) * inv_det;

    if (v < 0 || u + v > 1) {
        return 0;
    }

    return dot(e2, qvec) * inv_det;
}

float
Object::intersect(Ray ray) {
    if (!triangles) {
        return 0;
    }
    float res = 0;
    for (int i = 0; i < triangles_number; i++) {
        float dist = ::intersect(triangles[i], ray);
        if (dist > 0 && (res == 0 || dist < res)) {
            res = dist;
        }
    }
    return res;
}
