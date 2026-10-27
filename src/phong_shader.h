#pragma once
#include <algorithm>
#include <cmath>
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

    const std::vector<float>* occlusion = nullptr; // AO from viewer
    float occ[3] = {1.f, 1.f, 1.f};

    double angle = 0;
    bool white = false;

    PhongShader(const vec3 light, const Model &m)
      : model(m), ModelViewIT(ModelView.invert_transpose()) {
        l = normalized((ModelView*vec4{light.x, light.y, light.z, 0.}));
    }

    virtual vec4 vertex(const int face, const int vert) {
        uvs[vert] = model.uv(face, vert);
        nrm[vert] = ModelViewIT * model.normal(face, vert);
        vec4 pos = ModelView * model.vert(face, vert);
        tri[vert] = pos;
        occ[vert] = occlusion ? (*occlusion)[face*3 + vert] : 1.f;
        return Perspective * pos;
    }

    virtual std::pair<bool, TGAColor> fragment(const vec3 bc) const  {
      // NORMAL MAPPING
      mat<2,4> E = { tri[1] - tri[0], tri[2] - tri[0] };
      mat<2,2> U = { uvs[1] - uvs[0], uvs[2] - uvs[0] };
      mat<2,4> T = U.invert() * E;
      mat<4,4> B = {normalized(T[0]), normalized(T[1]), nrm[0]*bc[0] + nrm[1]*bc[1] + nrm[2]*bc[2], {0,0,0,1}};

      vec2 uv = uvs[0]*bc[0] +uvs[1]*bc[1] + uvs[2]*bc[2];
      TGAColor color = white ? TGAColor{255, 255, 255, 255} : sample2D(model.diffuse(), uv);

      double ao = occ[0] * bc[0] + occ[1] * bc[1] + occ[2] * bc[2];

      vec4 n = normalized(B.transpose() * rot(model.normal(uv), angle));
      vec4 r = normalized(n * (n * l) * 2 - l);

      double kd = 0.4;
      double ks = 0.7;
      double ambient = .6;
      double diff = std::max(0., n * l);
      double spec = std::pow(std::max(r.z, 0.),40);

      double intencity = std::min(1., ambient * ao + 0.2 + (kd * diff + ks * spec));
      for (int channel : {0,1,2}) {
          color[channel] = static_cast<uint8_t>(std::min(255., color[channel]*intencity));
      }

      return {false, color};
    }
};
