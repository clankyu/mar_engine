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
struct v2f {
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

inline v2f operator+(v2f a, v2f b) {
    v2f result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;

    return result;
}

inline v2f& operator+=(v2f& a, v2f b) {
    a = a + b;
    return a;
}

inline v2f operator+(v2f a, r32 b) {
    v2f result;
    result.x = a.x + b;
    result.y = a.y + b;

    return result;
}

inline v2f operator+(r32 b, v2f a) {
    v2f result = a + b;
    return result;
}

inline v2f operator-(v2f a, v2f b) {
    v2f result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;

    return result;
}

inline v2f& operator-=(v2f& a, v2f b) {
    a = a - b;
    return a;
}

inline v2f operator-(v2f a) {
    v2f result;
    result.x = -a.x;
    result.y = -a.y;

    return result;
}

inline v2f operator-(v2f a, r32 b) {
    v2f result;
    result.x = a.x - b;
    result.y = a.y - b;

    return result;
}

inline v2f operator*(v2f a, v2f b) {
    v2f result;
    result.x = a.x * b.x;
    result.y = a.y * b.y;

    return result;
}

inline v2f& operator*=(v2f& a, v2f b) {
    a = a * b;
    return a;
}

inline v2f operator*(v2f a, r32 b) {
    v2f result;
    result.x = a.x * b;
    result.y = a.y * b;

    return result;
}

inline v2f operator/(v2f a, v2f b) {
    v2f result;
    result.x = a.x / b.x;
    result.y = a.y / b.y;

    return result;
}

inline v2f& operator/=(v2f& a, v2f b) {
    a = a / b;
    return a;
}

inline v2f operator/(v2f a, r32 b) {
    v2f result;
    result.x = a.x / b;
    result.y = a.y / b;

    return result;
}

inline v2f create_v2f(f32 x, f32 y) {
    v2f result;
    result.x = x;
    result.y = y;

    return result;
}

inline v2f create_v2f(f32 n) {
    v2f result;
    result.x = n;
    result.y = n;

    return result;
}

inline v2f create_v2f() {
    v2f result = {};
    return result;
}

struct v3f {
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
            v2f xy;
            f32 __z;
        };
    };
};

inline v3f create_v3f(f32 x, f32 y, f32 z) {
    v3f result;
    result.x = x;
    result.y = y;
    result.z = z;

    return result;
}

inline v3f create_v3f(f32 n) {
    v3f result;
    result.x = n;
    result.y = n;
    result.z = n;

    return result;
}

inline v3f create_v3f() {
    v3f result = {};
    return result;
}

inline v2f v3f_to_v2f(v3f v) {
    v2f result;
    result.x = v.x;
    result.y = v.y;

    return result;
}

inline v3f v2f_to_v3f(v2f v) {
    v3f result;
    result.x = v.x;
    result.y = v.y;

    return result;
}

inline f32 v3f_dot(v3f a, v3f b) {
    f32 result = 0;
    result = a.x * b.x + a.y * b.y + a.z * b.z;
    return result;
}

inline v3f operator+(v3f a, v3f b) {
    v3f result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;

    return result;
}

inline v3f& operator+=(v3f& a, v3f b) {
    a = a + b;
    return a;
}

inline v3f operator+(v3f a, r32 b) {
    v3f result;
    result.x = a.x + b;
    result.y = a.y + b;
    result.z = a.z + b;

    return result;
}

inline v3f operator-(v3f a, v3f b) {
    v3f result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;

    return result;
}

inline v3f& operator-=(v3f& a, v3f b) {
    a = a - b;
    return a;
}

inline v3f operator-(v3f a) {
    v3f result;
    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;

    return result;
}

inline v3f operator-(v3f a, r32 b) {
    v3f result;
    result.x = a.x - b;
    result.y = a.y - b;
    result.z = a.z - b;

    return result;
}

inline v3f operator*(v3f a, v3f b) {
    v3f result;
    result.x = a.x * b.x;
    result.y = a.y * b.y;
    result.z = a.z * b.z;

    return result;
}

