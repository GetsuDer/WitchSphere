#include "geom.h"
#include <iostream>
#include <set>
#include <cmath>

Color
mix(Color under, Color over) {
    Color res = over * over.a + under * (1 - over.a);
    res.a = over.a + under.a * (1 - over.a); 
    return res;
}

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
        float len = sqrt(x * x + y * y + z * z);
        return Vec(x / len, y / len, z / len);
}

float
Vec::len() {
    return sqrt(x * x + y * y + z * z);
}

Vec rotateAroundAxis(Vec vector, Vec axis, float angle) {
    Vec res;
    axis = axis.normalize();
    res = vector * cos(angle) + cross(axis, vector) * sin(angle) + axis * dot(axis, vector) * (1 - cos(angle));
    return res;
}

Collision
intersect(Triangle t, Ray r) {
    Collision res;

    Vec e1 = t.b - t.a;
    Vec e2 = t.c - t.a;
    // normal vector
    Vec pvec = cross(r.dir, e2);
    float det = dot(e1, pvec);
    float EPS = 1;
    // || ploskosti
    if (det < EPS && det > -EPS) {
        return res;
    }

    float inv_det = 1 / det;
    Vec tvec = r.pos - t.a;
    float u = dot(tvec, pvec) * inv_det;
    if (u < 0 || u > 1) {
        return res;
    }

    Vec qvec = cross(tvec, e1);
    float v = dot(r.dir, qvec) * inv_det;

    if (v < 0 || u + v > 1) {
        return res;
    }
    
    res.hit = true;
    res.dist = dot(e2, qvec) * inv_det;
    res.normal = cross(e1, e2);

    if (res.dist < EPS) {
        res.hit = false;
    }
    return res;
}

Collision
Object::intersect(Ray ray) {
    Collision res, tmp;
    if (!triangles) {
        return res;
    }
    for (int i = 0; i < triangles_number; i++) {
        tmp = ::intersect(triangles[i], ray);
        if (tmp.hit && (!res.hit || res.dist > tmp.dist)) {
            res = tmp;
        }
    }
    res.color = color;
    return res;
}

