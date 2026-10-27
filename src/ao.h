#pragma once
#include "geometry.h"
#include "my_model.h"
#include <cmath>

struct AO {
    float epsilon = 1.0f;
    int samples = 20;

    float compute(const Model& model, int face, int idx) const {
        vec3 pos = model.vert(face, idx).xyz();
        vec3 normal = model.normal(face, idx).xyz();

        int counter = 0;
        for (int f = 0; f < model.nfaces(); f++) {
            for (int i=0; i < 3; i++) {
                if (f == face && i == idx) continue;
                vec3 to_other = model.vert(f,i).xyz() - pos;

                if (norm(to_other) > epsilon) continue;
                if (to_other * normal <= 0) continue;
                counter++;
            }
        }
        float metric = float(counter) / samples;
        if (metric > 1.0f) metric = 1.0f;
        return 1.0f - metric;
    }
};
