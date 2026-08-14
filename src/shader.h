#pragma once

#include "engine_math.h"
#include "memory.h"
#include "util.h"

enum Shader_Value_Type : u32 {
    Shader_Value_Type_f32,
    Shader_Value_Type_v2f,
    Shader_Value_Type_v3f,
    Shader_Value_Type_v4f,
    Shader_Value_Type_m3f,
    Shader_Value_Type_m4f,
};

struct Shader_Value {
    Shader_Value_Type type;
    union {
        f32 f32;
        v2f v2f;
        v3f v3f;
        v4f v4f;
        m3f m3f;
        m4f m4f;
    };
};

struct Shader_Value_Array {
    u64 count;
    Shader_Value *uniforms;
};

struct Vertex_Attributes {
    v3f pos; 
    v3f normal;
    v2f uv;
    v3f color;
};

struct Vertex_Attributes_Array {
    v3f *positions; 
    v3f *normals;
    v2f *texture_coordinates;
    v3f *colors;
};

typedef Vertex_Shader_Function (*Vertex_Shader_Result)(Shader_Value_Array, Vertex_Attributes_Array);
struct Vertex_Shader {
    Vertex_Shader_Function function;
};

struct Vertex_Shader_Result {
    Shader_Value_Array *outputs;
    v4f position; // ideally clip space position
};

typedef Fragment_Shader_Function (*Colorf)(Vertex_Shader_Result);
struct Fragment_Shader {
    Fragment_Shader_Function function;
};

struct Shader_Pipeline {
    Shader_Value_Array uniforms;
    Vertex_Attributes_Array attributes_array;
    Vertex_Shader vertex_shader;
    Fragment_Shader fragment_shader;
    b32 depth_testing;
};

Shader_Value_Array init_shader_uniforms(Arena *arena);
void push_shader_uniform(Shader_Value_Array *uniforms, Arena *arena, Shader_Value value);

void add_position_vertex_attribute(Vertex_Attributes_Array *vertex_attributes_array, v3f *positions);
void add_normal_vertex_attribute(Vertex_Attributes_Array *vertex_attributes_array, v3f *normals);
void add_texture_coordinate_vertex_attribute(Vertex_Attributes_Array *vertex_attributes_array, v3f *texture_coordinates);
void add_color_vertex_attribute(Vertex_Attributes_Array *vertex_attributes_array, v3f *colors);

Shader_Pipeline create_shader_pipeline(Shader_Value_Array uniforms, Vertex_Attributes_Array attributes_array, Vertex_Shader vertex_shader, Fragment_Shader fragment_shader);

// todo: finish shader pipeline, interpolating, clipping, think about how to do it precisely