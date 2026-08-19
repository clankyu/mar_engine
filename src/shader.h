#pragma once

#include "engine_math.h"
#include "render.h"
#include "memory.h"
#include "util.h"

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
    Shader_value_Type *type_pattern;
};

void vertex_shader() {
    u8 *type_ptr = output_v3() -> arena_push_struct(arena, sizeof(Shader_Value_Type), 1);
}

Vertex_Output_Array vertex_shader(Arena *arena) {
    Vertex_Output_Array result = {};
    result.data = pipeline->vertex_shader.function();
    V4 *pos_ptr = arena_push_struct(arena, sizeof(V4), 1);
    *pos_ptr = pos;
        
    return pos;
}


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
#undef Shader_Value_Types