#pragma once

#include "engine_math.h"
#include "render.h"
#include "memory.h"
#include "util.h"

enum Shader_Value_Type : u32 {
    Shader_Value_Type_f32,
    Shader_Value_Type_V2,
    Shader_Value_Type_V3,
    Shader_Value_Type_V4,
    Shader_Value_Type_M3,
    Shader_Value_Type_M4,
};

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

struct Vertex_Attributes {
    V3 pos; 
    V3 normal;
    V2 uv;
    V3 color;
};

struct Vertex_Shader_Result {
    Shader_Value_Array *outputs;
    V4 position; // ideally clip space position
};

#define VERTEX_SHADER_PARAMETERS(Shader_Value_Array, Shader_Value_Array, u64)
typedef Vertex_Shader_Result (*Vertex_Shader_Function)(VERTEX_SHADER_PARAMETERS);
struct Vertex_Shader {
    Vertex_Shader_Function function;
};


#define get_uniform_f32(index) uniforms.values[index].f32
#define get_uniform_v2(index) uniforms.values[index].V2
#define get_uniform_v3(index) uniforms.values[index].V3
#define get_uniform_v4(index) uniforms.values[index].V4
#define get_uniform_m3(index) uniforms.values[index].M3
#define get_uniform_m4(index) uniforms.values[index].M4

#define FRAGMENT_SHADER_PARAMETERS Shader_Value_Array attributes_array, u8* inputs, V3u triangle_indices, V3 barycentric_coordinates 
typedef Colorf (*Fragment_Shader_Function)(FRAGMENT_SHADER_PARAMETERS);
struct Fragment_Shader {
    Fragment_Shader_Function function;
};

struct Shader_Pipeline {
    Shader_Value_Array uniforms;
    Shader_Value_Array attributes_array;
    Vertex_Shader vertex_shader;
    Fragment_Shader fragment_shader;
    Frame_Buffer *frame_buffer;
    b32 depth_testing;
};

Shader_Value_Array init_shader_uniforms(Arena *arena);
void push_shader_uniform(Shader_Value_Array *uniforms, Arena *arena, Shader_Value value);

void add_position_vertex_attribute(Shader_Value_Array *vertex_attributes_array, V3 *positions);
void add_normal_vertex_attribute(Shader_Value_Array *vertex_attributes_array, V3 *normals);
void add_texture_coordinate_vertex_attribute(Shader_Value_Array *vertex_attributes_array, V2 *texture_coordinates);
void add_color_vertex_attribute(Shader_Value_Array *vertex_attributes_array, V3 *colors);

Shader_Pipeline create_shader_pipeline(Shader_Value_Array uniforms, Shader_Value_Array attributes_array, Vertex_Shader vertex_shader, Fragment_Shader fragment_shader);

// todo: finish shader pipeline, interpolating, clipping, think about how to do it precisely