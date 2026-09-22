#pragma once

#include "engine.h"
#include "engine_math.h"
#include "memory.h"

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


struct Clip_Triangle {
    V4 v0;
    V4 v1;
    V4 v2;
};

inline Clip_Triangle triangle_to_clip(Triangle t) {
    Clip_Triangle result;
    result.v0 = v3_to_v4(t.v0);
    result.v1 = v3_to_v4(t.v1);
    result.v2 = v3_to_v4(t.v2);

    return result;
}

struct Render_Entity {
    V3 pos;
    V3 scale;
    V3 rotation;
    Triangle *triangles;
    u32 triangle_count;
};

Triangle create_triangle(V3 v0, V3 v1, V3 v2, Color8 c0, Color8 c1, Color8 c2);

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

struct Clipping_Information {
    f32 alpha0_1, alpha1_0;
    f32 alpha1_2, alpha2_1;
    f32 alpha2_0, alpha0_2;
};

Raster_Vertex ndc_to_raster(V4 ndc_vertex, f32 w, Colorf color, f32 width, f32 height);
void order_vertices_and_info_clockwise(V4 *v0, V4 *v1, V4 *v2, V3u *indices, Clipping_Information *clip_info);

void put_pixel(Frame_Buffer *framebuffer, s32 x, s32 y, u32 color);

void m4_print(M4 mat);

b32 triangle_in_view_frustum(Clip_Triangle triangle);

#ifndef Shader_Value_Types
#define Shader_Value_Types \
Define_Shader_Value_Type(f32)\
Define_Shader_Value_Type(V2)\
Define_Shader_Value_Type(V3)\
Define_Shader_Value_Type(V4)\
Define_Shader_Value_Type(M3)\
Define_Shader_Value_Type(M4)
#endif

enum Shader_Value_Type : u32 {
    #define Define_Shader_Value_Type(type) Shader_Value_Type_##type,
    Shader_Value_Types
    #undef Define_Shader_Value_Type
};

inline u64 get_shader_value_type_size(Shader_Value_Type type) {
    u64 result = 0;
    u64 type_sizes[] = {
        #define Define_Shader_Value_Type(type) sizeof(type),
        Shader_Value_Types
        #undef Define_Shader_Value_Type
    };
    
    result = type_sizes[type];
    return result;
}

struct Shader_Value {
    Shader_Value_Type type;
    union {
        f32 f32;
        V2 V2;
        V3 V3;
        V4 V4;
        M3 M3;
        M4 M4;
    };
};

struct Shader_Value_Array {
    u64 count;
    Shader_Value *value;
};

struct Vertex_Output_Array {
    Shader_Value_Type *type_pattern;
};

struct Shader_Uniform {
    Shader_Value_Type type;
    u8 *data;
};

struct Shader_Uniform_Array {
    Shader_Uniform *items;
    u64 count;
    u64 capacity;
};

struct Vertex_Attribute {
    Shader_Value_Type type;
    u64 count;
    u8 *data;
};

struct Vertex_Attribute_Array {
    Vertex_Attribute *items;
    u64 count;
    u64 capacity;
};

struct Shader_Value_Type_Array {
    Shader_Value_Type *items;
    u64 count;
    u64 capacity;
};

// note: only use these macros inside a vertex or fragment shader
#define VERTEX_SHADER_PARAMETERS Shader_Value_Array*, Shader_Value_Array*, u8*, u64
#define VERTEX_SHADER_PARAMETER_TYPES Shader_Value_Array* uniforms, Shader_Value_Array *attributes, u8 *output_ptr, u64 vertex_index
typedef void (*Vertex_Shader)(VERTEX_SHADER_PARAMETER_TYPES);

#define FRAGMENT_SHADER_PARAMETER_TYPES Arena*, Shader_Value_Array*, Shader_Value_Type_Array*, u8*, u8*, u8*, V3, u8*
#define FRAGMMENT_SHADER_PARAMETERS Arena *gpu_arena, Shader_Value_Array *uniforms, Shader_Value_Type_Array *output_pattern, u8 *v0_inputs, u8 *v1_inputs, u8 *v2_inputs, V3 barycentric_coordinates, u8 *output_ptr
typedef void (*Fragment_Shader)(FRAGMENT_SHADER_PARAMETER_TYPES);

struct Shader_Pipeline {
    Arena *gpu_arena;
    Vertex_Attribute_Array attributes;
    Shader_Uniform_Array uniforms;
    
    Shader_Value_Type_Array output_pattern;
    b32 output_pattern_initialized;
    Arena *clipping_arena;
    
    Vertex_Shader vertex_shader;
    Fragment_Shader fragment_shader;
    Frame_Buffer *frame_buffer;
    b32 depth_testing;
};



