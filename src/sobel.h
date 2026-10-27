#pragma once
#include <cmath>
#include "pipeline.h"
#include "tgaimage.h"

inline void draw_sobel_edges(TGAImage& framebuffer, const double threshold = 0.05, const TGAColor line = {0, 0, 0, 255}) {
    const int w = framebuffer.width(), h = framebuffer.height();
    auto z = [&](int x, int y) { return zbuffer[x + y*w]; };

    std::vector<int> edge;
    for (int y = 1; y < h-1; y++) {
        for (int x = 1; x < w-1; x++) {
            double gx = -z(x-1,y-1) - 2*z(x-1,y) - z(x-1,y+1)
                        +z(x+1,y-1) + 2*z(x+1,y) + z(x+1,y+1);
            double gy = -z(x-1,y-1) - 2*z(x,y-1) - z(x+1,y-1)
                        +z(x-1,y+1) + 2*z(x,y+1) + z(x+1,y+1);

            if (std::sqrt(gx*gx + gy*gy) > threshold) edge.push_back(x + y*w);
        }
    }
    for (int i : edge) framebuffer.set(i % w, i / w, line);
}
