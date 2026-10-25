#include "tgaimage.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include "my_model.h"
#include "geometry.h"
#include "pipeline.h"
#include "phong_shader.h"

int main(int argc, char** argv) {
  if (argc < 2) {
      std::cerr << "Usage: " << argv[0] << " [--white] [--wire] [--normals] [--grid] obj/model.obj ..." << std::endl;
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

  bool white = false, wireframe = false, normals = false;
  TGAImage gridtex;
  bool usegrid = false;

  std::vector<Model> models;
  for (int m = 1; m < argc; m++) {
      const std::string arg = argv[m];
      if (arg == "--white") white = true;
      else if (arg == "--wire") wireframe = true;
      else if (arg == "--normals") normals = true;
      else if (arg == "--grid") usegrid = gridtex.read_tga_file("obj/grid.tga");
      else models.emplace_back(arg);
  }

  const vec3 light = normalized(vec3{1,1,1});

  for (int f=0; f<frames; f++) {
      double a = 2*M_PI * f/frames;

      TGAImage framebuffer(width, height, TGAImage::RGB);
      init_zbuffer(width, height);

      for (const Model& model: models) {
          PhongShader shader(light, model);
          shader.angle = a;
          shader.white = white;
          shader.wireframe = wireframe;
          shader.grid = usegrid ? &gridtex : nullptr;
          for (int face=0; face<model.nfaces(); face++) {
              vec4 clip[3];
              for (int i: {0,1,2}) {
                  vec4 v = rot(model.vert(face, i), a);
                  vec4 vv = ModelView * v;
                  shader.tri[i] = vv;
                  shader.uvs[i] = model.uv(face, i);
                  clip[i] = Perspective * vv;

                  vec4 n = rot(model.normal(face, i), a);
                  shader.nrm[i] = normalized(ModelView * n);
              }
              rasterize(clip, shader, framebuffer);
          }
      }

      if (normals) {
          for (const Model& model: models)
              for (int face=0; face<model.nfaces(); face++)
                  for (int i: {0,1,2}) {
                      vec4 p = ModelView * rot(model.vert(face, i), a);
                      vec4 n = normalized(ModelView * rot(model.normal(face, i), a));
                      line(p, p + n*.05, TGAColor{60, 60, 255, 255}, framebuffer); // BGRA: red
                  }
      }

    char name[64];
    std::snprintf(name, sizeof name, "frames/frame_%03d.tga", f);
    framebuffer.write_tga_file(name);
  }
  return 0;
}
