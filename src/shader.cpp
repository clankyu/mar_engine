#include "shader.h"
#include "memory.h"
#include "util.h"

Shader_Value_Array init_shader_uniforms(Arena *arena) {
    Shader_Value_Array result = {};
    result.values = (Shader_Value*) arena_push(arena, 0);
    
    return result;
}

// note: for now uniforms have to be added in order, if you allocate within the struct something else it can mess up and read wrong values
void push_shader_uniform(Shader_Pipeline *pipeline, Arena *arena, Shader_Value value) {
    Shader_Value *new_uniform = (Shader_Value*) arena_push_struct(arena, sizeof(Shader_Value), 1);
    *new_uniform = value;
    ++pipeline->uniforms.count;
}

void add_position_vertex_attribute(Vertex_Attributes_Array *vertex_attributes_array, V3 *positions) {
    vertex_attributes_array->positions = positions;
}
void add_normal_vertex_attribute(Vertex_Attributes_Array *vertex_attributes_array, V3 *normals) {
    vertex_attributes_array->normals = normals;
}
void add_texture_coordinate_vertex_attribute(Vertex_Attributes_Array *vertex_attributes_array, V2 *texture_coordinates) {
    vertex_attributes_array->texture_coordinates = texture_coordinates;
}
void add_color_vertex_attribute(Vertex_Attributes_Array *vertex_attributes_array, V3 *colors) {
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
void draw_object(Render_Entity entity, Shader_Pipeline pipeline) {
    Shader_Value_Array uniforms = pipeline->uniforms;
    Vertex_Attributes_Array attributes_array = pipeline->attributes_array;
    
    for (u32 pos_index = 0; pos_index < entity.triangle_count * 3;) {
        Vertex_Shader_Result v0 = pipeline.vertex_shader.function(&pipeline.uniforms, &pipeline.attributes, pos_index++);
        Vertex_Shader_Result v1 = pipeline.vertex_shader.function(&pipeline.uniforms, &pipeline.attributes, pos_index++);
        Vertex_Shader_Result v2 = pipeline.vertex_shader.function(&pipeline.uniforms, &pipeline.attributes, pos_index++);
        
        Clip_Triangle triangle;
        if (triangle_in_view_frustum(triangle)) {
            clip_and_draw_triangle(triangle);
        }
    }

}


/*
void diffuse_lighting(VERTEX_SHADER_PARAMS) {
    m4f model = get_uniform_m4f(0);
    
    out_smooth_f32(
    out_flat_f32(
}
*/