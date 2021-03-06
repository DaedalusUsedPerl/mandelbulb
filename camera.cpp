#include "camera.h"
#include <vector>
#include <algorithm>
#include <Eigen/Dense>
#include <iostream>
#include <stdio.h>
#include <complex>

using namespace std;

void printNamedVector3(string p, Vector3f v) {
    cout << p << ": <" << v[0] << ", " << v[1] << ", " << v[2] << ">" << endl;
}

void printNamedVector4(string p, Vector4f v) {
    cout << p << ": <" << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << ">" << endl;
}

Vector4f tov4(const Vector3f& v) {
    return Vector4f(v[0], v[1], v[2], 0);
}

float vec3length(const Vector3f& v){
    return sqrt(abs(v.dot(v)));
}

void Camera::resln(int w, int h) {
    width = w;
    height = h;
    colors.resize(w * h);
}

Vector4f Camera::getDir(float ix, float iy) {
    float x = left + (ix / width)*(right - left);
    float y = bottom + (iy / height)*(top - bottom);
    Vector4f d((near * tov4(dir)) + (x * tov4(h_dir)) + (y * tov4(v_dir)));
    return d.normalized();
}

void Camera::renderPixel(int ix, int iy) {
    ray_t ray = {tov4(pos), getDir((float) ix, (float) iy)};
    Vector4f color = trace(ray, 1);
    setColor(ix, iy, color);

}

void Camera::render() {
    dir = (-1 * pos).normalized();
    h_dir = (dir.cross(up)).normalized();
    v_dir = (h_dir.cross(dir)).normalized();
    for (int iy = 0; iy < height; iy++) {
        for (int ix = 0; ix < width; ix++) {
            renderPixel(ix, iy);
        }
        if (iy % 10 == 0) cout << iy << endl;
    }
    write();
}

void Camera::write() {
    FILE *fp = fopen(out.c_str(), "w");
    if (!fp) {
        cout << "Could not open file" << endl;
        exit(-1);
    }
    fprintf(fp, "P3\n%d %d\n255\n", width, height);
    for (int i = 0; i < colors.size(); ++i) {
        int r = (int) (colors[i])[0];
        int g = (int) (colors[i])[1];
        int b = (int) (colors[i])[2];
        fprintf(fp, "%d %d %d ", r, g, b);
    }
    fclose(fp);
}

void Camera::setColor(int ix, int iy, const Vector4f& color) {
    int iy2 = height - iy - 1; // Invert iy coordinate.
    colors[iy2 * width + ix] = color;
}

int iterate(const Vector3f& pos) {
    Vector3f c(pos[0], pos[1], pos[2]);
    int i = 0;
    for (; i < 100; ++i) {
        float x = c[0], y = c[1], z = c[2];
        float r = sqrt(x*x + y*y + z*z);
        float theta = atan2(sqrt(x*x + y*y), z);
        float phi = atan2(y, x);
        c = (pow(r, 8) * 
                Vector3f(sin(theta*8)*cos(phi*8),
                         sin(theta*8)*sin(phi*8),
                         cos(theta*8))) + pos;
        if (vec3length(c) < 2.0) return i;
    }
    return -1;
}

Vector4f Camera::trace(ray_t r, int d) {
    Vector3f from(r.origin[0], r.origin[1], r.origin[2]);
    Vector3f direction(r.dir[0], r.dir[1], r.dir[2]);
    int steps = 0;
    Vector3f p;
    for (; steps < nSteps; ++steps) {
        p = from + (direction * steps * step_disance);
        int i = iterate(p);
        if (i > -1) {
            float factor = 1.0f - ((float) i) / 100.0f;
            return factor * Vector4f(255.0f, 255.0f, 255.0f, 0.0f);
        }
    }
    return Vector4f(0.0f, 0.0f, 0.0f, 0.0f);
}
