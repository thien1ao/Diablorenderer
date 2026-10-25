#include "pipeline.h"
#include <algorithm>
#include <cmath>

mat<4,4> ModelView, Viewport, Perspective;
std::vector<double> zbuffer;

vec4 rot(const vec4 v, const double a) {
    return {std::cos(a)*v.x + std::sin(a)*v.z, v.y, -std::sin(a)*v.x + std::cos(a)*v.z, v.w};
}

void lookat(const vec3 eye, const vec3 center, const vec3 up) {
     vec3 n = normalized(eye-center);
    vec3 l = normalized(cross(up,n));
    vec3 m = normalized(cross(n, l));
    ModelView = mat<4,4>{{{l.x,l.y,l.z,0}, {m.x,m.y,m.z,0}, {n.x,n.y,n.z,0}, {0,0,0,1}}} *
                mat<4,4>{{{1,0,0,-center.x}, {0,1,0,-center.y}, {0,0,1,-center.z}, {0,0,0,1}}};
}

void init_perspective(const double f) {
    Perspective = {{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0, -1/f,1}}};
}

void init_viewport(const int x, const int y, const int w, const int h) {
    Viewport = {{{w/2., 0, 0, x+w/2.}, {0, h/2., 0, y+h/2.}, {0,0,1,0}, {0,0,0,1}}};
}

void init_zbuffer(const int w, const int h) {
    zbuffer = std::vector<double>(w*h, -1000.);
}

void rasterize(const Triangle &clip, const IShader &shader, TGAImage &framebuffer) {
    vec4 ndc[3]    = { clip[0]/clip[0].w, clip[1]/clip[1].w, clip[2]/clip[2].w };
    vec2 screen[3] = { (Viewport*ndc[0]).xy(), (Viewport*ndc[1]).xy(), (Viewport*ndc[2]).xy() };

    mat<3,3> ABC = {{ {screen[0].x, screen[0].y, 1.}, {screen[1].x, screen[1].y, 1.}, {screen[2].x, screen[2].y, 1.} }};
    if (ABC.det()<1) return;

    auto [bbminx,bbmaxx] = std::minmax({screen[0].x, screen[1].x, screen[2].x});
    auto [bbminy,bbmaxy] = std::minmax({screen[0].y, screen[1].y, screen[2].y});
    const mat<3,3> ABCit = ABC.invert_transpose(); // same for every pixel of the triangle
#pragma omp parallel for
    for (int x=std::max<int>(bbminx, 0); x<=std::min<int>(bbmaxx, framebuffer.width()-1); x++) {
        for (int y=std::max<int>(bbminy, 0); y<=std::min<int>(bbmaxy, framebuffer.height()-1); y++) {
            vec3 bc = ABCit * vec3{static_cast<double>(x), static_cast<double>(y), 1.};
            if (bc.x<0 || bc.y<0 || bc.z<0) continue;
            double z = bc * vec3{ ndc[0].z, ndc[1].z, ndc[2].z };
            if (z <= zbuffer[x+y*framebuffer.width()]) continue;
            auto [discard, color] = shader.fragment(bc);
            if (discard) continue;
            zbuffer[x+y*framebuffer.width()] = z;
            framebuffer.set(x, y, color);
        }
    }
}

void point(const vec4 p, const int size, const TGAColor color, TGAImage& framebuffer) {
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

void line(const vec4 a, const vec4 b, const TGAColor color, TGAImage &framebuffer) {
    vec4 ca = Perspective*a, cb = Perspective*b;
    if (ca.w <= 0 || cb.w <= 0) return; // behind the camera
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
