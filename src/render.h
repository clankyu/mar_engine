#pragma once

#include "engine.h"
#include "engine_math.h"

#define RGBA_WHITE 0xFFFFFFFF
#define RGBA_BLACK 0x000000FF
#define RGBA_RED 0xFF0000FF
#define RGBA_GREEN 0x00FF00FF
#define RGBA_BLUE 0x0000FFFF

struct Color8 {
    union {
        u32 val;

        // this order is important for rgba due to little endianess
        struct {
            u8 a;
            u8 b;
            u8 g;
            u8 r;
        };
    };
};

inline Color8 color8(u32 val) {
    Color8 result = {};
    result.val = val;

    return result;
}

inline Color8 color8(u8 r, u8 g, u8 b, u8 a) {
    Color8 result = {};
    result.r = r;
    result.g = g;
    result.b = b;
    result.a = a;

    return result;
}


inline Color8 operator+(Color8 a, Color8 b) {
    Color8 result;
    result.r = a.r + b.r;
    result.g = a.g + b.g;
    result.b = a.b + b.b;
    result.a = a.a + b.a;

    return result;
}

inline Color8& operator+=(Color8& a, Color8 b) {
    a = a + b;
    return a;
}

inline Color8 operator+(Color8 a, r32 b) {
    Color8 result;
    result.r = a.r + b;
    result.g = a.g + b;
    result.b = a.b + b;
    result.a = a.a + b;

    return result;
}

inline Color8 operator+(r32 b, Color8 a) {
    Color8 result = a + b;
    return result;
}

inline Color8 operator-(Color8 a, Color8 b) {
    Color8 result;
    result.r = a.r - b.r;
    result.g = a.g - b.g;
    result.b = a.b - b.b;
    result.a = a.a - b.a;

    return result;
}

inline Color8& operator-=(Color8& a, Color8 b) {
    a = a - b;
    return a;
}

inline Color8 operator-(Color8 a) {
    Color8 result;
    result.r = -a.r;
    result.g = -a.g;
    result.b = -a.b;
    result.a = -a.a;
    
    return result;
}

inline Color8 operator-(Color8 a, r32 b) {
    Color8 result;
    result.r = a.r - b;
    result.g = a.g - b;
    result.b = a.b - b;
    result.a = a.a - b;
    
    return result;
}

// todo: i think some of these are straight up wrong lol
inline Color8 operator*(Color8 color, r32 n) {
    Color8 result = color;
    result.r *= n;
    result.g *= n;
    result.b *= n;
    result.a *= n;

    return result;
}

inline Color8 operator/(Color8 color, r32 n) {
    Color8 result = color;
    result.r /= n;
    result.g /= n;
    result.b /= n;
    result.a /= n;

    return result;
}

inline Color8 color8_lerp(Color8 a, Color8 b, f32 alpha) {
    Color8 result;
    result = a + (b - a) * alpha;
    
    return result;
}

typedef V4 Colorf;

inline Colorf color8_to_colorf(Color8 color) {
    Colorf result;
    result.r = (f32) color.r / 255.0f;
    result.g = (f32) color.g / 255.0f;
    result.b = (f32) color.b / 255.0f;
    result.a = (f32) color.a / 255.0f;

    return result;
}

inline u32 color_bytes_to_u32(u8 r, u8 g, u8 b, u8 a) {
    u32 result = 0;
    result = (result | a) << 8;
    result = (result | b) << 8;
    result = (result | g) << 8;
    result |= r;
    
    return result;
}

struct Triangle {
    V3 v0;
    V3 v1;
    V3 v2;
    Color8 c0;
    Color8 c1;
    Color8 c2;
};

struct Vertex4d {
    V4 pos;
    Color8 color;
};

Vertex4d vertex4d_lerp(Vertex4d a, Vertex4d b, f32 alpha);

struct Clip_Triangle {
    Vertex4d v0;
    Vertex4d v1;
    Vertex4d v2;
};

inline Clip_Triangle triangle_to_clip(Triangle t) {
    Clip_Triangle result;
    result.v0.pos = v3_to_v4(t.v0);
    result.v1.pos = v3_to_v4(t.v1);
    result.v2.pos = v3_to_v4(t.v2);
    result.v0.color = t.c0;
    result.v1.color = t.c1;
    result.v2.color = t.c2;

    return result;
}

struct Render_Entity {
    V3 pos;
    V3 scale;
    V3 rotation;
    Triangle *triangles;
    u32 triangle_count;
};

Render_Entity create_empty_render_entity();
Render_Entity create_triangle_render_entity(Triangle triangle);
void draw_render_entity(Frame_Buffer *frame_buffer, Render_Entity e);

Triangle create_triangle(V3 v0, V3 v1, V3 v2, Color8 c0, Color8 c1, Color8 c2);
Clip_Triangle create_clip_triangle_mvp(Triangle triangle, M4 mvp);
void draw_triangle(Frame_Buffer *frame_buffer, Clip_Triangle clip_triangle);

struct Raster_Vertex {
    union {
        V2 pos;
        V2 xy;
        struct {
            f32 x, y;
        };
    };
    Colorf color;
    f32 depth;
    f32 one_over_w;
};

Raster_Vertex ndc_to_raster(V4 ndc_vertex, f32 w, Colorf color, f32 width, f32 height);
void order_vertices_clockwise(Raster_Vertex *v0, Raster_Vertex *v1, Raster_Vertex *v2);

void put_pixel(Frame_Buffer *framebuffer, s32 x, s32 y, u32 color);

void m4_print(M4 mat);

b32 triangle_in_view_frustum(Clip_Triangle triangle);
void clip_and_draw_triangle(Frame_Buffer *frame_buffer, Clip_Triangle triangle);
void clip_and_draw_one_out(Frame_Buffer *frame_buffer, Vertex4d out, Vertex4d in1, Vertex4d in2);
void clip_and_draw_two_out(Frame_Buffer *frame_buffer, Vertex4d out1, Vertex4d out2, Vertex4d in);