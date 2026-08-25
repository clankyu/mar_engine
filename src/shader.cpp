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

Clip_Triangle perspective_divide(Clip_Triangle triangle) {
    Clip_Triangle result = triangle;
    result.v0 = result.v0 / result.v0.w;
    result.v1 = result.v0 / result.v0.w;
    result.v2 = result.v0 / result.v0.w;
    
    return result;
}

// note: this is honestly really risky, might bite me in the ass
static V4 get_clip_vertex_pos(u8 **vertex_shader_output_array, u64 triangle_index, u64 vertex_index) {
    V4 result = (V4*)vertex_shader_output_array[triangle_index] + vertex_index;
    return result;
}

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

void program() {
    Arena vertex_scratch_arena = arena_init(MiB(2));
    Arena uniform_arena = arena_init(MiB(20));
    Arena cpu_arena = arena_init(MiB(50));
    Arena gpu_arena = arena_init(MiB(60));
    arena_touch_pages(&vertex_scratch_arena);
    arena_touch_pages(&uniform_arena);
    arena_touch_pages(&cpu_arena);
    arena_touch_pages(&gpu_arena);
    
    V3 *pos = get_obj_v();
    V3 *normals = get_obj_vn();
    u64 vertex_count = get_obj_vertex_count();
    u64 triangle_count = get_obj_triangle_count();
    
    M4 model = get_model_matrix();
    M4 view = get_view_matrix();
    M4 projection = get_projection_matrix();
    
    Shader_Pipeline pipeline = create_shader_pipeline();
    add_attribute(&pipeline, pos, Shader_Value_Type_V3, vertex_count);
    add_attribute(&pipeline, normals, Shader_Value_Type_V3, vertex_count);
    
    add_uniform(&pipeline, &model, Shader_Value_Type_M4);
    add_uniform(&pipeline, &view, Shader_Value_Type_M4);
    add_uniform(&pipeline, &projection, Shader_Value_Type_M4);
    
    while (running) {
        for (u64 vertex_index = 0; vertex_index < vertex_count; ++vertex_index) {
            V3 pos = load_attribute_v3(&pipeline, 0, vertex_index);
            V3 normal = load_attribute_v3(&pipeline, 1, vertex_index);
            
            M4 model = load_uniform_m4(&pipeline, 0);
            M4 view = load_uniform_m4(&pipeline, 1);
            M4 projection = load_uniform_m4(&pipeline, 2);
            
            // under the hood it pushes a v4 pos and a v3 normal together
            output_pos(&pipeline, projection * view * model * v4(pos, 1.0f)); 
            output_v3(&pipeline, normal);
        }
        
        // vertices are in clip space, so time to clip
        for (u64 triangle_index = 0; triangle_index < triangle_count; ++triangle_index) {
            V3u triangle_indices = get_vertex_indices(&pipeline, triangle_index);
            Clip_Triangle triangle = get_triangle(&pipeline, triangle_indices);
            V4 v0_pos = triangle.v0;
            V4 v1_pos = triangle.v1;
            V4 v2_pos = triangle.v2;
            
            if (triangle_in_view_frustum(triangle)) {
                if (v0_pos.z > v0_pos.w) {
                    if (v1_pos.z > v1_pos.w) {
                        clip_and_draw_two_out(&pipeline, triangle_indices.v0, triangle_indices.v1, triangle_indices.v2); 
                    } else if (v2_pos.z > v2_pos.w) {
                        clip_and_draw_two_out(&pipeline, triangle_indices.v0, triangle_indices.v2, triangle_indices.v1);
                    } else {
                        clip_and_draw_one_out(&pipeline, triangle_indices.v0, triangle_indices.v1, triangle_indices.v2);
                    }
                } else if (v1_pos.z > v1_pos.w) {
                    if (v2_pos.z > v2_pos.w) {
                        clip_and_draw_two_out(&pipeline, triangle_indices.v1, triangle_indices.v2, triangle_indices.v0);
                    } else {
                        clip_and_draw_one_out(&pipeline, triangle_indices.v1, triangle_indices.v0, triangle_indices.v2);
                    }
                } else if (v2_pos.z > v2_pos.w) {
                    clip_and_draw_one_out(&pipeline, triangle_indices.v2, triangle_indices.v0, triangle_indices.v1);
                } else {
                    draw_triangle(&pipeline, triangle_indices);
                }
            }
        }
    }
}

struct Clipping_Information {
    f32 alpha0_1, alpha1_0;
    f32 alpha1_2, alpha2_1;
    f32 alpha2_0, alpha0_2;
};