void add_attribute(Shader_Pipeline *pipeline, u8 *data, Shader_Value_Type type, u64 count);
void add_uniform(Shader_Pipeline *pipeline, u8 *data, Shader_Value_Type type);
u64 get_pattern_offset(Shader_Value_Type_Array *pattern, u64 input_index);
u64 get_shader_value_type_array_size(Shader_Value_Type_Array *arr);
u8 *get_inputs_ptr(Shader_Pipeline *pipeline, u64 vertex_index);
V4 get_pos(Shader_Pipeline *pipeline, u64 vertex_index);
V3u get_triangle_indices(Shader_Pipeline *pipeline, u64 triangle_index);
Clip_Triangle get_triangle(Shader_Pipeline *pipeline, V3u triangle_indices);

Shader_Pipeline create_shader_pipeline(Arena *clipping_arena, Vertex_Shader vertex_shader, Fragment_Shader fragment_shader);

f32 internal_load_attribute_f32 (Shader_Value_Array *attributes, u64 vertex_index, u64 attribute_index);
V2  internal_load_attribute_v2  (Shader_Value_Array *attributes, u64 vertex_index, u64 attribute_index);
V3  internal_load_attribute_v3  (Shader_Value_Array *attributes, u64 vertex_index, u64 attribute_index);
V4  internal_load_attribute_v4  (Shader_Value_Array *attributes, u64 vertex_index, u64 attribute_index);
M3  internal_load_attribute_m3  (Shader_Value_Array *attributes, u64 vertex_index, u64 attribute_index);
M4  internal_load_attribute_m4  (Shader_Value_Array *attributes, u64 vertex_index, u64 attribute_index);

#define load_attribute_f32 (attribute_index) internal_load_attribute_f32 (attributes, vertex_index, attribute_index)
#define load_attribute_v2  (attribute_index) internal_load_attribute_v2  (attributes, vertex_index, attribute_index)
#define load_attribute_v3  (attribute_index) internal_load_attribute_v3  (attributes, vertex_index, attribute_index)
#define load_attribute_v4  (attribute_index) internal_load_attribute_v4  (attributes, vertex_index, attribute_index)
#define load_attribute_m3  (attribute_index) internal_load_attribute_m3  (attributes, vertex_index, attribute_index)
#define load_attribute_m4  (attribute_index) internal_load_attribute_m4  (attributes, vertex_index, attribute_index)

f32 internal_load_uniform_f32 (Shader_Value_Array *uniforms, u64 uniform_index);
V2  internal_load_uniform_v2  (Shader_Value_Array *uniforms, u64 uniform_index);
V3  internal_load_uniform_v3  (Shader_Value_Array *uniforms, u64 uniform_index);
V4  internal_load_uniform_v4  (Shader_Value_Array *uniforms, u64 uniform_index);
M3  internal_load_uniform_m3  (Shader_Value_Array *uniforms, u64 uniform_index);
M4  internal_load_uniform_m4  (Shader_Value_Array *uniforms, u64 uniform_index);

#define load_uniform_f32 internal_load_uniform_f32 (Shader_Value_Array *attributes, u64 uniform_index)
#define load_uniform_v2  internal_load_uniform_v2  (Shader_Value_Array *attributes, u64 uniform_index)
#define load_uniform_v3  internal_load_uniform_v3  (Shader_Value_Array *attributes, u64 uniform_index)
#define load_uniform_v4  internal_load_uniform_v4  (Shader_Value_Array *attributes, u64 uniform_index)
#define load_uniform_m3  internal_load_uniform_m3  (Shader_Value_Array *attributes, u64 uniform_index)
#define load_uniform_m4  internal_load_uniform_m4  (Shader_Value_Array *attributes, u64 uniform_index)

// this is probably very slow, copying byte by byte, but for now ok i guess
inline void output(Shader_Pipeline *pipeline, u8 *value_ptr, Shader_Value_Type type) {
    if (!pipeline->output_pattern_initialized) { da_append(pipeline->output_pattern, type); }
    u64 size = get_shader_value_type_size(type);
    u8 *result = arena_push(pipeline->gpu_arena, size);
    memcpy(result, value_ptr, size);
}

#define output_pos(pos) internal_output_pos(output_ptr, pos)
inline void internal_output_pos(u8 *output_ptr, V4 pos) {
    *(V4*)output_ptr = pos;
}

#define output(value) internal_output(gpu_arena, value);
inline void internal_output(Arena *gpu_arena, f32 value) {
    u64 size = sizeof(f32);
    u8 *result = arena_push(gpu_arena, size);
    *(f32*)result = value;
}
inline void internal_output(Arena *gpu_arena, V2 value) {
    u64 size = sizeof(V2);
    u8 *result = arena_push(gpu_arena, size);
    *(V2*)result = value;
}

