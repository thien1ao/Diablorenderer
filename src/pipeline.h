#pragma once
#include "geometry.h"
#include "tgaimage.h"
#include <utility>

void lookat(const vec3 eye, const vec3 center, const vec3 up);
void init_perspective(const double f);
void init_viewport(const int x, const int y, const int w, const int h);
void init_zbuffer(const int w, const int h);
vec4 rot(const vec4 v, const double a);

// virtual interface responsibe for signature of the method but realisation is on other classes
struct IShader {
    // static: no need to create IShader obj to call a function
    static TGAColor sample2D(const TGAImage &img, const vec2 &uvf) {
        return img.get(uvf[0] * img.width(), uvf[1] * img.height());
    }

    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const = 0;
};

typedef vec4 Triangle[3];
void rasterize(const Triangle& clip, const IShader& shader, TGAImage& framebuffer);
// segment between two points in eye coordinates, hidden where the z-buffer is closer
void line(const vec4 a, const vec4 b, const TGAColor color, TGAImage& framebuffer);

