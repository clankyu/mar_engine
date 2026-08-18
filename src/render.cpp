#include "render.h"
#include "engine.h"
#include "engine_math.h"

#include <stdio.h>
#include <math.h>

void draw_render_entity(Frame_Buffer *frame_buffer, Render_Entity e) {
    // model to world space
    M4 scale = m4_scale(e.scale);
    M4 rotation = m4_rotate(e.rotation);
    M4 translation = m4_translate(e.pos);
    M4 model = translation * rotation * scale;

    // world space to view space
    V3 from = engine.camera.pos;
    V3 direction = v3_norm(v3_lookat(engine.camera.rotation));
    V3 to = direction + from;
    V3 up = create_v3(0.0f, 1.0f, 0.0f);

    // I think this is it? I think the dot products measure the translation
    // something to do with projection
    M4 view = m4_transpose(m4_lookat(from, to, up));

    // view space to clip space
    M4 projection = m4_perspective(
                                   (f32)engine.frame_buffer.width, (f32)engine.frame_buffer.height, engine.camera.fov,
                                   engine.camera.near_plane, engine.camera.far_plane
                                  );

    M4 mvp = projection * view * model;
    
    // gotta find a way to take into account depth so i don't draw over stuff that shouldn't
    for (u32 triangle_index = 0; triangle_index < e.triangle_count; ++triangle_index) {
        Triangle triangle = e.triangles[triangle_index];

        Clip_Triangle clip_triangle = create_clip_triangle_mvp(triangle, mvp);
        clip_and_draw_triangle(&engine.frame_buffer, clip_triangle);
    }
}

void clip_and_draw_triangle(Frame_Buffer *frame_buffer, Clip_Triangle triangle) {
    if (triangle_in_view_frustum(triangle)) {
        V4 v0_pos = triangle.v0.pos;
        V4 v1_pos = triangle.v1.pos;
        V4 v2_pos = triangle.v2.pos;
        
        Vertex4d v0 = triangle.v0;
        Vertex4d v1 = triangle.v1;
        Vertex4d v2 = triangle.v2;
        
        // We only really have to clip against the near plane to avoid division by 0 when getting to ndc space
        // We don't have to worry about the x and y axis since they get clipped at rasterization stage             
        // Also I worry that by shuffling the order we fuck up the rasterization stage, might need to rearrange vertices
        
        assert(v0_pos.w != 0.0f || v1_pos.w != 0.0f || v2_pos.w != 0.0f);
        // Im pretty sure this is correct and coherent with the math, since 0 <= zclip / wclip <= 1, with 1 being near plane
        if (v0_pos.z > v0_pos.w) {
            if (v1_pos.z > v1_pos.w) {
                clip_and_draw_two_out(frame_buffer, v0, v1, v2); 
            } else if (v2_pos.z > v2_pos.w) {
                clip_and_draw_two_out(frame_buffer, v0, v2, v1);
            } else {
                clip_and_draw_one_out(frame_buffer, v0, v1, v2);
            }
        } else if (v1_pos.z > v1_pos.w) {
            if (v2_pos.z > v2_pos.w) {
                clip_and_draw_two_out(frame_buffer, v1, v2, v0);
            } else {
                clip_and_draw_one_out(frame_buffer, v1, v0, v2);
            }
        } else if (v2_pos.z > v2_pos.w) {
            clip_and_draw_one_out(frame_buffer, v2, v0, v1);
        } else {
            // for the moment being no more clipping necessary
            draw_triangle(frame_buffer, triangle);
        }
    }
}

