#include "tgaimage.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <vector>
#include "my_model.h"
#include "geometry.h"
#include "pipeline.h"

extern mat<4,4> ModelView, Perspective;

struct PhongShader : IShader {
    vec3 tri[3];
    vec3 n;
    vec3 l;
    TGAColor color = {};
    double ka = 0.2; // ambient coefficient
    double kd = 0.7;
    double ks = 0.5;
    int e = 32; // lightening exponent

    std::pair<bool, TGAColor> fragment(const vec3 bc) const override {
        vec3 pos = tri[0]*bc.x + tri[1]*bc.y + tri[2]*bc.z;
        vec3 v = normalized(vec3{0,0,0} - pos);
        vec3 r = normalized(n*(2*(n*l))-l);

        double diff = std::max(0., n*l);
        double spec = std::pow(std::max(0., r*v), e);
        double intencity = std::min(1., ka + kd*diff + ks*spec);
        TGAColor c = color; // copy because const space
        for (int ch : {0,1,2}) c[ch] = std::min(255., c[ch]*intencity); // B G R multiply by intensity
        return {false, color};
    }

};

int main(int argc, char** argv) {
  if (argc < 2) {
      std::cerr << "Usage: " << argv[0] << " obj/model.obj ..." << std::endl;
      return 1;
  }

  constexpr int width = 800;
  constexpr int height = 800;

  constexpr int frames = 120;
  constexpr vec3 eye{-1,0,2};
  constexpr vec3 center{0, 0, 0};
  constexpr vec3 up{0, 1, 0};

  lookat(eye, center, up);
  init_perspective(norm(eye-center));
  init_viewport(width/16, height/16, width*7/8, height*7/8);

  std::vector<Model> models;
  for (int m = 1; m < argc; m++)
      models.emplace_back(argv[m]);

  const vec3 light = normalized(vec3{1,1,1}); // light is fix convert to view space
  const vec3 light_view = normalized((ModelView * vec4{light.x, light.y, light.z, 0.}).xyz());

  for (int f=0; f<frames; f++) {
      double a = 2*M_PI * f/frames;

      TGAImage framebuffer(width, height, TGAImage::RGB);
      init_zbuffer(width, height); // reset depth every frame
                                   //
      PhongShader shader;
      shader.l = light_view;
      shader.color = {255, 255, 255, 255};

      for (const Model& model: models) {
          for (int face=0; face<model.nfaces(); face++) {
              vec4 clip[3];
              for (int i: {0,1,2}) {
                  vec3 v = rot(model.vert(face, i), a);
                  vec4 vv = ModelView * vec4{v.x, v.y, v.z, 1.};
                  shader.tri[i] = vv.xyz();
                  clip[i] = Perspective * vv;
              }
              shader.n = normalized(cross(shader.tri[1] - shader.tri[0], shader.tri[2] - shader.tri[0]));
              rasterize(clip, shader, framebuffer);
          }
      }
    char name[64];
    std::snprintf(name, sizeof name, "frames/frame_%03d.tga", f);
    framebuffer.write_tga_file(name);
  }
  return 0;
}
