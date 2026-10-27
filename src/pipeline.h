#pragma once
#include "geometry.h"
#include "tgaimage.h"
#include <utility>
#include <vector>

extern std::vector<double> zbuffer;

void lookat(const vec3 eye, const vec3 center, const vec3 up);
void init_perspective(const double f);
void init_viewport(const int x, const int y, const int w, const int h);
void init_zbuffer(const int w, const int h);
vec4 rot(const vec4 v, const double a);

struct IShader {
    static TGAColor sample2D(const TGAImage &img, const vec2 &uvf) {
        return img.get(uvf[0] * img.width(), uvf[1] * img.height());
    }

    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const = 0;
};

typedef vec4 Triangle[3];
void rasterize(const Triangle& clip, const IShader& shader, TGAImage& framebuffer);

