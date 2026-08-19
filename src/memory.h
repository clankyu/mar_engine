#pragma once

#include "util.h"

#define KiB(count) (count * 1024)
#define MiB(count) (count * 1048576)
#define GiB(count) (count * 1073741824)

struct Arena {
    u64 size;
    u64 end;
    u8 *data;   
};

Arena arena_init(u64 size);
void arena_touch_pages(Arena *arena);
u64 get_os_page_size();
u8 *arena_push(Arena *arena, u64 size);
u8 *arena_push_struct(Arena *arena, u64 size, u64 count);
u8 *arena_get_end_ptr(Arena *arena);

void arena_clear(Arena *arena);
void arena_free(Arena *arena);

void memset(u8 *data, u64 size, u8 to);
void memcpy(u8 *dest, u8* src, u64 count);

struct {
    
};