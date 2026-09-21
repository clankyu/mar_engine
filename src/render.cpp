#include "render.h"
#include "engine.h"
#include "engine_math.h"

#include <stdio.h>
#include <math.h>

Triangle create_triangle(V3 v0, V3 v1, V3 v2, Color8 c0, Color8 c1, Color8 c2) {
    Triangle result = {};
    result.v0 = v0;
    result.v1 = v1;
    result.v2 = v2;
    result.c0 = c0;
    result.c1 = c1;
    result.c2 = c2;

    return result;
}

// note: for now we'll use [-1, 1] for x and y, and [0, 1] for z (0 being far
// plane and 1 near plane), so no need to change but for future graphics apis we
// may need to insert a function to change the coordinate system
Raster_Vertex ndc_to_raster(V4 ndc_vertex, f32 w, Colorf color, f32 width, f32 height) {
    Raster_Vertex result;
    f32 half_width = width / 2.0f;
    f32 half_height = height / 2.0f;
    
    result.pos.x = ndc_vertex.x * half_width + half_width;
    result.pos.y = -ndc_vertex.y * half_height + half_height; 
    
    result.color = color;
    result.depth = ndc_vertex.z;
    result.one_over_w = 1.0f / w;

    return result;
}

// center is (0, 0), x+ -> right, y+ -> up
// change this so it's like ndc [-1, 1] for x and y
void put_pixel(Frame_Buffer *frame_buffer, s32 x, s32 y, u32 color) {
    u32 width = frame_buffer->width;
    u32 height = frame_buffer->height;
    
    s32 x_screen = x;
    s32 y_screen = y;

    s32 frame_buffer_index = x_screen + y_screen * width;
    b32 inside = (x_screen >= 0 && x_screen < width)
        && (y_screen >= 0 && y_screen < height)
        && (frame_buffer_index >= 0 && frame_buffer_index < get_frame_buffer_pixel_count(frame_buffer));

    if (inside) {
        frame_buffer->color_buffer[frame_buffer_index] = color;
    }
}

void m4_print(M4 mat) {
    for (u32 row = 0; row < 4; ++row) {
        printf("| ");

        for (u32 column = 0; column < 4; ++column) {
            printf("%.2f ", mat.e[column][row]);
        }

        printf("|\n");
    }
}

// weird derefercing stuff but idk
void order_vertices_clockwise(Raster_Vertex *v0, Raster_Vertex *v1, Raster_Vertex *v2) {
    if (is_top_left(v2->xy, v1->xy)) {
        Raster_Vertex v0_temp = *v0;
        *v0 = *v2;
        *v2 = v0_temp;
    } else if (is_top_left(v0->xy, v2->xy)) {
        Raster_Vertex v2_temp = *v2;
        *v2 = *v1;
        *v1 = v2_temp;
    } else if (is_top_left(v1->xy, v0->xy)) {
        Raster_Vertex v0_temp = *v0;
        *v0 = *v1;
        *v1 = v0_temp;
    }
}

