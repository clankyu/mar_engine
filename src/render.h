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
    
    result = types_sizes[type];
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

struct Vertex_Shader_Result {
    Shader_Value_Array *outputs;
    V4 position; // ideally clip space position
};

#define VERTEX_SHADER_PARAMETERS Shader_Value_Array, Shader_Value_Array, u64
#define VERTEX_SHADER_FUNCTION void

struct Vertex_Shader {
    Vertex_Shader_Function function;
};

#define FRAGMENT_SHADER_PARAMETERS Shader_Value_Array attributes_array, u8* inputs, V3u triangle_indices, V3 barycentric_coordinates 
typedef Colorf (*Fragment_Shader_Function)(FRAGMENT_SHADER_PARAMETERS);
struct Fragment_Shader {
    Fragment_Shader_Function function;
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

struct Clipping_Information {
    f32 alpha0_1, alpha1_0;
    f32 alpha1_2, alpha2_1;
    f32 alpha2_0, alpha0_2;
};

void add_attribute(Shader_Pipeline *pipeline, u8 *data, Shader_Value_Type type, u64 count);
void add_uniform(Shader_Pipeline *pipeline, u8 *data, Shader_Value_Type type);
u64 get_pattern_offset(Shader_Value_Type_Array *pattern, u64 input_index);
u64 get_shader_value_type_array_size(Shader_Value_Array *arr);
u8 *get_inputs_ptr(Shader_Pipeline *pipeline, u64 vertex_index);
V4 get_pos(Shader_Pipeline *pipeline, u64 vertex_index);
V3u get_vertex_indices(Shader_Pipeline *pipeline, u64 triangle_index);
Clip_Triangle get_triangle(Shader_Pipeline *pipeline, V3u triangle_indices);

Shader_Pipeline create_shader_pipeline(Arena *clipping_arena, Vertex_Shader vertex_shader, Fragment_Shader fragment_shader);

f32 load_attribute_f32 (Shader_Pipeline *pipeline, u64 vertex_index, u64 attribute_index);
V2  load_attribute_v2  (Shader_Pipeline *pipeline, u64 vertex_index, u64 attribute_index);
V3  load_attribute_v3  (Shader_Pipeline *pipeline, u64 vertex_index, u64 attribute_index);
V4  load_attribute_v4  (Shader_Pipeline *pipeline, u64 vertex_index, u64 attribute_index);
M3  load_attribute_m3  (Shader_Pipeline *pipeline, u64 vertex_index, u64 attribute_index);
M4  load_attribute_m4  (Shader_Pipeline *pipeline, u64 vertex_index, u64 attribute_index);

f32 load_uniform_f32 (Shader_Pipeline *pipeline, u64 uniform_index);
V2  load_uniform_v2  (Shader_Pipeline *pipeline, u64 uniform_index);
V3  load_uniform_v3  (Shader_Pipeline *pipeline, u64 uniform_index);
V4  load_uniform_v4  (Shader_Pipeline *pipeline, u64 uniform_index);
M3  load_uniform_m3  (Shader_Pipeline *pipeline, u64 uniform_index);
M4  load_uniform_m4  (Shader_Pipeline *pipeline, u64 uniform_index);


// this is probably very slow, copying byte by byte, but for now ok i guess
inline void output(Shader_Pipeline *pipeline, u8 *value_ptr, Shader_Value_Type type) {
    if (!pipeline->output_pattern_initialized) { da_append(pipeline->output_pattern, type); }
    u64 size = get_shader_value_type_size(type);
    u8 *result = arena_push(pipeline->gpu_arena, size);
    memcpy(result, value_ptr, size);
}

inline void output_pos(u8 *output_ptr, V4 pos) {
    *(V4*)output_ptr = pos;
}

inline void output_f32(Shader_Pipeline *pipeline, f32 value) {
    output(pipeline, (u8*)&value, Shader_Value_Type_f32);
}        
inline void output_v2(Shader_Pipeline *pipeline, V2 value) {
    output(pipeline, (u8*)&value, Shader_Value_Type_V2);
}        
inline void output_v3(Shader_Pipeline *pipeline, V3 value) {
    output(pipeline, (u8*)&value, Shader_Value_Type_V3);
}
inline void output_v4(Shader_Pipeline *pipeline, V4 value) {
    output(pipeline, (u8*)&value, Shader_Value_Type_V4);
}
inline void output_m3(Shader_Pipeline *pipeline, M3 value) {
    output(pipeline, (u8*)&value, Shader_Value_Type_M3);
}
inline void output_m4(Shader_Pipeline *pipeline, M4 value) {
    output(pipeline, (u8*)&value, Shader_Value_Type_M4);
}

void clip_and_draw_one_out(Shader_Pipeline *pipeline, u32 out_index, u32 in1_index, u32 in2_index);
void clip_and_draw_two_out(Shader_Pipeline *pipeline, u32 out1_index, u32 out2_index, u32 in_index);

// note: be careful when using these
f32 internal_in_smooth_f32 (Shader_Pipeline *pipeline, u8 *v0_ptr, u8 *v1_ptr, u8 *v2_ptr, f32 v0_weight, f32 v1_weight, f32 v2_weight, u64 input_index);
V2 internal_in_smooth_v2   (Shader_Pipeline *pipeline, u8 *v0_ptr, u8 *v1_ptr, u8 *v2_ptr, f32 v0_weight, f32 v1_weight, f32 v2_weight, u64 input_index);
V3 internal_in_smooth_v3   (Shader_Pipeline *pipeline, u8 *v0_ptr, u8 *v1_ptr, u8 *v2_ptr, f32 v0_weight, f32 v1_weight, f32 v2_weight, u64 input_index);
V4 internal_in_smooth_v4   (Shader_Pipeline *pipeline, u8 *v0_ptr, u8 *v1_ptr, u8 *v2_ptr, f32 v0_weight, f32 v1_weight, f32 v2_weight, u64 input_index);

f32 internal_in_flat_f32 (Shader_Pipeline *pipeline, u8 *input_ptr, u64 input_index);
V2 internal_in_flat_v2   (Shader_Pipeline *pipeline, u8 *input_ptr, u64 input_index);
V3 internal_in_flat_v3   (Shader_Pipeline *pipeline, u8 *input_ptr, u64 input_index);
V4 internal_in_flat_v4   (Shader_Pipeline *pipeline, u8 *input_ptr, u64 input_index);
M3 internal_in_flat_m3   (Shader_Pipeline *pipeline, u8 *input_ptr, u64 input_index);
M4 internal_in_flat_m4   (Shader_Pipeline *pipeline, u8 *input_ptr, u64 input_index);

// note: ONLY use these inside of a fragment shader
#define FRAGMMENT_SHADER_PARAMETERS Shader_Pipeline *pipeline, u8 *v0_inputs, u8 *v1_inputs, u8 *v2_inputs, V3 barycentric_coordinates, u8 *result_ptr
#define FRAGMENT_SHADER_FUNCTION V4
#define in_smooth_f32(input_index) internal_in_smooth_f32 (pipeline, v0_inputs, v1_inputs, v2_inputs, barycentric_coordinates.a, barycentric_coordinates.b barycentric_coordinates.c, input_index) 
#define in_smooth_v2(input_index)  internal_in_smooth_v2  (pipeline, v0_inputs, v1_inputs, v2_inputs, barycentric_coordinates.a, barycentric_coordinates.b barycentric_coordinates.c, input_index) 
#define in_smooth_v3(input_index)  internal_in_smooth_v3  (pipeline, v0_inputs, v1_inputs, v2_inputs, barycentric_coordinates.a, barycentric_coordinates.b barycentric_coordinates.c, input_index) 
#define in_smooth_v4(input_index)  internal_in_smooth_v4  (pipeline, v0_inputs, v1_inputs, v2_inputs, barycentric_coordinates.a, barycentric_coordinates.b barycentric_coordinates.c, input_index) 

#define in_flat_f32(input_index) internal_in_flat_f32 (pipeline, v0_inputs, input_index) 
#define in_flat_v2(input_index)  internal_in_flat_v2  (pipeline, v0_inputs, input_index) 
#define in_flat_v3(input_index)  internal_in_flat_v3  (pipeline, v0_inputs, input_index) 
#define in_flat_v4(input_index)  internal_in_flat_v4  (pipeline, v0_inputs, input_index) 
#define in_flat_m3(input_index)  internal_in_flat_m3  (pipeline, v0_inputs, input_index) 
#define in_flat_m4(input_index)  internal_in_flat_m4  (pipeline, v0_inputs, input_index) 

void internal_fragment_shader_output(u8 *result_ptr, V4 color_result);
#define fragment_shader_output(color_result) internal_fragment_shader_output(result_ptr, color_result)

#undef Shader_Value_Types