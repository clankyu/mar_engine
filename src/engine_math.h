#pragma once

#include <stdio.h>
#include <immintrin.h>
#include <xmmintrin.h>

#include "util.h"
#include <math.h>

#define PI 3.14159265359f
#define TWO_PI 6.28318530718f
#define HALF_PI 1.57079632679f

#define DEG_TO_RAD 0.01745329251f
#define RAD_TO_DEG 57.2957795131f

#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a > b) ? a : b)

// not sure if this is really optimal, check asm later
inline f32 deg_to_rad(f32 deg) {
    f32 result = deg * DEG_TO_RAD;
    return result;
}

inline f32 rad_to_deg(f32 rad) {
    f32 result = rad * RAD_TO_DEG;
    return result;
}

inline f32 clamp(f32 n, f32 min, f32 max) {
    f32 result = (n < min) ? min : n;
    result = (result > max) ? max : result;

    return result;
}

inline f32 lerp(f32 a, f32 b, f32 alpha) {
    f32 result = a + (b - a) * alpha;
    return result;
}

// todo: optimize
inline f32 abs(f32 n) {
    f32 result = n < 0.0f ? n * -1 : n;
    return result;
}

inline f32 floor(f32 n) {
    f32 result = (s32)n;
    f32 negative_factor = (n < 0.0f) ? 1.0f : 0.0f;
    result -= negative_factor;
    
    return result;
}

inline f32 ceil(f32 n) {
    f32 result = (s32)n;
    f32 should_add_one = (result != n && result > 0.0f) ? 1.0f : 0.0f;
    result += should_add_one;
    
    return result;
}

// very naive implementation, later use compiler instrisics for roundss instruction for x86
inline f32 round(f32 n) {
    f32 result = 0.0f;
    f32 n_int = (s32)n;

    if (n > 0.0f) {
        result = (n - n_int >= 0.5f) ? n_int + 1 : n_int;
    } else {
        result = (n - n_int <= -0.5f) ? n_int - 1 : n_int;
    }

    return result;
}

// todo: on hte whole file make it so you can make scalar operations on vector no matter the side
struct V2 {
    union {
        struct {
            f32 x;
            f32 y;
        };
        struct {
            f32 u;
            f32 v;
        };
        struct {
            f32 pitch;
            f32 yaw;
        };
    };
};

inline V2 operator+(V2 a, V2 b) {
    V2 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;

    return result;
}

inline V2& operator+=(V2& a, V2 b) {
    a = a + b;
    return a;
}

inline V2 operator+(V2 a, r32 b) {
    V2 result;
    result.x = a.x + b;
    result.y = a.y + b;

    return result;
}

inline V2 operator+(r32 b, V2 a) {
    V2 result = a + b;
    return result;
}

inline V2 operator-(V2 a, V2 b) {
    V2 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;

    return result;
}

inline V2& operator-=(V2& a, V2 b) {
    a = a - b;
    return a;
}

inline V2 operator-(V2 a) {
    V2 result;
    result.x = -a.x;
    result.y = -a.y;

    return result;
}

inline V2 operator-(V2 a, r32 b) {
    V2 result;
    result.x = a.x - b;
    result.y = a.y - b;

    return result;
}

inline V2 operator*(V2 a, V2 b) {
    V2 result;
    result.x = a.x * b.x;
    result.y = a.y * b.y;

    return result;
}

inline V2& operator*=(V2& a, V2 b) {
    a = a * b;
    return a;
}

inline V2 operator*(V2 a, r32 b) {
    V2 result;
    result.x = a.x * b;
    result.y = a.y * b;

    return result;
}

inline V2 operator/(V2 a, V2 b) {
    V2 result;
    result.x = a.x / b.x;
    result.y = a.y / b.y;

    return result;
}

inline V2& operator/=(V2& a, V2 b) {
    a = a / b;
    return a;
}

inline V2 operator/(V2 a, r32 b) {
    V2 result;
    result.x = a.x / b;
    result.y = a.y / b;

    return result;
}

inline V2 create_v2(f32 x, f32 y) {
    V2 result;
    result.x = x;
    result.y = y;

    return result;
}

inline V2 create_v2(f32 n) {
    V2 result;
    result.x = n;
    result.y = n;

    return result;
}

inline V2 create_v2() {
    V2 result = {};
    return result;
}

struct V3 {
    union {
        struct {
            f32 x;
            f32 y;
            f32 z;
        };
        struct {
            f32 pitch;
            f32 yaw;
            f32 roll;
        };
        struct {
            V2 xy;
            f32 __z;
        };
    };
};