// weird derefercing stuff but idk
void order_vertices_clockwise_and_info_clockwise(V4 *v0, V4 *v1, V4 *v2, V3u *indices, Clipping_Information *clip_info) {
    if (is_top_left(v2->xy, v1->xy)) {
        Raster_Vertex v0_temp = *v0;
        *v0 = *v2;
        *v2 = v0_temp;
        
        u32 v0_index_temp = *indices->v0;
        indices->v0 = indices->v2;
        indices->v2 = v0_index_temp;
        
        // note: very weird but i think it works lol
        Clipping_Information temp = *clip_info;
        clip_info->alpha0_1 = temp.alpha2_1;
        clip_info->alpha0_2 = temp.alpha2_0;
        clip_info->alpha2_0 = temp.alpha0_2;
        clip_info->alpha2_1 = temp.alpha0_1;
        clip_info->alpha1_0 = temp.alpha1_2;
        clip_info->alpha1_2 = temp.alpha1_0;
    } else if (is_top_left(v0->xy, v2->xy)) {
        Raster_Vertex v2_temp = *v2;
        *v2 = *v1;
        *v1 = v2_temp;
        
        u32 v2_index_temp = *indices->v2;
        indices->v2 = indices->v1;
        indices->v1 = v2_index_temp;
        
        Clipping_Information temp = *clip_info;
        
        clip_info->alpha2_0 = temp.alpha1_0;
        clip_info->alpha2_1 = temp.alpha1_2;
        clip_info->alpha1_0 = temp.alpha2_0;
        clip_info->alpha1_2 = temp.alpha2_1;
        clip_info->alpha0_1 = temp.alpha0_2;
        clip_info->alpha0_1 = temp.alpha0_1;
    } else if (is_top_left(v1->xy, v0->xy)) {
        Raster_Vertex v0_temp = *v0;
        *v0 = *v1;
        *v1 = v0_temp;
        
        u32 v0_index_temp = *indices->v0;
        indices->v0 = indices->v1;
        indices->v1 = v0_index_temp;
        
        Clipping_Information temp = *clip_info;
        clip_info->alpha0_1 = temp.alpha1_0;
        clip_info->alpha0_2 = temp.alpha1_2;
        clip_info->alpha1_0 = temp.alpha0_1;
        clip_info->alpha1_2 = temp.alpha0_2;
        clip_info->alpha2_0 = temp.alpha2_1;
        clip_info->alpha2_1 = temp.alpha2_0;
    }
}

// checks if it's atleast partially in, only returns false if it's completely outside the view frustum
b32 triangle_in_view_frustum(Clip_Triangle t) {
    V4 v0 = t.v0.pos;
    V4 v1 = t.v1.pos;
    V4 v2 = t.v2.pos;
    
    if (v0.x > v0.w &&
        v1.x > v1.w && 
        v2.x > v2.w)
    {
        return false;
    }
    if (v0.x < -v0.w &&
        v1.x < -v1.w && 
        v2.x < -v2.w)
    {
        return false;
    } 
    if (v0.y > v0.w &&
        v1.y > v1.w && 
        v2.y > v2.w)
    {
        return false;
    }  
    if (v0.y < -v0.w &&
        v1.y < -v1.w && 
        v2.y < -v2.w)
    {
        return false;
    }  
    // I think this is correct, since near plane is equal to 1 in ndc
    if (v0.z > v0.w &&
        v1.z > v1.w && 
        v2.z > v2.w)
    {
        return false;
    }  
    // I think this is also correct?
    if (v0.z < 0.0f &&
        v1.z < 0.0f && 
        v2.z < 0.0f)
    {
        return false;
    }  
    
    return true;  
}

Clip_Triangle perspective_divide(Clip_Triangle triangle) {
    Clip_Triangle result = triangle;
    result.v0 = result.v0 / result.v0.w;
    result.v1 = result.v1 / result.v1.w;
    result.v2 = result.v2 / result.v2.w;
    
    return result;
}

