#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
#include "geometry.h"
#include "my_model.h"
#include "primitives.h"
#include "tgaimage.h"

extern mat<4,4> ModelView;

inline void draw_normals(const std::vector<Model>& models, TGAImage& framebuffer) {
    for (const Model& model : models)
        for (int face=0; face<model.nfaces(); face++)
            for (int i : {0,1,2}) {
                vec4 p = ModelView * model.vert(face, i);
                vec4 n = normalized(ModelView * model.normal(face, i));
                line(p, p + n*.05, TGAColor{60, 60, 255, 255}, framebuffer);
            }
}

inline void draw_wireframe(const std::vector<Model>& models, TGAImage& framebuffer) {
    for (const Model& model : models)
        for (int face=0; face<model.nfaces(); face++)
            for (int i : {0,1,2}) {
                vec4 a = ModelView * model.vert(face, i);
                vec4 b = ModelView * model.vert(face, (i+1)%3);
                line(a, b, TGAColor{0, 255, 0, 255}, framebuffer);
            }
}

inline void draw_points(const std::vector<Model>& models, const double dist, TGAImage& framebuffer) {
    for (const Model& model : models)
        for (int i=0; i<model.nverts(); i++) {
            vec4 p = ModelView * model.vert(i);
            double k = std::clamp((p.z + dist + 1.)/2., .2, 1.);
            auto b = static_cast<std::uint8_t>(255*k);
            point(p, 2, TGAColor{b, b, b, 255}, framebuffer);
        }
}
