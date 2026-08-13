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

union Colorf {
    struct {
        f32 r;
        f32 g;
        f32 b;
        f32 a;
    };
    v4f vec;
};

inline Colorf colorf(f32 r, f32 g, f32 b, f32 a) {
    Colorf result;
    result.r = r;
    result.g = g;
    result.b = b;
    result.a = a;

    return result;
}

inline Colorf colorf(f32 val) {
    Colorf result;
    result.r = val;
    result.g = val;
    result.b = val;
    result.a = val;

    return result;
}

inline Colorf color8_to_colorf(Color8 color) {
    Colorf result;
    result.r = (f32) color.r / 255.0f;
    result.g = (f32) color.g / 255.0f;
    result.b = (f32) color.b / 255.0f;
    result.a = (f32) color.a / 255.0f;

    return result;
}

inline Colorf operator*(Colorf color, r32 n) {
    Colorf result;
    result.r = color.r * n;
    result.g = color.g * n;
    result.b = color.b * n;
    result.a = color.a * n;

    return result;
}

inline Colorf& operator*=(Colorf& color, r32 n) {
    color.r *= n;
    color.g *= n;
    color.b *= n;
    color.a *= n;

    return color;
}

inline Colorf operator/(Colorf color, r32 n) {
    Colorf result;
    result.r = color.r / n;
    result.g = color.g / n;
    result.b = color.b / n;
    result.a = color.a / n;

    return result;
}

inline Colorf& operator/=(Colorf& color, r32 n) {
    color.r /= n;
    color.g /= n;
    color.b /= n;
    color.a /= n;

    return color;
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
    v3f v0;
    v3f v1;
    v3f v2;
    Color8 c0;
    Color8 c1;
    Color8 c2;
};

struct Vertex4d {
    v4f pos;
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
    result.v0.pos = v3f_to_v4f(t.v0);
    result.v1.pos = v3f_to_v4f(t.v1);
    result.v2.pos = v3f_to_v4f(t.v2);
    result.v0.color = t.c0;
    result.v1.color = t.c1;
    result.v2.color = t.c2;

    return result;
}

struct Render_Entity {
    v3f pos;
    v3f scale;
    v3f rotation;
    Triangle *triangles;
    u32 triangle_count;
};

Render_Entity create_empty_render_entity();
Render_Entity create_triangle_render_entity(Triangle triangle);
void draw_render_entity(Frame_Buffer *framebuffer, Render_Entity e);

Triangle create_triangle(v3f v0, v3f v1, v3f v2, Color8 c0, Color8 c1, Color8 c2);
Clip_Triangle create_clip_triangle_mvp(Triangle triangle, m4f mvp);
void draw_triangle(Frame_Buffer *framebuffer, Clip_Triangle clip_triangle);

struct Raster_Vertex {
    union {
        v2f pos;
        v2f xy;
        struct {
            f32 x, y;
        };
    };
    Colorf color;
    f32 depth;
    f32 one_over_w;
};

Raster_Vertex ndc_to_raster(v4f ndc_vertex, f32 w, Colorf color, f32 width, f32 height);
void order_vertices_clockwise(Raster_Vertex *v0, Raster_Vertex *v1, Raster_Vertex *v2);

void put_pixel(Frame_Buffer *framebuffer, s32 x, s32 y, u32 color);

void m4f_print(m4f mat);

b32 triangle_in_view_frustum(Clip_Triangle triangle);
void clip_and_draw_triangle(Frame_Buffer *frame_buffer, Clip_Triangle triangle);
void clip_and_draw_one_out(Frame_Buffer *frame_buffer, Vertex4d out, Vertex4d in1, Vertex4d in2);
void clip_and_draw_two_out(Frame_Buffer *frame_buffer, Vertex4d out1, Vertex4d out2, Vertex4d in);