void program() {
    Arena vertex_scratch_arena = arena_init(MiB(2));
    Arena cpu_arena = arena_init(MiB(50));
    Arena gpu_arena = arena_init(MiB(60));
    arena_touch_pages(&vertex_scratch_arena);
    arena_touch_pages(&cpu_arena);
    arena_touch_pages(&gpu_arena);
    
    V3 *pos = 0;
    V3 *normals = 0;
    u64 vertex_count = 0;
    u64 triangle_count = 0;
    
    M4 model = {};
    M4 view = {};
    M4 projection = {};
    
    Shader_Pipeline pipeline = create_shader_pipeline();
    
    add_attribute(&pipeline, (u8*)pos, Shader_Value_Type_V3, vertex_count);
    add_attribute(&pipeline, (u8*)normals, Shader_Value_Type_V3, vertex_count);

    add_uniform(&pipeline, &model, Shader_Value_Type_M4);
    add_uniform(&pipeline, &view, Shader_Value_Type_M4);
    add_uniform(&pipeline, &projection, Shader_Value_Type_M4);
    
    while (running) {
        for (u64 vertex_index = 0; vertex_index < vertex_count; ++vertex_index) {
            u8 *output_ptr = arena_push_struct(&pipeline->gpu_arena, sizeof(V4), 1);
            V3 pos = load_attribute_v3(&pipeline, 0, vertex_index);
            V3 normal = load_attribute_v3(&pipeline, 1, vertex_index);
            
            M4 model = load_uniform_m4(&pipeline, 0);
            M4 view = load_uniform_m4(&pipeline, 1);
            M4 projection = load_uniform_m4(&pipeline, 2);
            
            // under the hood it pushes a v4 pos and a v3 normal together
            output_pos(output_ptr, projection * view * model * create_v4(pos, 1.0f)); 
            output_v3(&pipeline, normal);
        }
        
        // vertices are in clip space, so time to clip
        for (u64 triangle_index = 0; triangle_index < triangle_count; ++triangle_index) {
            V3u triangle_indices = get_vertex_indices(&pipeline, triangle_index);
            Clip_Triangle triangle = get_triangle(&pipeline, triangle_indices); // do this
            
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
                    u8 *v0_inputs = get_inputs_ptr(&pipeline, triangle_indices.v0);
                    u8 *v1_inputs = get_inputs_ptr(&pipeline, triangle_indices.v1);
                    u8 *v2_inputs = get_inputs_ptr(&pipeline, triangle_indices.v2);
            
                    draw_triangle(&pipeline, v0_inputs, v1_inputs, v2_inputs);
                }
            }
        }
    }
}

u64 get_shader_value_type_array_size(Shader_Value_Array *arr) {
    u64 result = 0;
    for (u64 index = 0; index < arr->count; ++index) {
        u64 size = get_shader_value_type_size(arr->items[index].type);
        result += size;
    }
    
    return result;
}

u8 *get_inputs_ptr(Shader_Pipeline *pipeline, u64 vertex_index) {
    u8 *result = 0;
    u64 pattern_size = get_shader_value_type_array_size(pipeline->output_pattern);
    
    // todo: to take into account different things we need the offset or pointer to the start of outputs or whatever
    result = pipeline->gpu_arena->data + pattern_size;
    return result;
}

V4 get_pos(Shader_Pipeline *pipeline, u64 vertex_index) {
    V4 result = {};
    V4 *val_ptr = (V4*)get_inputs_ptr(pipeline, vertex_index);
    result = *val_ptr; // this is because the position is the first element of the inputs array
    
    return result;
}

