#include "shader.h"
#include "memory.h"
#include "util.h"

Shader_Value_Array init_shader_uniforms(Arena *arena) {
    Shader_Value_Array result = {};
    result.uniforms = arena->data + arena->end; // this is risky, we are essentially betting we will use this specific address
    
    return result;
}

// note: for now uniforms have to be added in order, if you allocate within the struct something else it can mess up and read wrong values
void push_shader_uniform(Shader_Value_Array *uniforms, Arena *arena, Shader_Value value) {
    arena_push_struct(arena, sizeof(Shader_Value), 1);
    uniforms[uniforms->count] = value;
    ++uniforms->count;
}

void add_position_vertex_attribute(Vertex_Attributes_Array *vertex_attributes_array, v3f *positions) {
    vertex_attributes_array->positions = positions;
}
void add_normal_vertex_attribute(Vertex_Attributes_Array *vertex_attributes_array, v3f *normals) {
    vertex_attributes_array->normals = normals;
}
void add_texture_coordinate_vertex_attribute(Vertex_Attributes_Array *vertex_attributes_array, v3f *texture_coordinates) {
    vertex_attributes_array->texture_coordinates = texture_coordinates;
}
void add_color_vertex_attribute(Vertex_Attributes_Array *vertex_attributes_array, v3f *colors) {
    vertex_attributes_array->colors = colors;
}

Shader_Pipeline create_shader_pipeline(Shader_Value_Array uniforms, Vertex_Attributes_Array attributes_array, Vertex_Shader vertex_shader, Fragment_Shader fragment_shader) {
    Shader_Pipeline result = {};
    result.uniforms = uniforms;
    result.attributes_array = attributes_array;
    result.vertex_shader = vertex_shader;
    result.fragment_shader = fragment_shader;
    
    return result;
}

// todo: implement arenas for uniforms array and inputs and outputs
void draw_object(Render_entity entity, Shader_Pipeline shader) {
    Shader_Value_Array uniforms = shader->uniforms;
    Vertex_Attributes_Array attributes_array = shader->vertex_attributes_array;
    
    for (u32 triangle_index = 0; triangle_index < entity.triangle_count; ++triangle_index) {
        Triangle final_triangle = entity.triangles[triangle_index];
        
        for (u32 pos_index = 0; pos_index < 3; ++pos_index) {
            Vertex_Attributes attributes = shader.attributes.pos
            Vertex_Shader_Output vs_result = {};
            final_triangle.vertices[pos_index] = vertex_shader(final_triangle.attributes, &vs_result);
            u32 final_color = fragment_shader(
        }
    }

}