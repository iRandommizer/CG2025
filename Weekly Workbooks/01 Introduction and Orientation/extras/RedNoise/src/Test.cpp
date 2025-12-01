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

float interpolate(CanvasPoint from, CanvasPoint to, float value, int dim) {
    if (dim == 0) { // Interpolating X based on Y
        float yRange = to.y - from.y;
        if (fabs(yRange) < 1e-6) return from.x; // Avoid divide by zero
        float pct = (value - from.y) / yRange;
        return from.x + pct * (to.x - from.x);
    } 
    else if (dim == 2) { // Depth
        float yRange = to.y - from.y; 
        if (fabs(yRange) < 1e-6) return from.depth;
        float pct = (value - from.y) / yRange;
        return from.depth + pct * (to.depth - from.depth);
    }
    else if (dim == 3) { // Texture U
        float yRange = to.y - from.y;
        if (fabs(yRange) < 1e-6) return from.texturePoint.u;
        float pct = (value - from.y) / yRange;
        return from.texturePoint.u + pct * (to.texturePoint.u - from.texturePoint.u);
    }
    else if (dim == 4) { // Texture V
        float yRange = to.y - from.y;
        if (fabs(yRange) < 1e-6) return from.texturePoint.v;
        float pct = (value - from.y) / yRange;
        return from.texturePoint.v + pct * (to.texturePoint.v - from.texturePoint.v);
    }
    return 0;
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

Finance
Account

Upgrade
Install
📺
Computer Graphics




Okay let's start this from scratch, I need your help to see if you can achieve my week 3 and week 4 labs. Here are some crucial files

7 attachments
Yes, both Week 3 (Triangles and Textures) and Week 4 (Wireframes and Rasterising) labs can be fully achieved by implementing the specified drawing functions, OBJ parser, projection, and depth buffering in C++ using the provided libs/sdw classes like CanvasPoint, CanvasTriangle, ModelTriangle, Colour, and TextureMap.​

Week 3: Key Implementations
Implement line drawing via interpolation between points, calling SDL_SetRenderDrawColor and SDL_RenderDrawPoint in a loop from min to max row, incrementing x position parametrically.​
For stroked triangles, call the line function three times on triangle edges.​
Filled triangles require sorting vertices by y-coordinate, then rasterizing horizontal scanlines with interpolated x positions, rounding appropriately (e.g., floor for start, ceil for end), filling pixels with the color, followed by a white stroked overlay; make interactive with 'f' key for random triangles.​
Textured triangles extend filling by interpolating TexturePoint uvs alongside x, sampling tex.pixels[int(u*width + v)] with proper bilinear-like indexing (no filtering needed).​
Verify with specific triangle (160,10)-(300,230)-(10,150) mapped to texture points (195,5)-(395,380)-(65,330), checking 7 stones on bottom edge.​

Week 4: Key Implementations
Parse cornell-box.obj and .mtl: store vertices in vector<vec3>, materials in map<string, Colour> via newmtl/Kd, create ModelTriangles on f lines using 1-based indices, apply scale 0.35, set colour on usemtl.​
Project vertices with CanvasPoint projectVertexOntoCanvasPoint(vec3 camPos, float focal, vec3 pos): transpose to camera space (pos-camPos), compute u = focal * x_cam / z_cam + width/2, v = -focal * y_cam / z_cam + height/2, using cam(0,0,4), focal=2.​
Render pointcloud by projecting all vertices, scaling by 160, drawing white pixels.​
Wireframe: project ModelTriangle verts to CanvasTriangle, draw stroked white.​
Rasterise: project to CanvasTriangle with colours, call filled triangle.​

Depth Buffer for Occlusion
Allocate float** depthBuffer = new float*[height]; for(int i=0; i<height; i++) depthBuffer[i] = new floatwidth; // init 0.​
In filled rasterizer, interpolate 1/z (CanvasPoint::depth) across triangle like x/y, test/update pixel if new 1/z > depthBuffer[row][col].​
Verify by moving cam to (0,-1,4), ensuring minimal box peek through floor.​

okay create a master plan of all the code you'd have to create to make this possible and then check with me if it is all good
Master plan for completing Week 3 and Week 4 labs: implement all required functions in a single main.cpp (or Renderer class), reusing Week 3 rasterizers for Week 4 rendering, with OBJ/MTL loaders, projection, and depth buffer.​

Core Structure
Headers/Includes: #include <DrawingWindow.h>, CanvasPoint.h/Colour.h/CanvasTriangle.h/ModelTriangle.h/TextureMap.h/Utils.h, GLM vec3, <vector><map><fstream><sstream>, SDL2.​

Globals: int width=640, height=480; std::vector<std::vector<float>> depthBuffer(height, std::vector<float>(width, 0.0f)); reset per frame.​​

Main Loop: Init DrawingWindow window(width,height); load palette MTL, model OBJ; event loop: handle keys ('u' stroked rand tri, 'f' filled rand, 'i' textured?, 'r' rasterise), clear window/depth=0, call render func, window.renderFrame();.​

Week 3 Functions (2D Rasterizer)
void drawLine(DrawingWindow& w, CanvasPoint from, CanvasPoint to, Colour c): Interp x/y along rows (min-max y), step dx = (to.x-from.x)/(to.y-from.y), SDL_SetRenderDrawColor, loop rows w.setPixelColour(round(x),row, pack(c));.​

void drawStrokedTriangle(DrawingWindow& w, CanvasTriangle t, Colour c): drawLine on t.v0-v1, v1-v2, v2-v0.​

void drawFilledTriangle(DrawingWindow& w, CanvasTriangle t, Colour c[, depthBuffer]): Sort verts a<b<c.y (swap), split mid d at b.y interp x/depth/texcoord from a-c; drawFlatBottom(w, a,b,d,c) drawFlatTop(w, b,c,d,c). No stroke needed in Week4.​​

Helpers: float interOnEdge(CanvasPoint a, CanvasPoint b, float yOrX, int dim(0=x,1=y,2=depth)): % handle range==0 return from. FlatBottom: loop y b.y to c.y, x_left inter(b,d,y,0), x_right inter(b,c,y,0) [+depthL/R]; inner x loop inter left-right, pack colour if depth>buffer update.​​

void drawTexturedTriangle(DrawingWindow& w, CanvasTriangle t, TextureMap tex): Same as filled but inter u/v, pixel idx = (u*tex.width + v) % tex.pixels.size(), unpack RGB.​

Week 4 Additions (3D Pipeline)
std::map<std::string, Colour> loadMTL(std::string path): ifstream, getline/split, newmtl name, Kd r g b Colour(name, r255,g255,b*255).​

std::vector<ModelTriangle> loadOBJ(std::string path, float scale=0.35, std::map<std::string, Colour>& palette): vec<vec3> verts; mtllib mtl loadMTL(mtl); loop lines v x y z verts.push_back({xscale,yscale,z*scale}); f i/j/k ModelTriangle({verts[i-1],verts[j-1],verts[k-1]}, white); usemtl m tri.colour = palette[m];.​

CanvasPoint projectVert(glm::vec3 cam, float focal, glm::vec3 v): vec3 rel = v-cam; if(rel.z<=0) invalid; return {focalrel.x/rel.z + width/2, -focalrel.y/rel.z + height/2, 1.f/rel.z};.​

void drawPointCloud(vec<ModelTriangle> model, vec3 cam, float focal, float scale, DrawingWindow& w): foreach tri foreach vert proj=project(...); w.setPixelColour(round(proj.x*scale),round(proj.y*scale), white);.​

void drawWireframe(...): foreach tri CanvasTriangle proj(project v0, v1, v2); drawStrokedTriangle(w, proj, white);.​

void drawRasterised(vec<ModelTriangle> model, ...): foreach tri CanvasTriangle proj(.. set proj.colour=tri.colour); drawFilledTriangle(w, proj, tri.colour);.​

Depth Version: Pass depthBuffer to drawFilledTriangle/flat helpers, set CanvasPoint.depth=proj.depth on verts, inter/test max 1/z > buffer[x][y] then draw/update; init buffer 0/frame; verify cam(0,-1,4) no peek.​​

Does this master plan match your code/setup and cover all tasks correctly? Confirm before I provide full code snippets.​

just for context these are premade .h and .cpp alr: So doublecheck if you need to update your masterplan

Attachment
image.jpg
No updates needed to the master plan—it fully aligns with the premade libs/sdw files (CanvasPoint/Triangle, Colour, DrawingWindow, ModelTriangle, TextureMap/Point, Utils for split), which provide all classes and DrawingWindow methods like setPixelColour/pollForInputEvents/handleEvent; implement only custom drawing/load/project functions in your main src file (e.g., RedNoise.cpp).​

The plan reuses exact classes (e.g., ModelTriangle verts/colour, CanvasPoint depth/texturePoint for interp), handles depth buffer externally (2D vector passed to rasterizers), and integrates event keys/main loop correctly per READMEs and your prior code style (interOnEdge, flat top/bottom).​

Ready to generate full code snippets or full main.cpp—confirm "good" or specify tweaks (e.g., exact func sigs from your code, textured 'i' key, imageScale handling)?

please complete this function:
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
        for
cpp
void drawTexturedTriangle(DrawingWindow& w, CanvasTriangle t, TextureMap& tex) {
    sortVertices(t);
    CanvasPoint mid;
    mid.x = interpolate(t.vertices[0], t.vertices[2], t.vertices[1].y
pls complete your code this is all I have:
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
        for
Here is the completion of your drawTexturedTriangle function, followed by the necessary handleEvent and main functions to make the Week 3 lab fully interactive and functional.

I have also fixed a small logic gap in the interpolation loop (clamping x values) and added the specific "Visual Verification" task for the 't' key as required by the workbook.

1. Completion of drawTexturedTriangle
Copy and append this immediately after where your code cuts off:

cpp
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
2. Event Handling & Main Function
Add this code at the bottom of your file to tie everything together. This handles the random generation for tasks 3 & 4 and the specific texture verification for Task 6.

cpp
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
Important Fix for your interpolate function
I noticed your interpolate function has a logic bug regarding the range. When interpolating X (dim 0) based on Y, the range should be the Y-distance (to.y - from.y), not the X-distance.

Please update your interpolate function at the top of the file to this safer version:

cpp
float interpolate(CanvasPoint from, CanvasPoint to, float value, int dim) {
    if (dim == 0) { // Interpolating X based on Y
        float yRange = to.y - from.y;
        if (fabs(yRange) < 1e-6) return from.x; // Avoid divide by zero
        float pct = (value - from.y) / yRange;
        return from.x + pct * (to.x - from.x);
    } 
    else if (dim == 2) { // Depth
        float yRange = to.y - from.y; 
        if (fabs(yRange) < 1e-6) return from.depth;
        float pct = (value - from.y) / yRange;
        return from.depth + pct * (to.depth - from.depth);
    }
    else if (dim == 3) { // Texture U
        float yRange = to.y - from.y;
        if (fabs(yRange) < 1e-6) return from.texturePoint.u;
        float pct = (value - from.y) / yRange;
        return from.texturePoint.u + pct * (to.texturePoint.u - from.texturePoint.u);
    }
    else if (dim == 4) { // Texture V
        float yRange = to.y - from.y;
        if (fabs(yRange) < 1e-6) return from.texturePoint.v;
        float pct = (value - from.y) / yRange;
        return from.texturePoint.v + pct * (to.texturePoint.v - from.texturePoint.v);
    }
    return 0;
}
okay give me a code to test all thes functions
Here is a complete, interactive test harness. Replace your main and handleEvent functions with this code.

It maps specific keys to the exact test cases required by the Lab Workbook so you can verify them one by one.

Test Harness Code
cpp
void handleEvent(SDL_Event event, DrawingWindow& window, TextureMap& texture) {
    if (event.type == SDL_KEYDOWN) {
        
        // --- CLEAR SCREEN (Key: C) ---
        if (event.key.keysym.sym == SDLK_c) {
            window.clearPixels();
            std::cout << "Cleared screen." << std::endl;
        }

        // --- TASK 2: LINE DRAWING TEST (Key: L) ---
        else if (event.key.keysym.sym == SDLK_l) {
            window.clearPixels();
            Colour white(255, 255, 255);
            Colour red(255, 0, 0);
            
            // Test cases from Workbook Task 2
            // 1. Top-left to Centre
            drawLine(window, CanvasPoint(0, 0), CanvasPoint(WIDTH/2, HEIGHT/2), white);
            
            // 2. Top-right to Centre
            drawLine(window, CanvasPoint(WIDTH-1, 0), CanvasPoint(WIDTH/2, HEIGHT/2), white);
            
            // 3. Vertical line down middle
            drawLine(window, CanvasPoint(WIDTH/2, 0), CanvasPoint(WIDTH/2, HEIGHT-1), red);
            
            // 4. Horizontal line (1/3rd width)
            float third = WIDTH / 3.0f;
            drawLine(window, CanvasPoint(third, HEIGHT/2), CanvasPoint(WIDTH - third, HEIGHT/2), red);
            
            std::cout << "drawn Lines test." << std::endl;
        }

        // --- TASK 3: STROKED TRIANGLE (Key: U) ---
        else if (event.key.keysym.sym == SDLK_u) {
            // Draws a random unfilled triangle
            CanvasTriangle t;
            t.vertices[0] = CanvasPoint(rand() % WIDTH, rand() % HEIGHT);
            t.vertices[1] = CanvasPoint(rand() % WIDTH, rand() % HEIGHT);
            t.vertices[2] = CanvasPoint(rand() % WIDTH, rand() % HEIGHT);
            
            Colour randomColour(rand() % 256, rand() % 256, rand() % 256);
            drawStrokedTriangle(window, t, randomColour);
            std::cout << "Drawn random stroked triangle." << std::endl;
        }

        // --- TASK 4: FILLED TRIANGLE (Key: F) ---
        else if (event.key.keysym.sym == SDLK_f) {
            // Draws a random filled triangle with white outline
            CanvasTriangle t;
            t.vertices[0] = CanvasPoint(rand() % WIDTH, rand() % HEIGHT);
            t.vertices[1] = CanvasPoint(rand() % WIDTH, rand() % HEIGHT);
            t.vertices[2] = CanvasPoint(rand() % WIDTH, rand() % HEIGHT);
            
            Colour randomColour(rand() % 256, rand() % 256, rand() % 256);
            drawFilledTriangle(window, t, randomColour);
            std::cout << "Drawn random filled triangle." << std::endl;
        }

        // --- TASK 5 & 6: TEXTURE MAPPING VERIFICATION (Key: T) ---
        else if (event.key.keysym.sym == SDLK_t) {
            window.clearPixels();
            
            // Hardcoded verification triangle from Workbook Task 6
            CanvasTriangle t;
            t.vertices[0] = CanvasPoint(160, 10);
            t.vertices[1] = CanvasPoint(300, 230);
            t.vertices[2] = CanvasPoint(10, 150);

            // Texture coordinates (Must match workbook exact values)
            t.vertices[0].texturePoint = TexturePoint(195.0f / texture.width, 5.0f / texture.height);
            t.vertices[1].texturePoint = TexturePoint(395.0f / texture.width, 380.0f / texture.height);
            t.vertices[2].texturePoint = TexturePoint(65.0f / texture.width, 330.0f / texture.height);

            drawTexturedTriangle(window, t, texture);
            
            // Draw outline to verify vertices are correct
            drawStrokedTriangle(window, t, Colour(255, 255, 255));
            
            std::cout << "Drawn Texture Verification Triangle." << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
    SDL_Event event;

    // Load texture.ppm (Make sure this file exists in your project folder!)
    TextureMap texture("texture.ppm"); 
    std::cout << "Texture loaded: " << texture.width << "x" << texture.height << std::endl;
    std::cout << "Controls:\n L: Line Test\n U: Random Stroked Triangle\n F: Random Filled Triangle\n T: Texture Verification\n C: Clear" << std::endl;

    while (true) {
        if (window.pollForInputEvents(event)) handleEvent(event, window, texture);
        window.renderFrame();
    }
    return 0;
}





