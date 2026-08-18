#include <stdio.h>
#include "util.h"
#include "render.h"
#include "engine.h"

int main(int argc, char **argv) {
    init_engine();
    
    V3 v0 = create_v3(-0.5, 0.0f, 0.04);
    V3 v1 = create_v3(0.0, 0.5f, 0.0f);
    V3 v2 = create_v3(0.5, 0.0f, 0.0f);
    V3 v3 = create_v3(-1.0f, 0.0f, 1.0f);
    
    Color8 c0 = color8(0xFF0000FF);
    Color8 c1 = color8(0x00FF00FF);
    Color8 c2 = color8(0x0000FFFF);
    
    Triangle triangle = create_triangle(v0, v1, v2, c0, c1, c2);
    Triangle triangle2 = create_triangle(v0, v1, v3, c1, c2, c0);
    Render_Entity entity = create_triangle_render_entity(triangle);
    Render_Entity entity2 = create_triangle_render_entity(triangle2);
    entity.pos.z += 3.0f;
    entity.scale = create_v3(5.0f);
    
    entity2.pos.z += 3.0f;
    entity2.scale = create_v3(5.0f);
    while (engine.running) {
        update_delta_time();
        poll_sdl_events();
        
        update_camera(&engine.camera);
        
        f32 fps = 1.0f / engine.delta_time;
        system("cls");
        printf("fps: %f\n", fps);
        V3 rot = engine.camera.rotation;
        V3 pos = engine.camera.pos;
        printf("rotation: pitch: %f | yaw: %f | roll: %f\n", rot.pitch, rot.yaw, rot.roll);
        printf("position: x: %f | y: %f | z: %f\n", pos.x, pos.y, pos.z);
        
        clear_screen(0x0);
        
        draw_render_entity(&engine.frame_buffer, entity);
        draw_render_entity(&engine.frame_buffer, entity2);
        
        present_screen();
                
        update_previous_time();
    }
    
    return 0;
}