inline void internal_output(Arena *gpu_arena, V3 value) {
    u64 size = sizeof(V3);
    u8 *result = arena_push(gpu_arena, size);
    *(V3*)result = value;
}

inline void internal_output(Arena *gpu_arena, V4 value) {
    u64 size = sizeof(V4);
    u8 *result = arena_push(gpu_arena, size);
    *(V4*)result = value;
}

inline void internal_output(Arena *gpu_arena, M3 value) {
    u64 size = sizeof(M3);
    u8 *result = arena_push(gpu_arena, size);
    *(M3*)result = value;
}

inline void internal_output(Arena *gpu_arena, M4 value) {
    u64 size = sizeof(M4);
    u8 *result = arena_push(gpu_arena, size);
    *(M4*)result = value;
}

void clip_and_draw_one_out(Shader_Pipeline *pipeline, u32 out_index, u32 in1_index, u32 in2_index);
void clip_and_draw_two_out(Shader_Pipeline *pipeline, u32 out1_index, u32 out2_index, u32 in_index);

// note: be careful when using these
f32 internal_in_smooth_f32 (Shader_Value_Type_Array *output_pattern, u8 *v0_ptr, u8 *v1_ptr, u8 *v2_ptr, f32 v0_weight, f32 v1_weight, f32 v2_weight, u64 input_index);
V2  internal_in_smooth_v2  (Shader_Value_Type_Array *output_pattern, u8 *v0_ptr, u8 *v1_ptr, u8 *v2_ptr, f32 v0_weight, f32 v1_weight, f32 v2_weight, u64 input_index);
V3  internal_in_smooth_v3  (Shader_Value_Type_Array *output_pattern, u8 *v0_ptr, u8 *v1_ptr, u8 *v2_ptr, f32 v0_weight, f32 v1_weight, f32 v2_weight, u64 input_index);
V4  internal_in_smooth_v4  (Shader_Value_Type_Array *output_pattern, u8 *v0_ptr, u8 *v1_ptr, u8 *v2_ptr, f32 v0_weight, f32 v1_weight, f32 v2_weight, u64 input_index);

f32 internal_in_flat_f32 (Shader_Value_Type_Array *output_pattern, u8 *input_ptr, u64 input_index);
V2  internal_in_flat_v2  (Shader_Value_Type_Array *output_pattern, u8 *input_ptr, u64 input_index);
V3  internal_in_flat_v3  (Shader_Value_Type_Array *output_pattern, u8 *input_ptr, u64 input_index);
V4  internal_in_flat_v4  (Shader_Value_Type_Array *output_pattern, u8 *input_ptr, u64 input_index);
M3  internal_in_flat_m3  (Shader_Value_Type_Array *output_pattern, u8 *input_ptr, u64 input_index);
M4  internal_in_flat_m4  (Shader_Value_Type_Array *output_pattern, u8 *input_ptr, u64 input_index);


#define in_smooth_f32 (input_index)  internal_in_smooth_f32 (output_pattern, v0_inputs, v1_inputs, v2_inputs, barycentric_coordinates.a, barycentric_coordinates.b barycentric_coordinates.c, input_index) 
#define in_smooth_v2  (input_index)  internal_in_smooth_v2  (output_pattern, v0_inputs, v1_inputs, v2_inputs, barycentric_coordinates.a, barycentric_coordinates.b barycentric_coordinates.c, input_index) 
#define in_smooth_v3  (input_index)  internal_in_smooth_v3  (output_pattern, v0_inputs, v1_inputs, v2_inputs, barycentric_coordinates.a, barycentric_coordinates.b barycentric_coordinates.c, input_index) 
#define in_smooth_v4  (input_index)  internal_in_smooth_v4  (output_pattern, v0_inputs, v1_inputs, v2_inputs, barycentric_coordinates.a, barycentric_coordinates.b barycentric_coordinates.c, input_index) 

#define in_flat_f32(input_index) internal_in_flat_f32 (output_pattern, v0_inputs, input_index) 
#define in_flat_v2(input_index)  internal_in_flat_v2  (output_pattern, v0_inputs, input_index) 
#define in_flat_v3(input_index)  internal_in_flat_v3  (output_pattern, v0_inputs, input_index) 
#define in_flat_v4(input_index)  internal_in_flat_v4  (output_pattern, v0_inputs, input_index) 
#define in_flat_m3(input_index)  internal_in_flat_m3  (output_pattern, v0_inputs, input_index) 
#define in_flat_m4(input_index)  internal_in_flat_m4  (output_pattern, v0_inputs, input_index) 

#define fragment_shader_output(color_result) internal_fragment_shader_output(output_ptr, color_result)
void internal_fragment_shader_output(u8 *output_ptr, V4 color_result);

#undef Shader_Value_Types