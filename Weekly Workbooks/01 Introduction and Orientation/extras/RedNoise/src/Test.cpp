#include "DrawingWindow.h"
#include "CanvasPoint.h"
#include "CanvasTriangle.h"
#include "Colour.h"
#include "TextureMap.h"
#include "Utils.h"
#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib> // rand
#include <iostream>

#define WIDTH 640
#define HEIGHT 480
uint32_t packColour(const Colour& c) {
    return (255 << 24) + (c.red << 16) + (c.green << 8) + c.blue;
}

float interpolate(CanvasPoint from, CanvasPoint to, float value, int dim) { // 0:x 1:y? but edge-based y-fixed; ext for depth/tex
    float a = (dim == 0 ? from.x : dim == 2 ? from.depth : dim == 3 ? from.texturePoint.u : from.texturePoint.v);
    float b = (dim == 0 ? to.x : dim == 2 ? to.depth : dim == 3 ? to.texturePoint.u : to.texturePoint.v);
    float range = (dim == 0 ? to.x - from.x : dim == 2 || dim == 4 ? to.y - from.y : to.x - from.x); // adjust per dim
    if (fabs(range) < 1e-6) return a;
    float pct = (value - from.y) / range; // assume y-interp for edges
    return a + pct * (b - a);
}

void drawLine(DrawingWindow& window, CanvasPoint from, CanvasPoint to, Colour colour) {
    float numSteps = std::max(fabsf(to.x - from.x), fabsf(to.y - from.y));
    if (numSteps < 1) numSteps = 1;
    float xStep = (to.x - from.x) / numSteps;
    float yStep = (to.y - from.y) / numSteps;
    uint32_t col = packColour(colour);
    for (float step = 0; step <= numSteps; step += 1) {
        int x = roundf(from.x + xStep * step);
        int y = roundf(from.y + yStep * step);
        if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) window.setPixelColour(x, y, col);
    }
}

void drawStrokedTriangle(DrawingWindow& window, CanvasTriangle triangle, Colour colour) {
    drawLine(window, triangle.vertices[0], triangle.vertices[1], colour);
    drawLine(window, triangle.vertices[1], triangle.vertices[2], colour);
    drawLine(window, triangle.vertices[2], triangle.vertices[0], colour);
}

void sortVertices(CanvasTriangle& t) { // y asc: v0 top, v2 bot
    if (t.vertices[0].y > t.vertices[1].y) std::swap(t.vertices[0], t.vertices[1]);
    if (t.vertices[0].y > t.vertices[2].y) std::swap(t.vertices[0], t.vertices[2]);
    if (t.vertices[1].y > t.vertices[2].y) std::swap(t.vertices[1], t.vertices[2]);
}
void drawFlatBottom(DrawingWindow& w, CanvasPoint botLeft, CanvasPoint botRight, CanvasPoint top, Colour c) { // top.y < botL.y==botR.y
    float step = 1.0f / (botLeft.y - top.y);
    for (float i = 0; i <= (botLeft.y - top.y); ++i) {
        int y = roundf(top.y + i);
        float pct = i * step;
        CanvasPoint left = {interpolate(top, botLeft, y, 0), y};
        CanvasPoint right = {interpolate(top, botRight, y, 0), y};
        for (int x = roundf(left.x); x <= roundf(right.x); ++x) {
            if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) w.setPixelColour(x, y, packColour(c));
        }
    }
}
void drawFlatTop(DrawingWindow& w, CanvasPoint topLeft, CanvasPoint topRight, CanvasPoint bot, Colour c) { // topL.y==topR.y < bot.y
    float step = 1.0f / (bot.y - topLeft.y);
    for (float i = 0; i <= (bot.y - topLeft.y); ++i) {
        int y = roundf(topLeft.y + i);
        float pct = i * step;
        CanvasPoint left = {interpolate(topLeft, bot, y, 0), y};
         CanvasPoint right = {interpolate(topRight, bot, y, 0), y};
        for (int x = std::min(WIDTH-1, std::max(0, roundf(left.x))); x <= std::min(WIDTH-1, std::max(0, roundf(right.x))); ++x) {
            if (y >= 0 && y < HEIGHT) w.setPixelColour(x, y, packColour(c));
        }
    }
}

void drawFilledTriangle(DrawingWindow& w, CanvasTriangle t, Colour c) {
    sortVertices(t);
    CanvasPoint mid;
    mid.x = interpolate(t.vertices[0], t.vertices[2], t.vertices[1].y, 0);
    mid.y = t.vertices[1].y;
    drawFlatBottom(w, t.vertices[0], t.vertices[1], mid, c);
    drawFlatTop(w, t.vertices[1], mid, t.vertices[2], c);
    drawStrokedTriangle(w, t, Colour(255,255,255)); // overlay
}

uint32_t sampleTexture(const TextureMap& tex, float u, float v) {
    int iu = std::max(0, std::min((int)(u * tex.width), tex.width-1));
    int iv = std::max(0, std::min((int)(v * tex.height), tex.height-1));
    return tex.pixels[iu + iv * tex.width];
}