void clip_and_draw_one_out(Shader_Pipeline *pipeline, u32 out_index, u32 in1_index, u32 in2_index) {
    V4 out = get_vertex(pipeline, out_index); // todo: implement this lol
    V4 in1 = get_vertex(pipeline, in1_index);
    V4 in2 = get_vertex(pipeline, in2_index);
    
    f32 in1_distance = in1.w - in1.z;
    f32 in2_distance = in2.w - in2.z;
    f32 out_distance = out.w - out.z;
    
    f32 alpha0 = out_distance / (out_distance - in1_distance);
    f32 alpha1 = out_distance / (out_distance - in2_distance);
    
    V3u indices0 = (V3u) { in1_index, in2_index, out_index };
    V3u indices1 = (V3u) { in1_index, in2_index, out_index };
    
    // manually derived on notebook
    Clipping_Information clip_info0 = {};
    clip_info0.alpha0_1 = 0.0f; clip_info0.alpha1_0 = 0.0f;
    clip_info0.alpha0_2 = 0.0f; clip_info0.alpha2_0 = alpha0;
    clip_info0.alpha1_2 = 0.0f; clip_info0.alpha2_1 = alpha1;
    
    Clipping_Information clip_info1 = {};
    clip_info1.alpha0_1 = 0.0f; clip_info1.alpha1_0 = 0.0f;
    clip_info1.alpha0_2 = 1.0f - alpha0; clip_info.alpha2_0 = alpha0;
    clip_info1.alpha1_2 = 0.0f; clip_info1.alpha2_1 = alpha1;
    
    order_vertices_and_info_clockwise(&in1, &in2, &out, &indices0, &clip_info0);
    order_vertices_and_info_clockwise(&in1, &in2, &out, &indices1, &clip_info1);
    
    // note: weird thing lol but basically copying to scratch arena
    u64 pattern_size = get_shader_value_type_array_size(&pipeline->output_pattern);
    u8 *v0_ptr0 = arena_push(&pipeline->clipping_arena, pattern_size);
    memcpy(v0_ptr0, get_inputs_ptr(indices0.v0), pattern_size);
    u8 *v1_ptr0 = arena_push(&pipeline->clipping_arena, pattern_size);
    memcpy(v1_ptr0, get_inputs_ptr(indices0.v1), pattern_size);
    u8 *v2_ptr0 = arena_push(&pipeline->clipping_arena, pattern_size);
    memcpy(v2_ptr0, get_inputs_ptr(indices0.v2), pattern_size);
    
    u8 *v0_ptr1 = arena_push(&pipeline->clipping_arena, pattern_size);
    memcpy(v0_ptr1, get_inputs_ptr(indices1.v0), pattern_size);
    u8 *v1_ptr1 = arena_push(&pipeline->clipping_arena, pattern_size);
    memcpy(v1_ptr1, get_inputs_ptr(indices1.v1), pattern_size);
    u8 *v1_ptr1 = arena_push(&pipeline->clipping_arena, pattern_size);
    memcpy(v2_ptr1, get_inputs_ptr(indices1.v2), pattern_size);
    
    interpolate_clipped_inputs(pipeline, v0_ptr0, v1_ptr0, v2_ptr0, clip_info0);
    interpolate_clipped_inputs(pipeline, v0_ptr1, v1_ptr1, v2_ptr1, clip_info1);
    
    draw_triangle(pipeline, v0_ptr0, v1_ptr0, v2_ptr0);
    draw_triangle(pipeline, v0_ptr1, v1_ptr1, v2_ptr1);
}


void clip_and_draw_two_out(Shader_Pipeline *pipeline, u32 out1_index, u32 out2_index, u32 in_index) {
    V4 out1 = get_vertex(pipeline, out1_index); // todo: implement this lol
    V4 out2 = get_vertex(pipeline, out2_index);
    V4 in = get_vertex(pipeline, in_index);
    
    f32 in_distance = in.pos.w - in.pos.z;
    f32 out1_distance = out1.pos.w - out1.pos.z;
    f32 out2_distance = out2.pos.w - out2.pos.z;
    
    f32 alpha0 = out1_distance / (out1_distance - in_distance);
    f32 alpha1 = out2_distance / (out2_distance - in_distance);
    
    // note: i think this stuff works, but need to check out
    V3u indices = { out1_index, in_index, out2_index };
    
    Clipping_Information clip_info;
    clip_info.alpha0_1 = alpha0; clip_info.alpha_1_0 = 0.0f;
    clip_info.alpha1_2 = 0.0f; clip_info.alpha_2_1 = alpha1;
    clip_info.alpha2_0 = 0.0f; clip_info.alpha0_2 = 0.0f;
    
    order_vertices_and_info_clockwise(&a, &in, &b, &indices, &clip_info);
    
    u64 pattern_size = get_shader_value_type_array_size(&pipeline->output_pattern);
    u8 *v0_ptr = arena_push(&pipeline->clipping_arena, pattern_size);
    memcpy(v0_ptr, get_inputs_ptr(indices.v0), pattern_size);
    
    u8 *v1_ptr = get_inputs_ptr(pipeline, in_index);
    
    u8 *v2_ptr = arena_push(&pipeline->clipping_arena, pattern_size);
    memcpy(v2_ptr, get_inputs_ptr(indices.v2), pattern_size);
    
    interpolate_clipped_inputs(pipeline, v0_ptr, v1_ptr, v2_ptr, clip_info);
    
    draw_triangle(pipeline, v0_ptr, v1_ptr, v2_ptr);
} 

