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

struct Vertex_Shader_Result {
    Shader_Value_Array *outputs;
    V4 position; // ideally clip space position
};

#define VERTEX_SHADER_PARAMETERS(Shader_Value_Array, Shader_Value_Array, u64)
typedef Vertex_Shader_Result (*Vertex_Shader_Function)(VERTEX_SHADER_PARAMETERS);
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

void add_attribute(Shader_Pipeline *pipeline, u8 *data, Shader_Value_Type type, u64 count);
void add_uniform(Shader_Pipeline *pipeline, u8 *data, Shader_Value_Type type);
u64 get_pattern_offset(Shader_Value_Type_Array *pattern, u64 input_index);
u64 get_shader_value_type_array_size(Shader_Value_Array *arr);
u8 *get_inputs_ptr(Shader_Pipeline *pipeline, u64 vertex_index);
V4 get_pos(Shader_Pipeline *pipeline, u64 vertex_index);

Shader_Pipeline create_shader_pipeline(Arena *clipping_arena, Vertex_Shader vertex_shader, Fragment_Shader fragment_shader);

inline void output_pos(u8 *output_ptr, V4 pos) {
    *output_ptr = pos;
}

inline void output_f32(Shader_Pipeline *pipeline, f32 value) {
    output(pipeline, &value, Shader_Value_Type_f32);
}        
inline void output_v2(Shader_Pipeline *pipeline, V2 value) {
    output(pipeline, &value, Shader_Value_Type_V2);
}        
inline void output_v3(Shader_Pipeline *pipeline, V3 value) {
    output(pipeline, &value, Shader_Value_Type_V3);
}
inline void output_v4(Shader_Pipeline *pipeline, V4 value) {
    output(pipeline, &value, Shader_Value_Type_V4);
}
inline void output_m3(Shader_Pipeline *pipeline, M3 value) {
    output(pipeline, &value, Shader_Value_Type_M3);
}
inline void output_m4(Shader_Pipeline *pipeline, M4 value) {
    output(pipeline, &value, Shader_Value_Type_M4);
}

// this is probably very slow, copying byte by byte, but for now ok i guess
inline void output(Shader_Pipeline *pipeline, u8 *value_ptr, Shader_Value_Type type) {
    if (!pipeline->output_pattern_initialized) { da_append(pipeline->output_pattern, type); }
    u64 size - get_shader_value_type_size(type);
    u8 *result = arena_push(pipeline->gpu_arena, size);
    memcpy(result, value_ptr, size);
}
#undef Shader_Value_Types

