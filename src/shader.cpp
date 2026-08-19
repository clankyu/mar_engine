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

// redo
Shader_Pipeline create_shader_pipeline(Shader_Value_Array uniforms, Vertex_Attributes_Array attributes_array, Vertex_Shader vertex_shader, Fragment_Shader fragment_shader) {
    Shader_Pipeline result = {};
    result.uniforms = uniforms;
    result.attributes_array = attributes_array;
    result.vertex_shader = vertex_shader;
    result.fragment_shader = fragment_shader;
    
    return result;
}

Clip_Triangle perspective_divide(Clip_Triangle triangle) {
    Clip_Triangle result = triangle;
    result.v0 = result.v0 / result.v0.w;
    result.v1 = result.v1 / result.v1.w;
    result.v2 = result.v2 / result.v2.w;
    
    return result;
}

void function thing() {
    add_attribute(arena, Shader_Value_Type_V3, &vertices);
};

void add_attribute(Shader_Pipeline *pipeline, Arena *arena, u8 *data, u64 size) {
    u8 *attribute_ptr = arena_push_struct(arena, sizeof(u8*), 1);
    pipeline->
}

// note: this is honestly really risky, might bite me in the ass
static V4 get_clip_vertex_pos(u8 **vertex_shader_output_array, u64 triangle_index, u64 vertex_index) {
    V4 result = (V4*)vertex_shader_output_array[triangle_index] + vertex_index;
    return result;
}