void interpolate_clipped_inputs(Shader_Pipeline *pipeline, u8 *v0_ptr, u8 *v1_ptr, u8 *v2_ptr, Clipping_Information clip_info) {
    u64 input_count = pipeline->output_pattern.count;
    u64 offset = 0;
    for (u64 input_index = 0; input_index < input_count; ++input_index) {
        Shader_Value_Type type = pipeline->output_pattern.items[input_index];
        
        switch (type) {
            case Shader_Value_Type_f32: {
                f32 *v0 = (f32*) v0_ptr + offset;
                f32 *v1 = (f32*) v1_ptr + offset;
                f32 *v2 = (f32*) v2_ptr + offset;
                
                *v0 = lerp(*v0, *v1, clip_info.alpha0_1);     
                *v0 = lerp(*v0, *v2, clip_info.alpha0_2);     
  
                *v1 = lerp(*v1, *v0, clip_info.alpha1_0);     
                *v1 = lerp(*v1, *v2, clip_info.alpha1_2);     
 
                *v2 = lerp(*v2, *v0, clip_info.alpha2_0);     
                *v2 = lerp(*v2, *v1, clip_info.alpha2_1);     
                
                break;
            }
            case Shader_Value_Type_V2: {
                V2 *v0 = (V2*) v0_ptr + offset;
                V2 *v1 = (V2*) v1_ptr + offset;
                V2 *v2 = (V2*) v2_ptr + offset;
                
                *v0 = v2_lerp(*v0, *v1, clip_info.alpha0_1);     
                *v0 = v2_lerp(*v0, *v2, clip_info.alpha0_2);     

                *v1 = v2_lerp(*v1, *v0, clip_info.alpha1_0);     
                *v1 = v2_lerp(*v1, *v2, clip_info.alpha1_2);     

                *v2 = v2_lerp(*v2, *v0, clip_info.alpha2_0);     
                *v2 = v2_lerp(*v2, *v1, clip_info.alpha2_1);     
                
                break;
            }
            case Shader_Value_Type_V3: {
                V3 *v0 = (V3*) v0_ptr + offset;
                V3 *v1 = (V3*) v1_ptr + offset;
                V3 *v2 = (V3*) v2_ptr + offset;
                
                *v0 = v3_lerp(*v0, *v1, clip_info.alpha0_1);     
                *v0 = v3_lerp(*v0, *v2, clip_info.alpha0_2);     

                *v1 = v3_lerp(*v1, *v0, clip_info.alpha1_0);     
                *v1 = v3_lerp(*v1, *v2, clip_info.alpha1_2);     

                *v2 = v3_lerp(*v2, *v0, clip_info.alpha2_0);     
                *v2 = v3_lerp(*v2, *v1, clip_info.alpha2_1);     
                
                break;
            }
            case Shader_Value_Type_V4: {
                V4 *v0 = (V4*) v0_ptr + offset;
                V4 *v1 = (V4*) v1_ptr + offset;
                V4 *v2 = (V4*) v2_ptr + offset;
                
                *v0 = v4_lerp(*v0, *v1, clip_info.alpha0_1);     
                *v0 = v4_lerp(*v0, *v2, clip_info.alpha0_2);     

                *v1 = v4_lerp(*v1, *v0, clip_info.alpha1_0);     
                *v1 = v4_lerp(*v1, *v2, clip_info.alpha1_2);     

                *v2 = v4_lerp(*v2, *v0, clip_info.alpha2_0);     
                *v2 = v4_lerp(*v2, *v1, clip_info.alpha2_1);     
                
                break;
            }
            case Shader_Value_Type_M3:
            case Shader_Value_Type_M4:
            default:
                break;
        }
        
        offset += get_shader_value_type_size(type);
    }
}

