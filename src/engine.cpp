#include <stdlib.h>
#include <memory.h>
#include "util.h"
#include "engine.h"
#include "memory.h"

Engine engine;

s32 init_engine() {
    engine = {};
    update_delta_time();
    update_previous_time();

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Raster", DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, 0, &engine.window.handle, &engine.renderer)) {
        SDL_Log("Couldn't create window and renderer: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_CENTER, "1");
    SDL_SetWindowRelativeMouseMode(engine.window.handle, true);

    engine.texture = SDL_CreateTexture(engine.renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
    engine.frame_buffer = frame_buffer_init(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
    engine.window.width = DEFAULT_WINDOW_WIDTH;
    engine.window.height = DEFAULT_WINDOW_HEIGHT;

    u32 mouse_state = SDL_GetMouseState(&engine.mouse.pos.x, &engine.mouse.pos.y);
    engine.mouse.pos.y *= -1.0f;
    engine.mouse.old_pos = engine.mouse.pos;
    engine.mouse.inside_window = false; // idk

    engine.camera = camera_init();
    engine.running = true;
    engine.paused = false;

    engine.shader_arena = arena_init(KiB(4));
    arena_touch_pages(&engine.shader_arena);
    
    return 0;
}

Camera camera_init() {
    Camera result = {};
    result.pos = {};
    result.rotation = create_v3f(0.0f, 0.0f, 0.0f);
    result.up = create_v3f(0.0f, 1.0f, 0.0f);
    result.near_plane = 0.5f;
    result.far_plane = 1000.0f;
    result.fov = DEFAULT_FOV;
    result.sensitivity = 30.0f;
    result.speed = 0.05;
    result.speed_boost = 2.0f;
    
    return result;
}

Frame_Buffer frame_buffer_init(u32 width, u32 height) {
    Frame_Buffer result = {};
    result.width = width;
    result.height = height;
    result.color_buffer = (u32*) malloc(width * height * sizeof(u32));
    result.depth_buffer = (f32*) malloc(width * height * sizeof(f32));

    return result;
}

void poll_sdl_events() {
    SDL_Event event;

    // mouse_state holds bitmask of pressed buttons on the mouse, check sdl docs
    engine.mouse.old_pos = engine.mouse.pos;
    u32 mouse_state = SDL_GetMouseState(&engine.mouse.pos.x, &engine.mouse.pos.y);
    engine.mouse.pos.y *= -1.0f;
    engine.mouse.delta_pos = {};
    
    while (SDL_PollEvent(&event)) {
        // Event stuff.

        if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
            stop_engine();
        }
        if (event.type == SDL_EVENT_QUIT) {
            engine.running = false;
        }
        if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
            engine.running = false;
        }
        if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            engine.window.width = event.window.data1;
            engine.window.height = event.window.data2;
        }
        if (event.key.scancode == SDL_SCANCODE_F1) {
            __debugbreak();
        }

        if (event.type == SDL_EVENT_MOUSE_MOTION) {
            f32 xrel = event.motion.xrel;
            f32 yrel = event.motion.yrel;
            yrel *= -1;

            engine.mouse.delta_pos = create_v2f(xrel, yrel);
        }
    }
}

void clear_screen(u32 color) {
    u8 r = color & 0xff;
    u8 g = (color >> 8) & 0xff;
    u8 b = (color >> 16) & 0xff;
    u8 a = (color >> 24) & 0xff;

    SDL_SetRenderDrawColor(engine.renderer, r, g, b, a);
    SDL_RenderClear(engine.renderer);
    clear_frame_buffer(&engine.frame_buffer, color);
}

void present_screen() {
    SDL_UpdateTexture(engine.texture, NULL, engine.frame_buffer.color_buffer, engine.window.width * sizeof(u32));
    SDL_RenderTexture(engine.renderer, engine.texture, NULL, NULL);
    SDL_RenderPresent(engine.renderer);
}