inline v3f& operator*=(v3f& a, v3f b) {
    a = a * b;
    return a;
}

inline v3f operator*(v3f a, r32 b) {
    v3f result;
    result.x = a.x * b;
    result.y = a.y * b;
    result.z = a.z * b;

    return result;
}

inline v3f operator*(r32 b, v3f a) {
    v3f result;
    result.x = a.x * b;
    result.y = a.y * b;
    result.z = a.z * b;

    return result;
}

inline v3f operator/(v3f a, v3f b) {
    v3f result;
    result.x = a.x / b.x;
    result.y = a.y / b.y;
    result.z = a.z / b.z;

    return result;
}

inline v3f& operator/=(v3f& a, v3f b) {
    a = a / b;
    return a;
}

inline v3f operator/(v3f a, r32 b) {
    v3f result;
    result.x = a.x / b;
    result.y = a.y / b;
    result.z = a.z / b;

    return result;
}

inline f32 v3f_length(v3f v) {
    f32 result;
    result = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);

    return result;
}

inline v3f v3f_norm(v3f v) {
    v3f result;
    f32 length = v3f_length(v);
    result = v / length;

    return result;
}

inline v3f v3f_cross(v3f a, v3f b) {
    v3f result;

    result.x = a.y * b.z - a.z * b.y;
    result.y = -(a.x * b.z) + a.z * b.x;
    result.z = a.x * b.y - a.y * b.x;

    return result;
}

inline v3f v3f_lerp(v3f a, v3f b, f32 alpha) {
    v3f result;
    result = a + (b - a) * alpha;
    
    return result;
}

struct v4f {
    union {
        struct {
            f32 x;
            f32 y;
            f32 z;
            f32 w;
        };
        struct {
            v2f xy;
            v2f zw;
        };
    };
};

inline v4f create_v4f(f32 x, f32 y, f32 z, f32 w) {
    v4f result;
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;

    return result;
}

inline v4f create_v4f(f32 n) {
    v4f result;
    result.x = n;
    result.y = n;
    result.z = n;
    result.w = n;

    return result;
}

inline v4f create_v4f() {
    v4f result = {};
    return result;
}

inline v4f v3f_to_v4f(v3f v) {
    v4f result;
    result.x = v.x;
    result.y = v.y;
    result.z = v.z;
    result.w = 0;

    return result;
}

inline v3f v4f_to_v3f(v4f v) {
    v3f result;
    result.x = v.x;
    result.y = v.y;
    result.z = v.z;

    return result;
}

inline v2f v4f_to_v2f(v4f v) {
    v2f result;
    result.x = v.x;
    result.y = v.y;

    return result;
}

inline v4f operator+(v4f a, v4f b) {
    v4f result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    result.w = a.w + b.w;

    return result;
}

inline v4f& operator+=(v4f& a, v4f b) {
    a = a + b;
    return a;
}

inline v4f operator+(v4f a, r32 b) {
    v4f result;
    result.x = a.x + b;
    result.y = a.y + b;
    result.z = a.z + b;
    result.w = a.w + b;

    return result;
}

inline v4f operator-(v4f a, v4f b) {
    v4f result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    result.w = a.w - b.w;

    return result;
}

inline v4f& operator-=(v4f& a, v4f b) {
    a = a - b;
    return a;
}

inline v4f operator-(v4f a) {
    v4f result;
    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;
    result.w = -a.w;

    return result;
}

inline v4f operator-(v4f a, r32 b) {
    v4f result;
    result.x = a.x - b;
    result.y = a.y - b;
    result.z = a.z - b;
    result.w = a.w - b;

    return result;
}

inline v4f operator*(v4f a, v4f b) {
    v4f result;
    result.x = a.x * b.x;
    result.y = a.y * b.y;
    result.z = a.z * b.z;
    result.w = a.w * b.w;

    return result;
}

