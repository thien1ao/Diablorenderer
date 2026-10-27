#pragma once
#include <algorithm>
#include <cmath>
#include "geometry.h"
#include "pipeline.h"
#include "tgaimage.h"

extern mat<4,4> Perspective, Viewport;

inline void point(const vec4 p, const int size, const TGAColor color, TGAImage& framebuffer) {
    vec4 c = Perspective * p;
    if (c.w <= 0) return;
    vec4 n = c / c.w;
    vec2 s = (Viewport*n).xy();
    int cx = static_cast<int>(std::round(s.x)), cy = static_cast<int>(std::round(s.y));
    for (int x=cx-size/2; x<=cx+size/2; x++)
        for (int y=cy-size/2; y<=cy+size/2; y++) {
            if (x<0 || y<0 || x>=framebuffer.width() || y>=framebuffer.height()) continue;
            if (n.z <= zbuffer[x+y*framebuffer.width()]) continue;
            zbuffer[x+y*framebuffer.width()] = n.z;
            framebuffer.set(x, y, color);
        }
}

inline void line(const vec4 a, const vec4 b, const TGAColor color, TGAImage &framebuffer) {
    vec4 ca = Perspective*a, cb = Perspective*b;
    if (ca.w <= 0 || cb.w <= 0) return;
    vec4 na = ca/ca.w, nb = cb/cb.w;
    vec2 sa = (Viewport*na).xy(), sb = (Viewport*nb).xy();

    int steps = std::max(1, static_cast<int>(std::ceil(std::max(std::abs(sb.x-sa.x), std::abs(sb.y-sa.y)))));
    for (int s=0; s<=steps; s++) {
        double t = s/static_cast<double>(steps);
        int x = static_cast<int>(std::round(sa.x + (sb.x-sa.x)*t));
        int y = static_cast<int>(std::round(sa.y + (sb.y-sa.y)*t));
        if (x<0 || y<0 || x>=framebuffer.width() || y>=framebuffer.height()) continue;
        double z = na.z + (nb.z-na.z)*t;
        if (z < zbuffer[x+y*framebuffer.width()] - .01) continue;
        framebuffer.set(x, y, color);
    }
}
