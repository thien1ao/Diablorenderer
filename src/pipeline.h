#pragma once
#include "geometry.h"
#include "tgaimage.h"
#include <utility>

void lookat(const vec3 eye, const vec3 center, const vec3 up);
void init_perspective(const double f);
void init_viewport(const int x, const int y, const int w, const int h);
void init_zbuffer(const int w, const int h);
vec3 rot(const vec3 v, const double a); // rotate around the y axis

// virtual interface responsibe for signature of the method but realisation is on other classes
struct IShader {
    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const = 0;
};

struct MyShader : IShader {
  std::pair<bool, TGAColor> fragment(const vec3 bar) const override {
      TGAColor color{255, 0, 255};
      return {false, color};
  }
};

typedef vec4 Triangle[3];
void rasterize(const Triangle& clip, const IShader& shader, TGAImage& framebuffer);