inline v4f& operator*=(v4f& a, v4f b) {
    a = a * b;
    return a;
}

inline v4f operator*(v4f a, r32 b) {
    v4f result;
    result.x = a.x * b;
    result.y = a.y * b;
    result.z = a.z * b;
    result.w = a.w * b;

    return result;
}

inline v4f operator*(r32 b, v4f a) {
    v4f result = a * b;
    return result;
}

inline v4f operator/(v4f a, v4f b) {
    v4f result;
    result.x = a.x / b.x;
    result.y = a.y / b.y;
    result.z = a.z / b.z;
    result.w = a.w / b.w;

    return result;
}

inline v4f& operator/=(v4f& a, v4f b) {
    a = a / b;
    return a;
}

inline v4f operator/(v4f a, r32 b) {
    v4f result;
    result.x = a.x / b;
    result.y = a.y / b;
    result.z = a.z / b;
    result.w = a.w / b;

    return result;
}

// note: checks if it's to the right
inline f32 edge_function(v2f a, v2f b, v2f p) {
    f32 result = 0;
    result = (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x);

    return result;
}

inline b32 is_top_left(v2f start, v2f end) {
    b32 result = 0;
    v2f edge = end - start;
    b32 is_top_edge = edge.y == 0.0f;

    // normally i'd think it should be greater than (positive slope), but i think it's this way due
    // to the fact the current screen coordinates are down == positive, not entirely sure tho
    b32 is_top_left = edge.y > 0.0f;

    result = is_top_edge || is_top_left;
    return result;
}

inline v4f v4f_lerp(v4f a, v4f b, f32 alpha) {
    v4f result;
    result = a + (b - a) * alpha;
    
    return result;
}

struct m2f {
    union {
        f32 e[2][2];
        f32 arr[4];
        struct {
            v2f x;
            v2f y;
        };
        struct {
            v2f col0;
            v2f col1;
        };
    };
};

inline m2f m2f_identity() {
    m2f result = {};
    result.e[0][0] = 1.0f;
    result.e[1][1] = 1.0f;

    return result;
}

inline f32 m2f_determinant(m2f mat) {
    f32 result = 0.0f;
    result = mat.e[0][0] * mat.e[1][1] - mat.e[0][1] * mat.e[1][0];

    return result;
}

struct m3f {
    union {
        f32 e[3][3];
        f32 arr[9];
        struct {
            v3f x;
            v3f y;
            v3f z;
        };
        struct {
            v3f col0;
            v3f col1;
            v3f col2;
        };
    };
};

inline m3f m3f_identity() {
    m3f result = {};
    result.e[0][0] = 1.0f;
    result.e[1][1] = 1.0f;
    result.e[2][2] = 1.0f;

    return result;
}

inline m3f operator*(m3f a, m3f b) {
    m3f result = {};

    v3f acol0 = a.col0;
    v3f acol1 = a.col1;
    v3f acol2 = a.col2;

    v3f bcol0 = b.col0;
    v3f bcol1 = b.col1;
    v3f bcol2 = b.col2;

    result.col0 = bcol0.x * acol0 + bcol0.y * acol1 + bcol0.z * acol2;
    result.col1 = bcol1.x * acol0 + bcol1.y * acol1 + bcol1.z * acol2;
    result.col2 = bcol2.x * acol0 + bcol2.y * acol1 + bcol2.z * acol2;

    return result;
}

inline v3f operator*(v3f v, m3f mat) {
    v3f result;
    result = v.x * mat.col0 + v.y * mat.col1 + v.z * mat.col2;

    return result;
}

inline v3f operator*(m3f mat, v3f v) {
    v3f result;
    result = v.x * mat.col0 + v.y * mat.col1 + v.z * mat.col2;

    return result;
}