inline V3 create_v3(f32 x, f32 y, f32 z) {
    V3 result;
    result.x = x;
    result.y = y;
    result.z = z;

    return result;
}

inline V3 create_v3(f32 n) {
    V3 result;
    result.x = n;
    result.y = n;
    result.z = n;

    return result;
}

inline V3 create_v3() {
    V3 result = {};
    return result;
}

inline V2 v3_to_v2(V3 v) {
    V2 result;
    result.x = v.x;
    result.y = v.y;

    return result;
}

inline V3 v2_to_v3(V2 v) {
    V3 result;
    result.x = v.x;
    result.y = v.y;

    return result;
}

inline f32 v3_dot(V3 a, V3 b) {
    f32 result = 0;
    result = a.x * b.x + a.y * b.y + a.z * b.z;
    return result;
}

inline V3 operator+(V3 a, V3 b) {
    V3 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;

    return result;
}

inline V3& operator+=(V3& a, V3 b) {
    a = a + b;
    return a;
}

inline V3 operator+(V3 a, r32 b) {
    V3 result;
    result.x = a.x + b;
    result.y = a.y + b;
    result.z = a.z + b;

    return result;
}

inline V3 operator-(V3 a, V3 b) {
    V3 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;

    return result;
}

inline V3& operator-=(V3& a, V3 b) {
    a = a - b;
    return a;
}

inline V3 operator-(V3 a) {
    V3 result;
    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;

    return result;
}

inline V3 operator-(V3 a, r32 b) {
    V3 result;
    result.x = a.x - b;
    result.y = a.y - b;
    result.z = a.z - b;

    return result;
}

inline V3 operator*(V3 a, V3 b) {
    V3 result;
    result.x = a.x * b.x;
    result.y = a.y * b.y;
    result.z = a.z * b.z;

    return result;
}

inline V3& operator*=(V3& a, V3 b) {
    a = a * b;
    return a;
}

inline V3 operator*(V3 a, r32 b) {
    V3 result;
    result.x = a.x * b;
    result.y = a.y * b;
    result.z = a.z * b;

    return result;
}

inline V3 operator*(r32 b, V3 a) {
    V3 result;
    result.x = a.x * b;
    result.y = a.y * b;
    result.z = a.z * b;

    return result;
}

inline V3 operator/(V3 a, V3 b) {
    V3 result;
    result.x = a.x / b.x;
    result.y = a.y / b.y;
    result.z = a.z / b.z;

    return result;
}

inline V3& operator/=(V3& a, V3 b) {
    a = a / b;
    return a;
}

inline V3 operator/(V3 a, r32 b) {
    V3 result;
    result.x = a.x / b;
    result.y = a.y / b;
    result.z = a.z / b;

    return result;
}

inline f32 v3_length(V3 v) {
    f32 result;
    result = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);

    return result;
}

inline V3 v3_norm(V3 v) {
    V3 result;
    f32 length = v3_length(v);
    result = v / length;

    return result;
}

inline V3 v3_cross(V3 a, V3 b) {
    V3 result;

    result.x = a.y * b.z - a.z * b.y;
    result.y = -(a.x * b.z) + a.z * b.x;
    result.z = a.x * b.y - a.y * b.x;

    return result;
}

inline V3 v3_lerp(V3 a, V3 b, f32 alpha) {
    V3 result;
    result = a + (b - a) * alpha;
    
    return result;
}

struct V4 {
    union {
        struct {
            f32 x;
            f32 y;
            f32 z;
            f32 w;
        };
        struct {
            f32 r;
            f32 g;
            f32 b;
            f32 a;
        };
        struct {
            V2 xy;
            V2 zw;
        };
    };
};

inline V4 create_v4(f32 x, f32 y, f32 z, f32 w) {
    V4 result;
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;

    return result;
}

inline V4 create_v4(f32 n) {
    V4 result;
    result.x = n;
    result.y = n;
    result.z = n;
    result.w = n;

    return result;
}

inline V4 create_v4() {
    V4 result = {};
    return result;
}

inline V4 v3_to_v4(V3 v) {
    V4 result;
    result.x = v.x;
    result.y = v.y;
    result.z = v.z;
    result.w = 0;

    return result;
}

inline V3 v4_to_v3(V4 v) {
    V3 result;
    result.x = v.x;
    result.y = v.y;
    result.z = v.z;

    return result;
}