// todo: implement float to integer rounding for relevant values
void draw_triangle(Frame_Buffer *frame_buffer, Clip_Triangle clip_triangle) {
    f32 width = engine.window.width;
    f32 height = engine.window.height;
    f32 half_width = width * 0.5f;
    f32 half_height = height * 0.5f;
    
    // x and y range from [-1, 1] and z [1, 0] (1 being exactly on the near plane and 0 on the far plane)
    V4 v0_ndc = clip_triangle.v0.pos / clip_triangle.v0.pos.w;
    V4 v1_ndc = clip_triangle.v1.pos / clip_triangle.v1.pos.w;
    V4 v2_ndc = clip_triangle.v2.pos / clip_triangle.v2.pos.w;
    
    Colorf c0 = color8_to_colorf(clip_triangle.v0.color) / clip_triangle.v0.pos.w;
    Colorf c1 = color8_to_colorf(clip_triangle.v1.color) / clip_triangle.v1.pos.w;
    Colorf c2 = color8_to_colorf(clip_triangle.v2.color) / clip_triangle.v2.pos.w;

    // note: we actually use this for calculations and final screen stuff
    Raster_Vertex v0 = ndc_to_raster(v0_ndc, clip_triangle.v0.pos.w, c0, width, height);
    Raster_Vertex v1 = ndc_to_raster(v1_ndc, clip_triangle.v1.pos.w, c1, width, height);
    Raster_Vertex v2 = ndc_to_raster(v2_ndc, clip_triangle.v2.pos.w, c2, width, height);
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
            // todo: check that this won't fuck up things, i think it should work but this is just intuition
            b32 inside = (
                (w0 <= 0.0f && w1 <= 0.0f && w2 <= 0.0f)
                || (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f)
            );

            if (inside) {
                // bug spotted in this, i was using alpha and the ws in order, but it was wrong, remember how the weights are calculated  
                f32 v0_weight = w1 / total_area;
                f32 v1_weight = w2 / total_area;
                f32 v2_weight = w0 / total_area;

                f32 r = v0.color.r * v0_weight + v1.color.r * v1_weight + v2.color.r * v2_weight;
                f32 g = v0.color.g * v0_weight + v1.color.g * v1_weight + v2.color.g * v2_weight;
                f32 b = v0.color.b * v0_weight + v1.color.b * v1_weight + v2.color.b * v2_weight;
                f32 a = v0.color.a * v0_weight + v1.color.a * v1_weight + v2.color.a * v2_weight;

                f32 interpolated_z = 1.0f / (v0.one_over_w * v0_weight + v1.one_over_w * v1_weight + v2.one_over_w * v2_weight);

                r *= interpolated_z * 255.0f;
                g *= interpolated_z * 255.0f;
                b *= interpolated_z * 255.0f;
                a *= interpolated_z * 255.0f;

                u32 rint = round(r);
                u32 gint = round(g);
                u32 bint = round(b);
                u32 aint = round(a);

                u32 color = color_bytes_to_u32(rint, gint, bint, aint);

                // todo: only render pixels that can actually be seen and not behind or too far
                // todo: actually interact with the depth buffer
                put_pixel(frame_buffer, x, y, color);
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

Render_Entity create_triangle_render_entity(Triangle triangle) {
    Render_Entity result = {};

    // this is obviously bullshit, mostly just for experimenting, later use some
    // sort of arena or buffer to store small stuff
    result.triangles = (Triangle *)malloc(sizeof(Triangle));
    result.triangles[0] = triangle;
    result.triangle_count = 1;
    result.scale = create_v3(1.0f);

    return result;
}

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

Clip_Triangle create_clip_triangle_mvp(Triangle triangle, M4 mvp) {
    Clip_Triangle result;

    V4 v0 = v3_to_v4(triangle.v0); v0.w = 1.0f;
    V4 v1 = v3_to_v4(triangle.v1); v1.w = 1.0f;
    V4 v2 = v3_to_v4(triangle.v2); v2.w = 1.0f;

    result.v0.pos = mvp * v0;
    result.v1.pos = mvp * v1;
    result.v2.pos = mvp * v2;
    result.v0.color = triangle.c0;
    result.v1.color = triangle.c1;
    result.v2.color = triangle.c2;

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

void clip_and_draw_one_out(Frame_Buffer *frame_buffer, Vertex4d out, Vertex4d in1, Vertex4d in2) {
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

void clip_and_draw_two_out(Frame_Buffer *frame_buffer, Vertex4d out1, Vertex4d out2, Vertex4d in) {
    f32 in_distance = in.pos.w - in.pos.z;
    f32 out1_distance = out1.pos.w - out1.pos.z;
    f32 out2_distance = out2.pos.w - out2.pos.z;
    
    f32 alpha0 = out1_distance / (out1_distance - in_distance);
    f32 alpha1 = out2_distance / (out2_distance - in_distance);
    
    Vertex4d a = vertex4d_lerp(out1, in, alpha0);
    Vertex4d b = vertex4d_lerp(out2, in, alpha1);
    
    Clip_Triangle triangle;
    triangle.v0 = in;
    triangle.v1 = a;
    triangle.v2 = b;
    
    draw_triangle(frame_buffer, triangle);
} 

// if more attributes come in the future might want to check out
Vertex4d vertex4d_lerp(Vertex4d a, Vertex4d b, f32 alpha) {
    Vertex4d result;
    result.pos = v4_lerp(a.pos, b.pos, alpha);
    result.color = color8_lerp(a.color, b.color, alpha);
    
    return result;
}