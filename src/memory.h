#pragma once

#include <windows.h>
#include <memoryapi.h>
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

/* these macros only work for structs like these:
struct Foo_Array {
    Foo *items;
    u64 count;
    u64 capcity;
};
*/
#define da_init(da)\
do {\
    da.items = (__typeof__(da.items))VirtualAlloc2(NULL, NULL, GiB(1), MEM_RESERVE, PAGE_READWRITE, NULL, 0);\
    da.count = 0;\
    da.capacity = 0;\
} while(0)\

#define da_dealloc(da)\
do {\
    VirtualFree((u8*)da.items, 0, MEM_RELEASE);\
    da.count = 0;\
    da.capacity = 0;\
} while(0)\

#define da_append(da, item)\
do {\
    if (da.count >= da.capacity) {\
        u64 expanded_capacity = da.capacity;\
        u64 inital_capacity = da.capacity;\
        if (da.capacity == 0) {\
            da.capacity = 256;\
            expanded_capacity = 256;\
        } else {\
            da.capacity *= 2;\
        }\
        VirtualAlloc2(NULL, (u8*)da.items + inital_capacity*sizeof(*da.items), expanded_capacity*sizeof(*da.items), MEM_COMMIT, PAGE_READWRITE, NULL, 0);\
    }\
    da.items[da.count++] = item;\
} while(0)\

#define da_clear(da)\
do {\
    memset((u8*)da.items, da.capacity*sizeof(*da.items), 0);\
    da.count = 0;\
} while(0)\