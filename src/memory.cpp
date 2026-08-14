#include <windows.h>
#include <memoryapi.h>
#include <assert.h>

#include "memory.h"
#include "util.h"

Arena arena_alloc(u64 size) {
    Arena result = {};
    result.data = (u8*) VirtualAlloc2(NULL, NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE, NULL, 0); // memory already set to zero and page aligned
    result.size = size;
    result.end = 0;
    
    return result;
}

void arena_touch_pages(Arena *arena) {
    u64 page_size = get_os_page_size();
    
    // note: i think this should work, volatile sus but i wanna make sure it gets read to allocate the pages
    for (u64 page_index = 0; page_index < arena->size / 4096; ++page_index) {
        volatile u8 read = arena->data[page_index];
    }
}

u64 get_os_page_size() {
    return 4096;
}

// for now if it doesn't fit we give 0 back. or maybe i should assert and force myself to make growable arenas
u8 *arena_push(Arena *arena, u64 size) {
    u8 *result = 0;
    u64 remaining_space = arena->size - arena->end;
    
    // note: idk if i should change the end, maybe i close off the arena
    if (remaining_space >= size) {
        result = arena->data + arena->end;
        arena->end += size;
    }
    
    return result;
}

u8 *arena_push_struct(Arena *arena, u64 size, u64 count) {
    u8 *result = 0;
    u64 total_size = size * count;
    u64 remaining_space = arena->size - arena->end;
    
    if (remaining_space >= total_size) {
        result = arena->data + arena->end;
        arena->end += total_size;
    }
    
    return result;
}

void arena_clear(Arena *arena) {
    if (arena->data != 0) {
        memset(arena->data, arena->size, 0);
        arena->end = 0;
    }
}

void arena_free(Arena *arena) {
    s32 result = VirtualFree(arena->data, 0, MEM_RELEASE);
    assert(result != 0); // for now
    *arena = {};
}

void memset(u8 *data, u64 size, u8 to) {
    if (data != 0) {
        for (u64 index = 0; index < size; ++index) {
            data[index] = to;
        } 
    }
}