inline V2 v4_to_v2(V4 v) {
    V2 result;
    result.x = v.x;
    result.y = v.y;

    return result;
}

inline V4 operator+(V4 a, V4 b) {
    V4 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    result.w = a.w + b.w;

    return result;
}

inline V4& operator+=(V4& a, V4 b) {
    a = a + b;
    return a;
}

inline V4 operator+(V4 a, r32 b) {
    V4 result;
    result.x = a.x + b;
    result.y = a.y + b;
    result.z = a.z + b;
    result.w = a.w + b;

    return result;
}

inline V4 operator-(V4 a, V4 b) {
    V4 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    result.w = a.w - b.w;

    return result;
}

inline V4& operator-=(V4& a, V4 b) {
    a = a - b;
    return a;
}

inline V4 operator-(V4 a) {
    V4 result;
    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;
    result.w = -a.w;

    return result;
}

inline V4 operator-(V4 a, r32 b) {
    V4 result;
    result.x = a.x - b;
    result.y = a.y - b;
    result.z = a.z - b;
    result.w = a.w - b;

    return result;
}

inline V4 operator*(V4 a, V4 b) {
    V4 result;
    result.x = a.x * b.x;
    result.y = a.y * b.y;
    result.z = a.z * b.z;
    result.w = a.w * b.w;

    return result;
}

inline V4& operator*=(V4& a, V4 b) {
    a = a * b;
    return a;
}

inline V4 operator*(V4 a, r32 b) {
    V4 result;
    result.x = a.x * b;
    result.y = a.y * b;
    result.z = a.z * b;
    result.w = a.w * b;

    return result;
}

inline V4 operator*(r32 b, V4 a) {
    V4 result = a * b;
    return result;
}

inline V4 operator/(V4 a, V4 b) {
    V4 result;
    result.x = a.x / b.x;
    result.y = a.y / b.y;
    result.z = a.z / b.z;
    result.w = a.w / b.w;

    return result;
}

inline V4& operator/=(V4& a, V4 b) {
    a = a / b;
    return a;
}

inline V4 operator/(V4 a, r32 b) {
    V4 result;
    result.x = a.x / b;
    result.y = a.y / b;
    result.z = a.z / b;
    result.w = a.w / b;

    return result;
}

// note: checks if it's to the right
inline f32 edge_function(V2 a, V2 b, V2 p) {
    f32 result = 0;
    result = (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x);

    return result;
}

inline b32 is_top_left(V2 start, V2 end) {
    b32 result = 0;
    V2 edge = end - start;
    b32 is_top_edge = edge.y == 0.0f;

    // normally i'd think it should be greater than (positive slope), but i think it's this way due
    // to the fact the current screen coordinates are down == positive, not entirely sure tho
    b32 is_top_left = edge.y > 0.0f;

    result = is_top_edge || is_top_left;
    return result;
}

inline V4 v4_lerp(V4 a, V4 b, f32 alpha) {
    V4 result;
    result = a + (b - a) * alpha;
    
    return result;
}

struct M2 {
    union {
        f32 e[2][2];
        f32 arr[4];
        struct {
            V2 x;
            V2 y;
        };
        struct {
            V2 col0;
            V2 col1;
        };
    };
};

inline M2 m2_identity() {
    M2 result = {};
    result.e[0][0] = 1.0f;
    result.e[1][1] = 1.0f;

    return result;
}

inline f32 m2_determinant(M2 mat) {
    f32 result = 0.0f;
    result = mat.e[0][0] * mat.e[1][1] - mat.e[0][1] * mat.e[1][0];

    return result;
}

struct M3 {
    union {
        f32 e[3][3];
        f32 arr[9];
        struct {
            V3 x;
            V3 y;
            V3 z;
        };
        struct {
            V3 col0;
            V3 col1;
            V3 col2;
        };
    };
};

inline M3 m3_identity() {
    M3 result = {};
    result.e[0][0] = 1.0f;
    result.e[1][1] = 1.0f;
    result.e[2][2] = 1.0f;

    return result;
}

inline M3 operator*(M3 a, M3 b) {
    M3 result = {};

    V3 acol0 = a.col0;
    V3 acol1 = a.col1;
    V3 acol2 = a.col2;

    V3 bcol0 = b.col0;
    V3 bcol1 = b.col1;
    V3 bcol2 = b.col2;

    result.col0 = bcol0.x * acol0 + bcol0.y * acol1 + bcol0.z * acol2;
    result.col1 = bcol1.x * acol0 + bcol1.y * acol1 + bcol1.z * acol2;
    result.col2 = bcol2.x * acol0 + bcol2.y * acol1 + bcol2.z * acol2;

    return result;
}

