#pragma once

#include "engine_math.h"
#include "memory.h"
#include "util.h"

// im probably fine with just these attributes, no need to make it too complicated
struct Obj_Data {
    V3 *v;
    V3 *vn;
    V3 *vt;
    V3 *f;
    V3 *p;
    u64 vertex_count;
};

Obj_Data load_obj(Arena *arena, char *file);