// todo: implement arenas for uniforms array and inputs and outputs
void draw_object(Render_Entity entity, Shader_Pipeline *pipeline, Arena *arena) {
    Shader_Value_Array uniforms = pipeline->uniforms;
    Vertex_Attributes_Array *attributes_array = pipeline->attributes_array;
    
    u64 vertex_count = entity.triangle_count * 3;
    u8 **vertex_shader_output_array = (u8**) arena_push_struct(arena, sizeof(u8*), vertex_count);
    
    // i think we can infer attribute sizes?
    for (u64 vertex_index = 0; vertex_index < vertex_count; vertex_index++) {
        pipeline.vertex_shader.function(&pipeline.uniforms, &pipeline.attributes, arena, vertex_shader_output_array, vertex_index);
    }

    // clip and rasterize
    for (u64 triangle_index = 0; triangle_index < entity.triangle_count; ++triangle_index) {
        u64 index0 = triangle_index * 3;
        u64 index1 = triangle_index * 3 + 1;
        u64 index2 = triangle_index * 3 + 2;
        
        Clip_Triangle pre_clipping_triangle;
        V4 clip_v0 = get_clip_vertex_pos(vertex_shader_output_array, triangle_index index0);
        V4 clip_v1 = get_clip_vertex_pos(vertex_shader_output_array, triangle_index index1);
        V4 clip_v2 = get_clip_vertex_pos(vertex_shader_output_array, triangle_index index2);
        
        pre_clipping_triangle.v0 = clip_v0;
        pre_clipping_triangle.v1 = clip_v1;
        pre_clipping_triangle.v2 = clip_v2;
        
        // problem: if we clip triangles, we have to keep this is mind to interpolate whatever attributes and outputs.
        // I think we can solve this by interpolating inside the function. I will have to get all attributes and outputs, and interpolate them
        // appropiately, i will have to create a new triangle tho, i think yes. There will be more triangles in the pipeline than originally oviously
        // so just take that in mind
        u32 clip_triangle_count = 0;
        Clip_Triangle clipped_triangles[2];
        clip_triangle(pre_clipping_triangle, clipped_triangles, &clip_triangle_count);
        
        for (u64 clip_triangle_index = 0; clip_triangle_index < clip_triangle_count; ++clip_triangle_index) {
            Clip_Triangle clip_triangle = clipped_triangles[clip_triangle_index];
            Clip_Triangle ndc_triangle = perspective_divide(clip_triangle);        
       
            Raster_Vertex v0 = ndc_to_raster(ndc_triangle.v0, 1.0f / clip_triangle.v0.w, width height);
            Raster_Vertex v1 = ndc_to_raster(ndc_triangle.v1, 1.0f / clip_triangle.v1.w, width height);
            Raster_Vertex v2 = ndc_to_raster(ndc_triangle.v2, 1.0f / clip_triangle.v2.w, width height);            
            
            // todo: going to have to deal with attributes later on
            // todo: order indeces too
            order_vertices_clockwise(&v0, &v1, &v2);

            f32 x_min = round(MAX(MIN(MIN(v0.x, v1.x), v2.x), 0.0f));
            f32 y_min = round(MAX(MIN(MIN(v0.y, v1.y), v2.y), 0.0f));
            f32 x_max = round(MIN(MAX(MAX(v0.x, v1.x), v2.x), width - 1.0f));
            f32 y_max = round(MIN(MAX(MAX(v0.y, v1.y), v2.y), height - 1.0f));
            
            V2 p0 = create_v2(x_min, y_min);
        
            // note: derived these by manually calculating delta w's for cols and rows (weird)
            f32 delta_w0_col = v1.y - v0.y;
            f32 delta_w1_col = v2.y - v1.y;
            f32 delta_w2_col = v0.y - v2.y;
        
            f32 delta_w0_row = v0.x - v1.x;
            f32 delta_w1_row = v1.x - v2.x;
            f32 delta_w2_row = v2.x - v0.x;
            
            // because of the previous reordering now i now v0 to v1 is top left, so i don't need to do this (but i'll leave it alone for now)
            s32 bias0 = is_top_left(v0.xy, v1.xy) ? 0 : -1;
            s32 bias1 = is_top_left(v1.xy, v2.xy) ? 0 : -1;
            s32 bias2 = is_top_left(v2.xy, v0.xy) ? 0 : -1;
            
            f32 w0_row = edge_function(v0.xy, v1.xy, p0) + bias0;
            f32 w1_row = edge_function(v1.xy, v2.xy, p0) + bias1;
            f32 w2_row = edge_function(v2.xy, v0.xy, p0) + bias2;
            f32 total_area = edge_function(v0.xy, v1.xy, v2.xy);
            
            for (s32 y = (s32)y_min; y <= y_max; ++y) {
                f32 w0 = w0_row;
                f32 w1 = w1_row;
                f32 w2 = w2_row;
        
                for (s32 x = (s32)x_min; x <= x_max; ++x) {
                    b32 inside = (
                        (w0 <= 0.0f && w1 <= 0.0f && w2 <= 0.0f)
                        || (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f)
                    );
        
                    if (inside) {
                        f32 v0_weight = w1 / total_area;
                        f32 v1_weight = w2 / total_area;
                        f32 v2_weight = w0 / total_area;
                        V3 barycentric_coordinates = { v0_weight, v1_weight, v2_weight };
                        V3u triangle_indices = { index0, index1, index2 }; // interpolation stuff
        
                        u32 result = pipeline->fragment_shader.function(&attributes_array, vertex_shader_output_array[triangle_index], triangle_indices, barycentric_coordinates);
                        f32 interpolated_z = 1.0f / (v0.one_over_w * v0_weight + v1.one_over_w * v1_weight + v2.one_over_w * v2_weight);
                        
                        if (pipeline.depth_testing) {
                            if (interpolated_z > frame_buffer->depth_buffer[x + y * width]) {
                                put_pixel(pipeline.frame_buffer, x, y, result);
                                pipeline.frame_buffer[x + y * width] = interpolated_z;
                            }
                        } else {
                            put_pixel(pipeline.frame_buffer, x, y, result);
                        }
                    }
        
                    w0 += delta_w0_col;
                    w1 += delta_w1_col;
                    w2 += delta_w2_col;
                }
                
                w0_row += delta_w0_row;
                w1_row += delta_w1_row;
                w2_row += delta_w2_row;
            }
        }
    }
}