void drawTexturedTriangle(DrawingWindow& w, CanvasTriangle t, TextureMap& tex) {
    sortVertices(t);
    CanvasPoint mid;
    mid.x = interpolate(t.vertices[0], t.vertices[2], t.vertices[1].y, 0);
    mid.texturePoint.u = interpolate(t.vertices[0], t.vertices[2], t.vertices[1].y, 3);
    mid.texturePoint.v = interpolate(t.vertices[0], t.vertices[2], t.vertices[1].y, 4);
    mid.y = t.vertices[1].y;
    // Flat bottom
    for (float i = 0; i <= t.vertices[1].y - t.vertices[0].y; ++i) {
        int y = roundf(t.vertices[0].y + i);
        float l_u = interpolate(t.vertices[0], t.vertices[1], y, 3), l_v = interpolate(t.vertices[0], t.vertices[1], y, 4);
        float r_u = interpolate(t.vertices[0], mid, y, 3), r_v = interpolate(t.vertices[0], mid, y, 4);
        float xl = interpolate(t.vertices[0], t.vertices[1], y, 0);
        float xr = interpolate(t.vertices[0], mid, y, 0);
        for (float px = xl; px <= xr; ++px) {
            int x = roundf(px);
            if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
                float pct_x = (px - xl) / (xr - xl);
                float u = l_u + pct_x * (r_u - l_u);
                float v = l_v + pct_x * (r_v - l_v);
                w.setPixelColour(x, y, sampleTexture(tex, u, v));
            }
        }
    }
    // Flat top similar...
    for (float i = 0; i <= t.vertices[2].y - t.vertices[1].y; ++i) {
        int y = roundf(t.vertices[1].y + i);
        float l_u = interpolate(t.vertices[1], t.vertices[2], y, 3), l_v = interpolate(t.vertices[1], t.vertices[2], y, 4);
        float r_u = interpolate(mid, t.vertices[2], y, 3), r_v = interpolate(mid, t.vertices[2], y, 4);
        float xl = interpolate(t.vertices[1], t.vertices[2], y, 0);
        float xr = interpolate(mid, t.vertices[2], y, 0);
        for (float px = xl; px <= xr; ++px) {
            int x = roundf(px);
            // Bounds check for X to prevent crashing
            if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
                float pct_x = (xr != xl) ? (px - xl) / (xr - xl) : 0.0f;
                float u = l_u + pct_x * (r_u - l_u);
                float v = l_v + pct_x * (r_v - l_v);
                w.setPixelColour(x, y, sampleTexture(tex, u, v));
            }
        }
    }

    // Flat top loop (From v1 to v2)
    for (float i = 0; i <= t.vertices[2].y - t.vertices[1].y; ++i) {
        int y = roundf(t.vertices[1].y + i);
        
        // Interpolate values along the LEFT edge (v1 -> v2) and RIGHT edge (mid -> v2)
        float l_u = interpolate(t.vertices[1], t.vertices[2], y, 3);
        float l_v = interpolate(t.vertices[1], t.vertices[2], y, 4);
        
        float r_u = interpolate(mid, t.vertices[2], y, 3);
        float r_v = interpolate(mid, t.vertices[2], y, 4);
        
        float xl = interpolate(t.vertices[1], t.vertices[2], y, 0);
        float xr = interpolate(mid, t.vertices[2], y, 0);
        
        for (float px = xl; px <= xr; ++px) {
            int x = roundf(px);
            if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
                float pct_x = (xr != xl) ? (px - xl) / (xr - xl) : 0.0f;
                float u = l_u + pct_x * (r_u - l_u);
                float v = l_v + pct_x * (r_v - l_v);
                w.setPixelColour(x, y, sampleTexture(tex, u, v));
            }
        }
    }
}

void handleEvent(SDL_Event event, DrawingWindow& window, TextureMap& texture) {
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_u) {
            // Task 3: Random Stroked Triangle
            CanvasTriangle t;
            for(int i=0; i<3; i++) t.vertices[i] = CanvasPoint(rand()%WIDTH, rand()%HEIGHT);
            Colour c(rand()%256, rand()%256, rand()%256);
            drawStrokedTriangle(window, t, c);
        }
        else if (event.key.keysym.sym == SDLK_f) {
            // Task 4: Random Filled Triangle
            CanvasTriangle t;
            for(int i=0; i<3; i++) t.vertices[i] = CanvasPoint(rand()%WIDTH, rand()%HEIGHT);
            Colour c(rand()%256, rand()%256, rand()%256);
            drawFilledTriangle(window, t, c);
        }
        else if (event.key.keysym.sym == SDLK_t) {
            // Task 6: Texture Verification
            window.clearPixels();
            
            CanvasTriangle t;
            t.vertices[0] = CanvasPoint(160, 10);
            t.vertices[1] = CanvasPoint(300, 230);
            t.vertices[2] = CanvasPoint(10, 150);

            // Map pixel coordinates to UVs (0.0 - 1.0)
            t.vertices[0].texturePoint = TexturePoint(195.0f / texture.width, 5.0f / texture.height);
            t.vertices[1].texturePoint = TexturePoint(395.0f / texture.width, 380.0f / texture.height);
            t.vertices[2].texturePoint = TexturePoint(65.0f / texture.width, 330.0f / texture.height);

            drawTexturedTriangle(window, t, texture);
        }
        else if (event.key.keysym.sym == SDLK_c) {
            window.clearPixels();
        }
    }
}

int main(int argc, char* argv[]) {
    DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
    SDL_Event event;

    // Load the texture file (ensure texture.ppm is in the correct directory)
    TextureMap texture("texture.ppm"); 

    while (true) {
        // We MUST poll for events - otherwise the window will freeze!
        if (window.pollForInputEvents(event)) handleEvent(event, window, texture);
        window.renderFrame();
    }
    return 0;
}