inline V3 operator*(V3 v, M3 mat) {
    V3 result;
    result = v.x * mat.col0 + v.y * mat.col1 + v.z * mat.col2;

    return result;
}

inline V3 operator*(M3 mat, V3 v) {
    V3 result;
    result = v.x * mat.col0 + v.y * mat.col1 + v.z * mat.col2;

    return result;
}

inline f32 m3_determinant(M3 mat) {
    f32 result = 0.0f;

    u32 base_row_index = 0;
    u32 moving_column_index = 0;

    u32 scalar_negative_factor = 1;

    // mat index is for the 3 2x2 matrices that we form from the original mat.
    for (u32 moving_column_index = 0; moving_column_index < 3; ++moving_column_index, scalar_negative_factor *= -1) {
        M2 selected_mat;

        // these indices are to choose the appropiate values, don't use them to index into the new matrix.
        u32 selected_row_index = 0;
        u32 selected_column_index = 0;

        f32 scalar = mat.e[base_row_index][moving_column_index];
        if (scalar == 0.0f) {
            continue;
        }

        // row_index and column_index are the indices of the actual new 2x2 matrix
        for (u32 row_index = 0; row_index < 2; ++row_index) {
            selected_row_index = row_index == base_row_index ? selected_row_index + 1 : selected_row_index; // this is sort of redunant since we start at row index 1

            for (u32 column_index = 0; column_index < 2; ++column_index) {
                selected_column_index += column_index == moving_column_index;
                selected_mat.e[column_index][row_index] = mat.e[selected_column_index][selected_row_index];
            }
        }

        f32 selected_mat_determinant = m2_determinant(selected_mat);
        result += scalar * selected_mat_determinant;
    }

    return result;
}

inline M3 m3_rotate_x(f32 deg) {
    M3 result = m3_identity();

    f32 angle = deg_to_rad(deg);

    f32 x_cos = cos(angle);
    f32 x_sin = sin(angle);

    result.e[0][0] = 1.0f;
    result.e[1][1] = x_cos;
    result.e[2][1] = -x_sin;
    result.e[1][2] = x_sin;
    result.e[2][2] = x_cos;

    return result;
}

inline M3 m3_rotate_y(f32 deg) {
    M3 result = m3_identity();

    f32 angle = deg_to_rad(deg);

    f32 y_cos = cos(angle);
    f32 y_sin = sin(angle);

    result.e[0][0] = y_cos;
    result.e[2][0] = y_sin;
    result.e[1][1] = 1.0f;
    result.e[0][2] = -y_sin;
    result.e[2][2] = y_cos;

    return result;
}

inline M3 m3_rotate_z(f32 deg) {
    M3 result = m3_identity();

    f32 angle = deg_to_rad(deg);

    f32 z_cos = cos(angle);
    f32 z_sin = sin(angle);

    result.e[0][0] = z_cos;
    result.e[1][0] = -z_sin;
    result.e[0][1] = z_sin;
    result.e[1][1] = z_cos;
    result.e[2][2] = 1.0f;

    return result;
}

inline M3 m3_rotate(V3 rot) {
    M3 result = m3_identity();

    M3 x = m3_rotate_x(rot.x);
    M3 y = m3_rotate_y(rot.y);
    M3 z = m3_rotate_z(rot.z);

    // note: yaw pitch then roll, this is supposedly standard for fps games, rotation order matters, quaternions
    // supposedly fix this, so i'll look into them probably once i want third person view
    result = z * x * y;

    return result;
}

struct M4 {
    union {
        // stored column major - e[column][row]
        f32 e[4][4];
        struct {
            V4 col0;
            V4 col1;
            V4 col2;
            V4 col3;
        };
        struct {
            f32 m00, m01, m02, m03,
            m10, m11, m12, m13,
            m20, m21, m22, m23,
            m30, m31, m32, m33;
        };
    };
};

inline V3 v3_lookat(V3 rotation) {
    V3 result;
    f32 pitch = deg_to_rad(rotation.pitch);
    f32 yaw = deg_to_rad(rotation.yaw);
    f32 roll = deg_to_rad(rotation.roll);

    result.x = cosf(pitch) * sinf(yaw);
    result.y = sinf(pitch); 
    result.z = cosf(pitch) * cosf(yaw);

    return result;
}

