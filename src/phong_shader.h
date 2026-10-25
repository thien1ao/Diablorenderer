#pragma once
#include <algorithm>
#include <cmath>
#include <utility>
#include "geometry.h"
#include "my_model.h"
#include "pipeline.h"

extern mat<4,4> ModelView, Perspective, Viewport;

struct PhongShader : IShader {
    const Model& model;
    mat<4,4> ModelViewIT;
    vec4 tri[3];
    vec4 nrm[3];
    vec2 uvs[3];
    vec4 l;

    double angle = 0;
    bool white = false;
    bool wireframe = false;
    const TGAImage* grid = nullptr;

    PhongShader(const vec3 light, const Model &m) : model(m), ModelViewIT(ModelView.invert_transpose()) {
        l = normalized((ModelView*vec4{light.x, light.y, light.z, 0.}));
    }

    virtual vec4 vertex(const int face, const int vert) {
        uvs[vert] = model.uv(face, vert);
        nrm[vert] = ModelViewIT * model.normal(face, vert);
        vec4 pos = ModelView * model.vert(face, vert);
        tri[vert] = pos;
        return Perspective * pos;
    }

    virtual std::pair<bool, TGAColor> fragment(const vec3 bc) const  {
      mat<2,4> E = { tri[1] - tri[0], tri[2] - tri[0] };
      mat<2,2> U = { uvs[1] - uvs[0], uvs[2] - uvs[0] };
      mat<2,4> T = U.invert() * E;
      // BASIS IN TANGENT SPACE
      mat<4,4> B = {normalized(T[0]), normalized(T[1]), nrm[0]*bc[0] + nrm[1]*bc[1] + nrm[2]*bc[2], {0,0,0,1}};

      vec2 uv = uvs[0]*bc[0] +uvs[1]*bc[1] + uvs[2]*bc[2];
      TGAColor color = white ? TGAColor{255, 255, 255, 255} : sample2D(grid ? *grid : model.diffuse(), uv);
      vec4 n = normalized(B.transpose() * rot(model.normal(uv), angle));
      vec4 r = normalized(n * (n * l) * 2 - l);
      double kd = 0.7;
      double ks = 0.5;
      double ambient = .3;
      double diff = std::max(0., n * l);
      double spec = std::pow(std::max(r.z, 0.), 35);

      double intencity = std::min(1., ambient + kd * diff + ks * spec);
      for (int channel : {0,1,2}) {
          color[channel] = static_cast<uint8_t>(std::min(255., color[channel]*intencity));
      }

      if (wireframe) {
          vec2 scr[3];
          for (int i : {0,1,2}) {
              vec4 clip = Perspective * tri[i];
              scr[i] = (Viewport * (clip / clip.w)).xy();
          }
          double area2 = std::abs((scr[1].x-scr[0].x)*(scr[2].y-scr[0].y) - (scr[2].x-scr[0].x)*(scr[1].y-scr[0].y));
          double dist = 1e9;
          for (int i : {0,1,2})
              dist = std::min(dist, bc[i] * area2 / norm(scr[(i+2)%3] - scr[(i+1)%3]));
          double t = std::clamp(dist - 0.5, 0., 1.);
          constexpr double line = 30.;               // line brightness
          for (int channel : {0,1,2})
              color[channel] = static_cast<uint8_t>(line + (color[channel] - line) * t);
      }
      return {false, color};
    }
};
