#pragma once
#include <vector>
#include "geometry.h"
#include "my_model.h"
#include "pipeline.h"

extern mat<4,4> ModelView, Perspective, Viewport;

// depth of the scene as seen from the light
struct ShadowMap {
    int width = 0, height = 0;
    std::vector<double> depth;
    mat<4,4> mvp;      // world -> light clip coords
    mat<4,4> viewport; // light ndc -> shadow map pixels
};

struct DepthShader : IShader {
    virtual std::pair<bool, TGAColor> fragment(const vec3) const { return {false, TGAColor{255, 255, 255, 255}}; }
};

// Renders the depth from the light. Overwrites ModelView/Perspective/Viewport/zbuffer:
// set the camera AFTER calling this.
inline ShadowMap build_shadow_map(const std::vector<Model>& models, const vec3 light, const vec3 center, const vec3 up,
                                  const double angle, const int size) {
    lookat(center + light, center, up);
    Perspective = {{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}}}; // directional light -> orthographic projection
    init_viewport(size/16, size/16, size*7/8, size*7/8);
    init_zbuffer(size, size);

    TGAImage trash(size, size, TGAImage::GRAYSCALE);
    DepthShader shader;
    for (const Model& model : models)
        for (int face=0; face<model.nfaces(); face++) {
            vec4 clip[3];
            for (int i : {0,1,2})
                clip[i] = Perspective * ModelView * rot(model.vert(face, i), angle);
            rasterize(clip, shader, trash);
        }
    return {size, size, zbuffer, Perspective * ModelView, Viewport};
}