inline V4 operator*(V4 v, M4 mat) {
    V4 result;
    result = v.x * mat.col0 + v.y * mat.col1 + v.z * mat.col2 + v.w * mat.col3;

    return result;
}

inline V4 operator*(M4 mat, V4 v) {
    V4 result = v * mat;
    return result;
}

inline M4 operator*(M4 a, M4 b) {
    M4 result = {};

    V4 acol0 = a.col0;
    V4 acol1 = a.col1;
    V4 acol2 = a.col2;
    V4 acol3 = a.col3;

    V4 bcol0 = b.col0;
    V4 bcol1 = b.col1;
    V4 bcol2 = b.col2;
    V4 bcol3 = b.col3;

    result.col0 = bcol0.x * acol0 + bcol0.y * acol1 + bcol0.z * acol2 + bcol0.w * acol3;
    result.col1 = bcol1.x * acol0 + bcol1.y * acol1 + bcol1.z * acol2 + bcol1.w * acol3;
    result.col2 = bcol2.x * acol0 + bcol2.y * acol1 + bcol2.z * acol2 + bcol2.w * acol3;
    result.col3 = bcol3.x * acol0 + bcol3.y * acol1 + bcol3.z * acol2 + bcol3.w * acol3;

    return result;
}

inline M4 m4_identity() {
    M4 result = {};
    result.e[0][0] = 1.0f;
    result.e[1][1] = 1.0f;
    result.e[2][2] = 1.0f;
    result.e[3][3] = 1.0f;

    return result;
}

// this whole function is wrong i think? maybe they're swapping each other into being correct idk lol
inline M4 create_m4(f32 m00, f32 m01, f32 m02, f32 m03,
                      f32 m10, f32 m11, f32 m12, f32 m13,
                      f32 m20, f32 m21, f32 m22, f32 m23,
                      f32 m30, f32 m31, f32 m32, f32 m33)
{
    M4 result;
    result.e[0][0] = m00; result.e[1][0] = m10; result.e[2][0] = m20; result.e[3][0] = m30;
    result.e[0][1] = m01; result.e[1][1] = m11; result.e[2][1] = m21; result.e[3][1] = m31;
    result.e[0][2] = m02; result.e[1][2] = m12; result.e[2][2] = m22; result.e[3][2] = m32;
    result.e[0][3] = m03; result.e[1][3] = m13; result.e[2][3] = m23; result.e[3][3] = m33;

    return result;
}

inline f32 m4_determinant(M4 mat) {
    f32 result = 0.0f;

    u32 base_row_index = 0;
    u32 moving_column_index = 0;

    u32 scalar_negative_factor = 1;

    // mat index is for the 4 3x3 matrices that we form from the original mat.
    for (u32 moving_column_index = 0; moving_column_index < 4; ++moving_column_index, scalar_negative_factor *= -1) {
        M3 selected_mat;

        // these indices are to select the appropiate values, don't use them to index into the new matrix.
        u32 selected_row_index = 0;
        u32 selected_column_index = 0;

        f32 scalar = mat.e[base_row_index][moving_column_index];
        if (scalar == 0.0f) {
            continue;
        }

        // row_index and column_index are the indices of the actual new 3x3 matrix
        for (u32 row_index = 0; row_index < 3; ++row_index) {
            selected_row_index = row_index == base_row_index ? selected_row_index + 1 : selected_row_index; // this is sort of redunant since we start at row index 1

            for (u32 column_index = 0; column_index < 3; ++column_index) {
                selected_column_index += column_index == moving_column_index;
                selected_mat.e[column_index][row_index] = mat.e[selected_column_index][selected_row_index];
            }
        }

        f32 selected_mat_determinant = m3_determinant(selected_mat);
        result += scalar * selected_mat_determinant;
    }

    return result;
}

inline M4 m4_transpose(M4 mat) {
    M4 result;
    for (u32 row = 0; row < 4; ++row) {
        for (u32 column = 0; column < 4; ++column) {
            result.e[column][row] = mat.e[row][column];
        }
    }

    return result;
}

inline f32 m4_row_dot(M4 mat, u32 row, f32 n) {
    f32 result = 0.0f;
    result = (mat.e[0][row] * n) + (mat.e[1][row] * n) + (mat.e[2][row] * n) + (mat.e[3][row] * n);

    return result;
}