inline f32 m3f_determinant(m3f mat) {
    f32 result = 0.0f;

    u32 base_row_index = 0;
    u32 moving_column_index = 0;

    u32 scalar_negative_factor = 1;

    // mat index is for the 3 2x2 matrices that we form from the original mat.
    for (u32 moving_column_index = 0; moving_column_index < 3; ++moving_column_index, scalar_negative_factor *= -1) {
        m2f selected_mat;

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

        f32 selected_mat_determinant = m2f_determinant(selected_mat);
        result += scalar * selected_mat_determinant;
    }

    return result;
}

inline m3f m3f_rotate_x(f32 deg) {
    m3f result = m3f_identity();

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

inline m3f m3f_rotate_y(f32 deg) {
    m3f result = m3f_identity();

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

inline m3f m3f_rotate_z(f32 deg) {
    m3f result = m3f_identity();

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

inline m3f m3f_rotate(v3f rot) {
    m3f result = m3f_identity();

    m3f x = m3f_rotate_x(rot.x);
    m3f y = m3f_rotate_y(rot.y);
    m3f z = m3f_rotate_z(rot.z);

    // note: yaw pitch then roll, this is supposedly standard for fps games, rotation order matters, quaternions
    // supposedly fix this, so i'll look into them probably once i want third person view
    result = z * x * y;

    return result;
}

struct m4f {
    union {
        // stored column major - e[column][row]
        f32 e[4][4];
        struct {
            v4f col0;
            v4f col1;
            v4f col2;
            v4f col3;
        };
        struct {
            f32 m00, m01, m02, m03,
            m10, m11, m12, m13,
            m20, m21, m22, m23,
            m30, m31, m32, m33;
        };
    };
};

inline v3f v3f_lookat(v3f rotation) {
    v3f result;
    f32 pitch = deg_to_rad(rotation.pitch);
    f32 yaw = deg_to_rad(rotation.yaw);
    f32 roll = deg_to_rad(rotation.roll);

    result.x = cosf(pitch) * sinf(yaw);
    result.y = sinf(pitch); 
    result.z = cosf(pitch) * cosf(yaw);

    return result;
}

inline v4f operator*(v4f v, m4f mat) {
    v4f result;
    result = v.x * mat.col0 + v.y * mat.col1 + v.z * mat.col2 + v.w * mat.col3;

    return result;
}

inline v4f operator*(m4f mat, v4f v) {
    v4f result = v * mat;
    return result;
}

inline m4f operator*(m4f a, m4f b) {
    m4f result = {};

    v4f acol0 = a.col0;
    v4f acol1 = a.col1;
    v4f acol2 = a.col2;
    v4f acol3 = a.col3;

    v4f bcol0 = b.col0;
    v4f bcol1 = b.col1;
    v4f bcol2 = b.col2;
    v4f bcol3 = b.col3;

    result.col0 = bcol0.x * acol0 + bcol0.y * acol1 + bcol0.z * acol2 + bcol0.w * acol3;
    result.col1 = bcol1.x * acol0 + bcol1.y * acol1 + bcol1.z * acol2 + bcol1.w * acol3;
    result.col2 = bcol2.x * acol0 + bcol2.y * acol1 + bcol2.z * acol2 + bcol2.w * acol3;
    result.col3 = bcol3.x * acol0 + bcol3.y * acol1 + bcol3.z * acol2 + bcol3.w * acol3;

    return result;
}

inline m4f m4f_identity() {
    m4f result = {};
    result.e[0][0] = 1.0f;
    result.e[1][1] = 1.0f;
    result.e[2][2] = 1.0f;
    result.e[3][3] = 1.0f;

    return result;
}

// this whole function is wrong i think? maybe they're swapping each other into being correct idk lol
inline m4f create_m4f(f32 m00, f32 m01, f32 m02, f32 m03,
                      f32 m10, f32 m11, f32 m12, f32 m13,
                      f32 m20, f32 m21, f32 m22, f32 m23,
                      f32 m30, f32 m31, f32 m32, f32 m33)
{
    m4f result;
    result.e[0][0] = m00; result.e[1][0] = m10; result.e[2][0] = m20; result.e[3][0] = m30;
    result.e[0][1] = m01; result.e[1][1] = m11; result.e[2][1] = m21; result.e[3][1] = m31;
    result.e[0][2] = m02; result.e[1][2] = m12; result.e[2][2] = m22; result.e[3][2] = m32;
    result.e[0][3] = m03; result.e[1][3] = m13; result.e[2][3] = m23; result.e[3][3] = m33;

    return result;
}

inline f32 m4f_determinant(m4f mat) {
    f32 result = 0.0f;

    u32 base_row_index = 0;
    u32 moving_column_index = 0;

    u32 scalar_negative_factor = 1;

    // mat index is for the 4 3x3 matrices that we form from the original mat.
    for (u32 moving_column_index = 0; moving_column_index < 4; ++moving_column_index, scalar_negative_factor *= -1) {
        m3f selected_mat;

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

        f32 selected_mat_determinant = m3f_determinant(selected_mat);
        result += scalar * selected_mat_determinant;
    }

    return result;
}

inline m4f m4f_transpose(m4f mat) {
    m4f result;
    for (u32 row = 0; row < 4; ++row) {
        for (u32 column = 0; column < 4; ++column) {
            result.e[column][row] = mat.e[row][column];
        }
    }

    return result;
}

inline f32 m4f_row_dot(m4f mat, u32 row, f32 n) {
    f32 result = 0.0f;
    result = (mat.e[0][row] * n) + (mat.e[1][row] * n) + (mat.e[2][row] * n) + (mat.e[3][row] * n);

    return result;
}

inline m4f m4f_scale(v3f scale) {
    m4f result = m4f_identity();
    result.e[0][0] = scale.x;
    result.e[1][1] = scale.y;
    result.e[2][2] = scale.z;

    return result;
}

inline m4f m4f_rotate_x(f32 deg) {
    m4f result = m4f_identity();

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

inline m4f m4f_rotate_y(f32 deg) {
    m4f result = m4f_identity();

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

inline m4f m4f_rotate_z(f32 deg) {
    m4f result = m4f_identity();

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

inline m4f m4f_rotate(v3f rotation) {
    m4f result = m4f_identity();

    m4f x = m4f_rotate_x(rotation.x);
    m4f y = m4f_rotate_y(rotation.y);
    m4f z = m4f_rotate_z(rotation.z);

    // note: yaw pitch then roll, this is supposedly standard for fps games, rotation order matters, quaternions
    // supposedly fix this, so i'll look into them probably once i want third person view
    result = z * x * y;

    return result;
}

inline m4f m4f_translate(v3f translation) {
    m4f result = m4f_identity();
    result.e[3][0] = translation.x;
    result.e[3][1] = translation.y;
    result.e[3][2] = translation.z;

    return result;
}

inline m4f m4f_inverse_translate(v3f translation) {
    m4f result = m4f_identity();
    result.e[3][0] = -translation.x;
    result.e[3][1] = -translation.y;
    result.e[3][2] = -translation.z;

    return result;
}

inline m4f m4f_inverse_affine(m4f mat) {
    m4f result = m4f_identity();
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

inline m4f m4f_lookat(v3f from, v3f to, v3f up) {
    m4f result = m4f_identity();
    v3f z = v3f_norm(to - from);
    v3f x = v3f_norm(v3f_cross(up, z));
    v3f y = v3f_norm(v3f_cross(z, x));

    f32 xdot = -v3f_dot(from, x);
    f32 ydot = -v3f_dot(from, y);
    f32 zdot = -v3f_dot(from, z);

    result = create_m4f(
                        x.x, x.y, x.z, xdot,
                        y.x, y.y, y.z, ydot,
                        z.x, z.y, z.z, zdot,
                        0.0f, 0.0f, 0.0f, 1.0f
                        );

    return result;
}

inline m4f m4f_perspective(f32 width, f32 height, f32 fov, f32 near_plane, f32 far_plane) {
    m4f result = m4f_identity();
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
