#pragma once
#include "geometry.h"
#include "my_model.h"
#include <cmath>
#include <random>
#include <algorithm>
#include <vector>

struct AO {
    const double ao_radius = .1; // length of the arrows
    const int samples = 128; // number of arrows
    const double bias = .01; // arrow starts above the surface

    float compute(const Model& model, int face, int idx) const {
        const vec3 pos = model.vert(face, idx).xyz();
        const vec3 normal = normalized(model.normal(face, idx).xyz());
        const vec3 origin = pos + normal * bias; // disable intersaction with initiall surface

        // pseudo random number generator(initial seed)
        std::mt19937 gen(face * 3 + idx);
        std::uniform_real_distribution<double> rnd(-1., 1.);

        // only triangles within reach of the rays can be hit, collect them once
        std::vector<int> nearby;
        for (int f = 0; f < model.nfaces(); f++)
            if (f != face && is_near(model, f, pos)) nearby.push_back(f);

        // skip vectors: || || > 1. or || || approx 0
        int blocked = 0;
        for (int s = 0; s < samples; s++) {
            vec3 dir;
            do {
                dir = {rnd(gen), rnd(gen), rnd(gen)};
            } while (dir * dir > 1. || dir * dir < 1e-6);

            dir = normalized(dir);
            if (dir * normal < 0) dir = dir * -1.; // opposite direction (only need direction --> norm

            if (hits_anything(model, origin, dir, nearby)) blocked++;
        }
        return 1.0f - float(blocked) / samples; // closer to 1 more lighter
    }

private:
    // bounding box of the triangle vs sphere around pos
    bool is_near(const Model& model, int f, const vec3& pos) const {
        const double reach = ao_radius + bias;
        double d2 = 0;
        for (int k = 0; k < 3; k++) {
            double lo = 1e30, hi = -1e30;
            for (int i = 0; i < 3; i++) {
                double c = model.vert(f, i).xyz()[k];
                lo = std::min(lo, c); hi = std::max(hi, c);
            }
            double x = std::clamp(pos[k], lo, hi) - pos[k];
            d2 += x * x;
        }
        return d2 <= reach * reach;
    }

    bool hits_anything(const Model& model, const vec3& o, const vec3& d, const std::vector<int>& faces) const {
        for (int f : faces) {

            vec3 a = model.vert(f, 0).xyz();
            vec3 e1 = model.vert(f, 1).xyz() - a;
            vec3 e2 = model.vert(f, 2).xyz() - a;

            vec3 p = cross(d, e2); // generate vecor perpdendicular to d and e2
            double det = e1 * p; // calculate volume of parallelepiped with basis (e2, p, e1)
            if (std::abs(det) < 1e-9) continue; // p almost parallel to triangle

            vec3 s = o - a;
            double u = (s * p) / det;
            if (u < 0. || u > 1.) continue;

            vec3 q = cross(s, e1);
            double v = (d * q) / det;
            if (v < 0. || u + v > 1.) continue;

            double t = (e2 * q) / det;
            if (t > 1e-6 && t < ao_radius) return true;
        }
        return false;
    }
};