inline M4 m4_scale(V3 scale) {
    M4 result = m4_identity();
    result.e[0][0] = scale.x;
    result.e[1][1] = scale.y;
    result.e[2][2] = scale.z;

    return result;
}

inline M4 m4_rotate_x(f32 deg) {
    M4 result = m4_identity();

    f32 angle = deg_to_rad(deg);

    f32 x_cos = cos(angle);
    f32 x_sin = sin(angle);

    result.e[0][0] = 1.0f;
    result.e[1][1] = x_cos;
    result.e[2][1] = -x_sin;
    result.e[1][2] = x_sin;
    result.e[2][2] = x_cos;

    return result;
}

inline M4 m4_rotate_y(f32 deg) {
    M4 result = m4_identity();

    f32 angle = deg_to_rad(deg);

    f32 y_cos = cos(angle);
    f32 y_sin = sin(angle);

    result.e[0][0] = y_cos;
    result.e[2][0] = y_sin;
    result.e[1][1] = 1.0f;
    result.e[0][2] = -y_sin;
    result.e[2][2] = y_cos;

    return result;
}

inline M4 m4_rotate_z(f32 deg) {
    M4 result = m4_identity();

    f32 angle = deg_to_rad(deg);

    f32 z_cos = cos(angle);
    f32 z_sin = sin(angle);

    result.e[0][0] = z_cos;
    result.e[1][0] = -z_sin;
    result.e[0][1] = z_sin;
    result.e[1][1] = z_cos;
    result.e[2][2] = 1.0f;

    return result;
}

inline M4 m4_rotate(V3 rotation) {
    M4 result = m4_identity();

    M4 x = m4_rotate_x(rotation.x);
    M4 y = m4_rotate_y(rotation.y);
    M4 z = m4_rotate_z(rotation.z);

    // note: yaw pitch then roll, this is supposedly standard for fps games, rotation order matters, quaternions
    // supposedly fix this, so i'll look into them probably once i want third person view
    result = z * x * y;

    return result;
}

inline M4 m4_translate(V3 translation) {
    M4 result = m4_identity();
    result.e[3][0] = translation.x;
    result.e[3][1] = translation.y;
    result.e[3][2] = translation.z;

    return result;
}

inline M4 m4_inverse_translate(V3 translation) {
    M4 result = m4_identity();
    result.e[3][0] = -translation.x;
    result.e[3][1] = -translation.y;
    result.e[3][2] = -translation.z;

    return result;
}

inline M4 m4_inverse_affine(M4 mat) {
    M4 result = m4_identity();
    return result;
}

/*
We write xdot, ydot and zdot at the bottom part of the columns, normally this
would have to go at the last column for translation, but it is IMPORTANT to
remember that this matrix has to be transposed when using it for the camera,
since the transpose of the lookat matrix is the cameras rotation matrix, or
in other words, the inverse rotation, which causes the desired effect.

It's important to remember this only affects rotation (i think? i don't know what the dot products are for),
so it's important that when making the view matrix, you multiply with the inverse translation matrix.

I think the dot products represent the translation? I'm not sure, I need to look into this.
*/

inline M4 m4_lookat(V3 from, V3 to, V3 up) {
    M4 result = m4_identity();
    V3 z = v3_norm(to - from);
    V3 x = v3_norm(v3_cross(up, z));
    V3 y = v3_norm(v3_cross(z, x));

    f32 xdot = -v3_dot(from, x);
    f32 ydot = -v3_dot(from, y);
    f32 zdot = -v3_dot(from, z);

    result = create_m4(
                        x.x, x.y, x.z, xdot,
                        y.x, y.y, y.z, ydot,
                        z.x, z.y, z.z, zdot,
                        0.0f, 0.0f, 0.0f, 1.0f
                        );

    return result;
}

inline M4 m4_perspective(f32 width, f32 height, f32 fov, f32 near_plane, f32 far_plane) {
    M4 result = m4_identity();
    f32 angle = deg_to_rad(fov);
    f32 aspect_ratio = width / height;
    f32 scale = 1.0f / tan(angle * 0.5f);

    f32 m22 = near_plane / (near_plane - far_plane);
    f32 m32 = (far_plane * near_plane) / (far_plane - near_plane);

    /// normalize to [-1, 1] and scale appropiately for rectangular aspect ratios
    result.e[0][0] = scale / aspect_ratio;
    result.e[1][1] = scale;


    result.e[2][2] = m22;
    result.e[3][2] = m32;

    result.e[2][3] = 1.0f;
    
    result.e[3][3] = 0.0f;

    return result;
}
