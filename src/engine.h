#pragma once

#include <SDL3/SDL.h>
#include <memory.h>
#include <assert.h>

#include "util.h"
#include "engine_math.h"

#define DEFAULT_WINDOW_WIDTH 1000
#define DEFAULT_WINDOW_HEIGHT 800

#define DEFAULT_FOV 60.0f

struct Frame_Buffer {
    u32 *color_buffer;
    f32 *depth_buffer;
    u32 width;
    u32 height;
};

struct Camera {
    v3f pos;
    v3f rotation;
    v3f up;
    f32 fov;
    f32 sensitivity;
    f32 speed; // m/s
    f32 speed_boost;

    f32 far_plane;
    f32 near_plane;
};

Frame_Buffer frame_buffer_init(u32 width, u32 height);

inline u32 get_frame_buffer_byte_count(Frame_Buffer *src) {
    u32 size = sizeof(src->width);
    u32 result = src->width * src->height * size;
    return result;
}

inline u32 get_frame_buffer_pixel_count(Frame_Buffer *src) {
    u32 result = src->width * src->height;
    return result;
}

inline void clear_frame_buffer(Frame_Buffer *src, u8 color) {
    memset(src->color_buffer, 0, get_frame_buffer_byte_count(src));
    memset(src->depth_buffer, 0, get_frame_buffer_byte_count(src));
}

struct Engine {
    f32 current_time;
    f32 previous_time;
    f32 delta_time;
    struct Window {
        SDL_Window *handle;
        u32 width;
        u32 height;
    } window;
    struct Mouse {
        v2f pos;
        v2f old_pos;
        v2f delta_pos;
        b32 inside_window;
    } mouse;
    Camera camera;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    Frame_Buffer frame_buffer;
    bool running;
    bool paused;
    bool should_pause_at_rendering;
};

extern Engine engine;

s32 init_engine();
inline void stop_engine() {
    engine.running = false;
}

inline f32 get_window_width() {
    f32 result = engine.window.width;
    return result;
}

inline f32 get_window_height() {
    f32 result = engine.window.height;
    return result;
}

Camera camera_init();

inline f32 get_camera_fov() {
    f32 result = engine.camera.fov;
    return result;
}

void poll_sdl_events();
void clear_screen(u32 color);
void present_screen();

inline void update_delta_time() {
    engine.current_time = ((f64) SDL_GetTicks()) / 1000.0f;
    engine.delta_time = engine.current_time - engine.previous_time;
}

inline void update_previous_time() {
    engine.previous_time = engine.current_time;
}

inline v2f get_delta_mouse_pos() {
    v2f result;
    result = engine.mouse.delta_pos;

    return result;
}

void update_camera(Camera *camera);

void move_camera(Camera *camera, v3f pos, v3f at, v3f up);
void rotate_camera_by_mouse(Camera *camera);
void rotate_camera(Camera *camera, v3f rotation);