// im asuming you're giving a translation (already calculated velocity), but in reality
// it should probably be booleans or something that simply tell you if it moved,
// maybe it's a good idea to have a separate function to move the camera,
// (or maybe just here it's better, calculate translation from velocity outside)
void move_camera(Camera *camera, v3f translation, v3f at, v3f up) {
    v3f final_translation = {};
    v3f forward = v3f_norm(at);
    
    // this is so x+ is to the right
    v3f right = v3f_norm(v3f_cross(up, at));

    // multiplying probably isn't good
    f32 x_direction = translation.x < 0.0f ? -1.0f : 1.0f * (translation.x != 0);
    f32 y_direction = translation.y < 0.0f ? -1.0f : 1.0f * (translation.y != 0);
    f32 z_direction = translation.z < 0.0f ? -1.0f : 1.0f * (translation.z != 0);

    final_translation += x_direction * right;
    final_translation += y_direction * up;
    final_translation += z_direction * forward;
    
    camera->pos += final_translation;
}

void rotate_camera(Camera *camera, v3f delta_rotation) {
    f32 sensitivity = camera->sensitivity;

    camera->rotation += delta_rotation * sensitivity;
    camera->rotation.pitch = clamp(camera->rotation.pitch, -89.9f, 89.9f);
}

void update_camera(Camera *camera) {
    const bool *keyboard_state = SDL_GetKeyboardState(NULL);    
    f32 dt = engine.delta_time;
    
    // Camera rotation.
    v3f delta_camera_rotation = {};
    delta_camera_rotation.pitch += engine.mouse.delta_pos.y;
    delta_camera_rotation.yaw += engine.mouse.delta_pos.x;

    if (keyboard_state[SDL_SCANCODE_LEFT]) {
        delta_camera_rotation.yaw -= 10.0f;
    }
    if (keyboard_state[SDL_SCANCODE_RIGHT]) {
        delta_camera_rotation.yaw += 10.0f;
    }
    if (keyboard_state[SDL_SCANCODE_UP]) {
        delta_camera_rotation.pitch += 10.0f;
    }
    if (keyboard_state[SDL_SCANCODE_DOWN]) {
        delta_camera_rotation.pitch -= 10.0f;
    }

    delta_camera_rotation = delta_camera_rotation * dt;

    // Camera position.

    if (keyboard_state[SDL_SCANCODE_F2]) {
        engine.camera.rotation = {};
        engine.camera.pos = {};
    }

    v3f delta_camera_pos = {};
    f32 speed = engine.camera.speed;
    f32 speed_boost = 1.0f;
    if (keyboard_state[SDL_SCANCODE_W]) {
        delta_camera_pos.z += 1.0f;
    }
    if (keyboard_state[SDL_SCANCODE_S]) {
        delta_camera_pos.z -= 1.0f;
    }
    if (keyboard_state[SDL_SCANCODE_A]) {
        delta_camera_pos.x -= 1.0f;
    }
    if (keyboard_state[SDL_SCANCODE_D]) {
        delta_camera_pos.x += 1.0f;
    }
    if (keyboard_state[SDL_SCANCODE_SPACE]) {
        delta_camera_pos.y += 1.0f;
    }
    if (keyboard_state[SDL_SCANCODE_LCTRL]) {
        delta_camera_pos.y -= 1.0f;
    }
    if (keyboard_state[SDL_SCANCODE_LSHIFT]) {
        speed_boost = engine.camera.speed_boost;
    }
    
    delta_camera_pos = delta_camera_pos * speed * speed_boost * dt;

    v3f at = v3f_norm(v3f_lookat(engine.camera.rotation));
    v3f up = v3f_norm(create_v3f(0.0f, 1.0f, 0.0f));

    rotate_camera(&engine.camera, delta_camera_rotation);
    move_camera(&engine.camera, delta_camera_pos, at, up);
}