void clip_and_draw_one_out(Shader_Pipeline *pipeline, u32 out, u32 in1, u32 in2) {
    f32 in1_distance = in1.pos.w - in1.pos.z;
    f32 in2_distance = in2.pos.w - in2.pos.z;
    f32 out_distance = out.pos.w - out.pos.z;
    
    f32 alpha0 = out_distance / (out_distance - in1_distance);
    f32 alpha1 = out_distance / (out_distance - in2_distance);
    
    Vertex4d a = vertex4d_lerp(out, in1, alpha0);
    Vertex4d b = vertex4d_lerp(out, in2, alpha1);
    
    Clip_Triangle t0;
    Clip_Triangle t1; 
    
    t0.v0 = in1;
    t0.v1 = in2;
    t0.v2 = a;
    
    t1.v0 = in2;
    t1.v1 = a;
    t1.v2 = b;
    
    draw_triangle(frame_buffer, t0);
    draw_triangle(frame_buffer, t1);
}

void clip_and_draw_two_out(Shader_Pipeline *pipeline, u32 out1_index, u32 out2_index, u32 in_index) {
    V4 out1 = get_vertex(pipeline, out1_index); 
    V4 out2 = get_vertex(pipeline, out2_index);
    V4 in = get_vertex(pipeline, in_index);
    
    f32 in_distance = in.pos.w - in.pos.z;
    f32 out1_distance = out1.pos.w - out1.pos.z;
    f32 out2_distance = out2.pos.w - out2.pos.z;
    
    f32 alpha0 = out1_distance / (out1_distance - in_distance);
    f32 alpha1 = out2_distance / (out2_distance - in_distance);
    
    V4 a = v4_lerp(out1, in, alpha0);
    V4 b = v4_lerp(out2, in, alpha1);
    
    Clipping_Information clip_info;
    clip_info.alpha0_1 = alpha0; clip_info.alpha_1_0 = 0.0f;
    clip_info.alpha1_2 = 0.0f; clip_info.alpha_2_1 = alpha1;
    clip_info.alpha2_0 = 0.0f; clip_info.alpha0_2 = 0.0f;
    
    Clip_Triangle clipped_triangle = { a, in, b };
    V3u indices = { out1_index, in_index, out2_index };
    // who knows if this fucking works lol
    draw_clipped_triangle(pipeline, clipped_triangle, indices, clip_info);
} 

draw_clipped_triangle(Shader_Pipeline *pipeline, Clip_Triangle clipped_triangle, V3u indices, Clipping_Information clip_info) {
    Clip_Triangle ndc_triangle = perspective_divide(clipped_triangle);        
    assert(!triangle_should_be_clipped(ndc_triangle));
    
    u64 width = engine.window.width;
    u64 height = engine.window.height;
    
    Raster_Vertex v0 = ndc_to_raster(ndc_triangle.v0, 1.0f / clip_triangle.v0.w, width, height);
    Raster_Vertex v1 = ndc_to_raster(ndc_triangle.v1, 1.0f / clip_triangle.v1.w, width, height);
    Raster_Vertex v2 = ndc_to_raster(ndc_triangle.v2, 1.0f / clip_triangle.v2.w, width, height);            
    
    // todo: going to have to deal with attributes later on
    // todo: order indeces too
    order_vertices_clockwise(&v0, &v1,&v2);
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

                V3 normal = in_smooth_v3(pipeline, 0, );
                V2 pos = { x, y };
                
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

V3 in_smooth_v3(Shader_Pipeline *pipeline, u64 triangle_index, f32 v0_weight, f32 v1_weight, f32 v2_weight, V3u vertex_indices, Clipping_Information clip_info) {
    V3 result = {};
    V3 v0 = in_v3(pipeline, input_index, verte);
    V3 v1 = in_v3(pipeline, input_index, verte);
    V3 v2 = in_v3(pipeline, input_index, verte); // dude idk
    
    v0 = clip_info.alpha0_1 != 0.0f ? v3_lerp(v0, v1, clip_info.alpha0_1) : v0;     
    v0 = clip_info.alpha0_2 != 0.0f ? v3_lerp(v0, v2, clip_info.alpha0_2) : v0;     
     
    v1 = clip_info.alpha1_0 != 0.0f ? v3_lerp(v1, v0, clip_info.alpha1_0) : v1;     
    v1 = clip_info.alpha1_2 != 0.0f ? v3_lerp(v1, v2, clip_info.alpha1_2) : v1;     
    
    v2 = clip_info.alpha2_0 != 0.0f ? v3_lerp(v2, v0, clip_info.alpha2_0) : v2;     
    v2 = clip_info.alpha2_1 != 0.0f ? v3_lerp(v2, v1, clip_info.alpha2_1) : v2;     
    
    result = v0_weight * v0 + v1_weight * v1 + v2_weight * v2;
    return result;
}