draw_triangle(Shader_Pipeline *pipeline, u8 *v0_inputs, u8 *v1_inputs, u8 *v2_inputs) {
    Clip_Triangle ndc_triangle = perspective_divide(clipped_triangle);        
    assert(!triangle_should_be_clipped(ndc_triangle));
    
    u64 width = engine.window.width;
    u64 height = engine.window.height;
    
    Raster_Vertex v0 = ndc_to_raster(ndc_triangle.v0, 1.0f / clip_triangle.v0.w, width, height);
    Raster_Vertex v1 = ndc_to_raster(ndc_triangle.v1, 1.0f / clip_triangle.v1.w, width, height);
    Raster_Vertex v2 = ndc_to_raster(ndc_triangle.v2, 1.0f / clip_triangle.v2.w, width, height);            
    
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

                V3 normal = in_smooth_v3(pipeline, 0, v0_weight, v1_weight, v2_weight, indices, clip_info);
                V2 pos = { x, y };
                
                u32 result = pipeline->fragment_shader.function(pipeline, v0_inputs, v1_inputs, v2_inputs, barycentric_coordinates);
                u32 fragment_shader_thing(Shader_Pipeline *pipeline, u8 *v0_inputs, u8 *v1_inputs, u8 *v2_input, barycentric_coordinates) {
                    M4 model = load_uniform_m4(pipeline, 0);
                    
                    V3 normal = internal_in_smooth_v3(pipeline, v0_inputs, v1_inputs, v2_inputs, barycentric_coordinates.a, barycentric_coordinates.b, barycentric_coordinates.c, 0);
                    V3 normal = in_smooth_v3(0);
                }
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


u64 get_pattern_offset(Shader_Value_Type_Array *pattern, u64 input_index) {
    u64 result = 0;
    for (u64 index = 0; index < input_index; ++index) {
        Shader_Value_Type type = pattern->items[index];
        u64 size = get_shader_value_type_size(type);
        result += size;
    }
    
    return result;
}

void add_attribute(Shader_Pipeline *pipeline, u8 *data, Shader_Value_Type type, u64 count) {
    Vertex_Attribute result = {};
    result.type = type;
    result.count = count;
    result.data = data;
    
    da_append(pipeline->attributes, result);
}
    
void add_uniform(Shader_Pipeline *pipeline, u8 *data, Shader_Value_Type type) {
    Shader_Uniform result = {};
    result.type = type;
    result.data = data;
    
    da_append(pipeline->uniforms, result);
};    

Shader_Pipeline create_shader_pipeline(Arena *clipping_arena, Vertex_Shader vertex_shader, Fragment_Shader fragment_shader) {
    Shader_Pipeline result = {};
    da_init(result.uniforms);
    da_init(result.attributes);
    da_init(result.output_pattern);
    result.clipping_arena = clipping_arena;
    result.vertex_shader = vertex_shader;
    result.fragment_shader = fragment_shader;
    
    Shader_Value_Type v4_type = Shader_Value_Type_V4;
    da_append(result.output_pattern, v4_type);
    
    return result;
};

// todo: for now we don't have ebo implemented, but when we do we want to change this
V3u get_vertex_indices(Shader_Pipeline *pipeline, u64 triangle_index) {
    V3u result = {};
    u64 index = triangle_index * 3;
    result.v0 = index;
    result.v1 = index + 1;
    result.v2 = index + 2;
    
    return result;
}

Clip_Triangle get_triangle(Shader_Pipeline *pipeline, V3u triangle_indices) {
    Clip_Triangle result = {};
    u64 offset = get_shader_value_type_array_size(pipeline->output_pattern);
    
    result.v0 = *(V4*)(pipeline->gpu_arena->data + offset * triangle_indices.v0);
    result.v1 = *(V4*)(pipeline->gpu_arena->data + offset * triangle_indices.v1);
    result.v2 = *(V4*)(pipeline->gpu_arena->data + offset * triangle_indices.v2);
    
    return result;
}

// todo: some form of error handling or something like that
// note: consider manually inlining all of this
f32 load_attribute_f32(Shader_Pipeline *pipeline, u64 vertex_index, u64 attribute_index) {
    f32 result = 0.0f;
    result = *((f32*)pipeline->attributes.items[attribute_index].data + vertex_index);
    
    return result;
}

V2 load_attribute_v2(Shader_Pipeline *pipeline, u64 vertex_index, u64 attribute_index) {
    V2 result = {};
    result = *((V2*)pipeline->attributes.items[attribute_index].data + vertex_index);
    
    return result;
}

V3 load_attribute_v3(Shader_Pipeline *pipeline, u64 vertex_index, u64 attribute_index) {
    V3 result = {};
    result = *((V3*)pipeline->attributes.items[attribute_index].data + vertex_index);
    
    return result;
}

V4 load_attribute_v4(Shader_Pipeline *pipeline, u64 vertex_index, u64 attribute_index) {
    V3 result = {};
    result = *((V4*)pipeline->attributes.items[attribute_index].data + vertex_index);
    
    return result;
}

M3 load_attribute_m3(Shader_Pipeline *pipeline, u64 vertex_index, u64 attribute_index) {
    V3 result = {};
    result = *((M3*)pipeline->attributes.items[attribute_index].data + vertex_index);
    
    return result;
}

M4 load_attribute_m4(Shader_Pipeline *pipeline, u64 vertex_index, u64 attribute_index) {
    V3 result = {};
    result = *((M4*)pipeline->attributes.items[attribute_index].data + vertex_index);
    
    return result;
}

// todo: check if this shit even works lol
f32 load_uniform_f32(Shader_Pipeline *pipeline, u64 uniform_index) {
    f32 result = 0.0f;
    result = *((f32*)pipeline->uniforms.items[attribute_index].data);
    
    return result;
};

V2 load_uniform_v2(Shader_Pipeline *pipeline, u64 uniform_index) {
    V2 result = 0.0f;
    result = *((V2*)pipeline->uniforms.items[attribute_index].data);
    
    return result;
};

V3 load_uniform_v3(Shader_Pipeline *pipeline, u64 uniform_index) {
    V3 result = 0.0f;
    result = *((V3*)pipeline->uniforms.items[attribute_index].data);
    
    return result;
};

V4 load_uniform_v4(Shader_Pipeline *pipeline, u64 uniform_index) {
    V4 result = 0.0f;
    result = *((V4*)pipeline->uniforms.items[attribute_index].data);
    
    return result;
};

M3 load_uniform_m3(Shader_Pipeline *pipeline, u64 uniform_index) {
    V2 result = 0.0f;
    result = *((M3*)pipeline->uniforms.items[attribute_index].data);
    
    return result;
};

M4 load_uniform_m4(Shader_Pipeline *pipeline, u64 uniform_index) {
    M4 result = 0.0f;
    result = *((M4*)pipeline->uniforms.items[attribute_index].data);
    
    return result;
};

// todo: i think now that we've clipped we can just multiply by barycentric coordinates
f32 internal_in_smooth_f32(Shader_Pipeline *pipeline, u8 *v0_ptr, u8 *v1_ptr, u8 *v2_ptr, f32 v0_weight, f32 v1_weight, f32 v2_weight, u64 input_index) {
    f32 result = 0.0f; 
    f32 v0 = in_f32(pipeline, v0_ptr, input_index);
    f32 v1 = in_f32(pipeline, v1_ptr, input_index);
    f32 v2 = in_f32(pipeline, v2_ptr, input_index);
    
    result = v0_weight * v0 + v1_weight * v1 + v2_weight * v2;
    return result;
}

V2 internal_in_smooth_v2(Shader_Pipeline *pipeline, u8 *v0_ptr, u8 *v1_ptr, u8 *v2_ptr, f32 v0_weight, f32 v1_weight, f32 v2_weight, u64 input_index) {
    V2 result = {};
    V2 v0 = in_v2(pipeline, v0_ptr, input_index);
    V2 v1 = in_v2(pipeline, v1_ptr, input_index);
    V2 v2 = in_v2(pipeline, v2_ptr, input_index);    
    
    result = v0_weight * v0 + v1_weight * v1 + v2_weight * v2;
    return result;
}

V3 internal_in_smooth_v3(Shader_Pipeline *pipeline, u8 *v0_ptr, u8 *v1_ptr, u8 *v2_ptr, f32 v0_weight, f32 v1_weight, f32 v2_weight, u64 input_index) {
    V3 result = {};
    V3 v0 = in_v3(pipeline, v0_ptr, input_index);
    V3 v1 = in_v3(pipeline, v1_ptr, input_index);
    V3 v2 = in_v3(pipeline, v2_ptr, input_index);    
    
    result = v0_weight * v0 + v1_weight * v1 + v2_weight * v2;
    return result;
}

V4 internal_in_smooth_v4(Shader_Pipeline *pipeline, u8 *v0_ptr, u8 *v1_ptr, u8 *v2_ptr, f32 v0_weight, f32 v1_weight, f32 v2_weight, u64 input_index) {
    V4 result = {};
    V4 v0 = in_v4(pipeline, v0_ptr, input_index);
    V4 v1 = in_v4(pipeline, v1_ptr, input_index);
    V4 v2 = in_v4(pipeline, v2_ptr, input_index);    
    
    result = v0_weight * v0 + v1_weight * v1 + v2_weight * v2;
    return result;
}

// todo: check that this actually works, and see if simpler way to do it
f32 internal_in_flat_f32(Shader_Pipeline *pipeline, u8 *input_ptr, u64 input_index) {
    f32 result = 0.0f;
    u64 pattern_offset = get_pattern_offset(pipeline->output_pattern, input_index);
    u8 *value_ptr = input_ptr + pattern_offset;
    result = *(f32*)value_ptr;
    
    return result;
}

V2 internal_in_flat_v2(Shader_Pipeline *pipeline, u8 *input_ptr, u64 input_index) {
    V2 result = {};
    u64 pattern_offset = get_pattern_offset(pipeline->output_pattern, input_index);
    u8 *value_ptr = input_ptr + pattern_offset;
    result = *(V2*)value_ptr;
    
    return result;
}

V3 internal_in_flat_v3(Shader_Pipeline *pipeline, u8 *input_ptr, u64 input_index) {
    V3 result = {};
    u64 pattern_offset = get_pattern_offset(pipeline->output_pattern, input_index);
    u8 *value_ptr = input_ptr + pattern_offset;
    result = *(V3*)value_ptr;
    
    return result;
}

V4 internal_in_flat_v4(Shader_Pipeline *pipeline, u8 *input_ptr, u64 input_index) {
    V4 result = {};
    u64 pattern_offset = get_pattern_offset(pipeline->output_pattern, input_index);
    u8 *value_ptr = input_ptr + pattern_offset;
    result = *(V4*)value_ptr;
    
    return result;
}

M3 internal_in_flat_m3(Shader_Pipeline *pipeline, u8 *input_ptr, u64 input_index) {
    M3 result = {};
    u64 pattern_offset = get_pattern_offset(pipeline->output_pattern, input_index);
    u8 *value_ptr = input_ptr + pattern_offset;
    result = *(M3*)value_ptr;
    
    return result;
}

M4 internal_in_flat_m4(Shader_Pipeline *pipeline, u8 *input_ptr, u64 input_index) {
    M4 result = {};
    u64 pattern_offset = get_pattern_offset(pipeline->output_pattern, input_index);
    u8 *value_ptr = input_ptr + pattern_offset;
    result = *(M4*)value_ptr;
    
    return result;
}

void internal_fragment_shader_output(u8 *result_ptr, V4 color_result) {
    V4 *color = (V4*)output;
    *color = result;
}

FRAGMENT_SHADER_FUNCTION diffuse(FRAGMENT_SHADER_PARAMETERS) {
    M4 model = uniform_m4(0);
    V3 normal = in_smooth_v3(0);
    
    V4 result = create_v4(normal) * create_v4(1.0f, 1.0f, 1.0f, 1.0f);
    
    fragment_shader_output(result);
}

