#include <SDL2/SDL.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include "my_model.h"
#include "geometry.h"
#include "pipeline.h"
#include "phong_shader.h"

// Interactive viewer: drag with left mouse button to orbit, wheel to zoom, W wireframe, T texture on/off, N normals, Esc to quit.
int main(int argc, char** argv) {
  if (argc < 2) {
      std::cerr << "Usage: " << argv[0] << " obj/model.obj ..." << std::endl;
      return 1;
  }

  constexpr int width = 800;
  constexpr int height = 800;
  constexpr vec3 center{0, 0, 0};
  constexpr vec3 up{0, 1, 0};
  const vec3 light = normalized(vec3{1,1,1});

  std::vector<Model> models;
  for (int m = 1; m < argc; m++)
      models.emplace_back(argv[m]);

  // camera on a sphere around center; starts at the same eye{-1,0,2} as my_renderer
  double yaw = std::atan2(-1., 2.);
  double pitch = 0;
  double dist = std::sqrt(5.);

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
      std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
      return 1;
  }
  SDL_Window* window = SDL_CreateWindow("tinyrenderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
  SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, width, height);
  if (!window || !renderer || !texture) {
      std::cerr << "SDL setup failed: " << SDL_GetError() << std::endl;
      return 1;
  }

  std::vector<std::uint8_t> pixels(width*height*3); // array of RGB bytes
  bool running = true, dragging = false, dirty = true;
  bool white = false, wireframe = false, normals = false;

  auto handle = [&](const SDL_Event& e) {
      switch (e.type) {
          case SDL_QUIT: running = false; break;
          case SDL_KEYDOWN:
              if (e.key.keysym.sym == SDLK_ESCAPE) running = false;
              if (e.key.keysym.sym == SDLK_w) { wireframe = !wireframe; dirty = true; } // press w wireframe mode dirty=true new frame
              if (e.key.keysym.sym == SDLK_t) { white = !white; dirty = true; }
              if (e.key.keysym.sym == SDLK_n) { normals = !normals; dirty = true; }
              break;
          case SDL_MOUSEBUTTONDOWN: if (e.button.button == SDL_BUTTON_LEFT) dragging = true; break;
          case SDL_MOUSEBUTTONUP:   if (e.button.button == SDL_BUTTON_LEFT) dragging = false; break;
          case SDL_MOUSEMOTION:
              if (dragging) {
                  yaw   -= e.motion.xrel * 0.01;
                  pitch += e.motion.yrel * 0.01;
                  pitch = std::clamp(pitch, -1.5, 1.5); // camera position changed
                  dirty = true;
              }
              break;
      }
  };

  while (running) {
      SDL_Event e;
      if (!dirty && SDL_WaitEvent(&e)) handle(e);
      while (SDL_PollEvent(&e)) handle(e);
      if (!running || !dirty) continue;
      dirty = false;

      auto t0 = std::chrono::steady_clock::now();

      const vec3 eye = center + vec3{std::cos(pitch)*std::sin(yaw), std::sin(pitch), std::cos(pitch)*std::cos(yaw)} * dist;
      lookat(eye, center, up);
      init_perspective(norm(eye-center));
      init_viewport(width/16, height/16, width*7/8, height*7/8);

      TGAImage framebuffer(width, height, TGAImage::RGB);
      init_zbuffer(width, height);
      for (const Model& model: models) {
          PhongShader shader(light, model);
          shader.white = white;
          shader.wireframe = wireframe;
          for (int face=0; face<model.nfaces(); face++) {
              vec4 clip[3];
              for (int i: {0,1,2})
                  clip[i] = shader.vertex(face, i);
              rasterize(clip, shader, framebuffer);
          }
      }
      if (normals) {
          for (const Model& model: models)
              for (int face=0; face<model.nfaces(); face++)
                  for (int i: {0,1,2}) {
                      vec4 p = ModelView * model.vert(face, i);
                      vec4 n = normalized(ModelView * model.normal(face, i));
                      line(p, p + n*.05, TGAColor{60, 60, 255, 255}, framebuffer); // BGRA: red
                  }
      }

      for (int y=0; y<height; y++)
          for (int x=0; x<width; x++) {
              TGAColor c = framebuffer.get(x, height-1-y);
              std::uint8_t* p = &pixels[(x + y*width)*3];
              p[0] = c[2]; p[1] = c[1]; p[2] = c[0];
          }
      SDL_UpdateTexture(texture, nullptr, pixels.data(), width*3);
      SDL_RenderClear(renderer);
      SDL_RenderCopy(renderer, texture, nullptr, nullptr);
      SDL_RenderPresent(renderer);

      double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
      SDL_SetWindowTitle(window, ("tinyrenderer - " + std::to_string(static_cast<int>(ms)) + " ms").c_str());
  }

  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
