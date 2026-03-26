#ifndef TAP_MAT_H
#define TAP_MAT_H

#include "tap_vec.h"

typedef struct {
    float m0, m2;
    float m1, m3;
} TapMat2;

typedef struct {
    float m0, m3, m6;
    float m1, m4, m7;
    float m2, m5, m8;
} TapMat3;

typedef struct {
    float m0, m4, m8, m12;
    float m1, m5, m9, m13;
    float m2, m6, m10, m14;
    float m3, m7, m11, m15;
} TapMat4;

static TapMat2 tap_mat2_add(TapMat2 left, TapMat2 right);
static TapMat2 tap_mat2_sub(TapMat2 left, TapMat2 right);
static TapMat2 tap_mat2_mul(TapMat2 left, TapMat2 right);
static TapMat2 tap_mat2_scale(TapMat2 mat, float s);
static TapMat2 tap_mat2_transpose(TapMat2 mat);
static TapMat2 tap_mat2_invert(TapMat2 mat);
static TapVec2 tap_mat2_mulv(TapMat2 mat, TapVec2 vec);
static float tap_mat2_det(TapMat2 mat);

static TapMat3 tap_mat3_add(TapMat3 left, TapMat3 right);
static TapMat3 tap_mat3_sub(TapMat3 left, TapMat3 right);
static TapMat3 tap_mat3_mul(TapMat3 left, TapMat3 right);
static TapMat3 tap_mat3_scale(TapMat3 mat, float s);
static TapMat3 tap_mat3_transpose(TapMat3 mat);
static TapMat3 tap_mat3_invert(TapMat3 mat);
static TapVec3 tap_mat3_mulv(TapMat3 mat, TapVec3 vec);
static float tap_mat3_det(TapMat3 mat);

static TapMat4 tap_mat4_add(TapMat4 left, TapMat4 right);
static TapMat4 tap_mat4_sub(TapMat4 left, TapMat4 right);
static TapMat4 tap_mat4_mul(TapMat4 left, TapMat4 right);
static TapMat4 tap_mat4_scale(TapMat4 mat, float s);
static TapMat4 tap_mat4_transpose(TapMat4 mat);
static TapMat4 tap_mat4_invert(TapMat4 mat);
static TapVec4 tap_mat4_mulv(TapMat4 mat, TapVec4 vec);
static float tap_mat4_det(TapMat4 mat);

static TapMat2 tap_mat2_add(const TapMat2 left, const TapMat2 right)
{
    TapMat2 out = {0};
    out.m0 = left.m0 + right.m0;
    out.m1 = left.m1 + right.m1;
    out.m2 = left.m2 + right.m2;
    out.m3 = left.m3 + right.m3;
    return out;
}

static TapMat2 tap_mat2_sub(const TapMat2 left, const TapMat2 right)
{
    TapMat2 out = {0};
    out.m0 = left.m0 - right.m0;
    out.m1 = left.m1 - right.m1;
    out.m2 = left.m2 - right.m2;
    out.m3 = left.m3 - right.m3;
    return out;
}

static TapMat2 tap_mat2_mul(const TapMat2 left, const TapMat2 right)
{
    TapMat2 out = {0};
    out.m0 = (left.m0*right.m0) + (left.m2*right.m1);
    out.m1 = (left.m1*right.m0) + (left.m3*right.m1);
    out.m2 = (left.m0*right.m2) + (left.m2*right.m3);
    out.m3 = (left.m1*right.m2) + (left.m3*right.m3);
    return out;
}

static TapMat2 tap_mat2_scale(const TapMat2 mat, const float s)
{
    TapMat2 out = {0};
    out.m0 = mat.m0 * s;
    out.m1 = mat.m1 * s;
    out.m2 = mat.m2 * s;
    out.m3 = mat.m3 * s;
    return out;
}

static TapMat2 tap_mat2_transpose(const TapMat2 mat)
{
    TapMat2 out = {0};
    out.m0 = mat.m0;
    out.m1 = mat.m2;
    out.m2 = mat.m1;
    out.m3 = mat.m3;
    return out;
}

static TapMat2 tap_mat2_invert(const TapMat2 mat)
{
    TapMat2 out = {0};

    const float a = mat.m0;
    const float b = mat.m1;
    const float c = mat.m2;
    const float d = mat.m3;

    const float det = 1.0F / (a * d - b * c);

    out.m0 =  d * det;
    out.m1 = -b * det;
    out.m2 = -c * det;
    out.m3 =  a * det;

    return out;
}

static TapVec2 tap_mat2_mulv(const TapMat2 mat, const TapVec2 vec)
{
    TapVec2 out = {0};
    out.x = mat.m0 * vec.x + mat.m2 * vec.y;
    out.y = mat.m1 * vec.x + mat.m3 * vec.y;
    return out;
}

static float tap_mat2_det(const TapMat2 mat)
{
    return (mat.m0 * mat.m3) - (mat.m2 * mat.m1);
}

static TapMat3 tap_mat3_add(const TapMat3 left, const TapMat3 right)
{
    TapMat3 out = {0};
    out.m0 = left.m0 + right.m0;
    out.m1 = left.m1 + right.m1;
    out.m2 = left.m2 + right.m2;
    out.m3 = left.m3 + right.m3;
    out.m4 = left.m4 + right.m4;
    out.m5 = left.m5 + right.m5;
    out.m6 = left.m6 + right.m6;
    out.m7 = left.m7 + right.m7;
    out.m8 = left.m8 + right.m8;
    return out;
}

static TapMat3 tap_mat3_sub(const TapMat3 left, const TapMat3 right)
{
    TapMat3 out = {0};
    out.m0 = left.m0 - right.m0;
    out.m1 = left.m1 - right.m1;
    out.m2 = left.m2 - right.m2;
    out.m3 = left.m3 - right.m3;
    out.m4 = left.m4 - right.m4;
    out.m5 = left.m5 - right.m5;
    out.m6 = left.m6 - right.m6;
    out.m7 = left.m7 - right.m7;
    out.m8 = left.m8 - right.m8;
    return out;
}

static TapMat3 tap_mat3_mul(const TapMat3 left, const TapMat3 right)
{
    TapMat3 out = {0};
    out.m0 = (left.m0*right.m0) + (left.m3*right.m1) + (left.m6*right.m2);
    out.m1 = (left.m1*right.m0) + (left.m4*right.m1) + (left.m7*right.m2);
    out.m2 = (left.m2*right.m0) + (left.m5*right.m1) + (left.m8*right.m2);
    out.m3 = (left.m0*right.m3) + (left.m3*right.m4) + (left.m6*right.m5);
    out.m4 = (left.m1*right.m3) + (left.m4*right.m4) + (left.m7*right.m5);
    out.m5 = (left.m2*right.m3) + (left.m5*right.m4) + (left.m8*right.m5);
    out.m6 = (left.m0*right.m6) + (left.m3*right.m7) + (left.m6*right.m8);
    out.m7 = (left.m1*right.m6) + (left.m4*right.m7) + (left.m7*right.m8);
    out.m8 = (left.m2*right.m6) + (left.m5*right.m7) + (left.m8*right.m8);
    return out;
}

static TapMat3 tap_mat3_scale(const TapMat3 mat, const float s)
{
    TapMat3 out = {0};
    out.m0 = mat.m0 * s;
    out.m1 = mat.m1 * s;
    out.m2 = mat.m2 * s;
    out.m3 = mat.m3 * s;
    out.m4 = mat.m4 * s;
    out.m5 = mat.m5 * s;
    out.m6 = mat.m6 * s;
    out.m7 = mat.m7 * s;
    out.m8 = mat.m8 * s;
    return out;
}

static TapMat3 tap_mat3_transpose(const TapMat3 mat)
{
    TapMat3 out = {0};
    out.m0 = mat.m0;
    out.m1 = mat.m3;
    out.m2 = mat.m6;
    out.m3 = mat.m1;
    out.m4 = mat.m4;
    out.m5 = mat.m7;
    out.m6 = mat.m2;
    out.m7 = mat.m5;
    out.m8 = mat.m8;
    return out;
}

static TapMat3 tap_mat3_invert(const TapMat3 mat)
{
    TapMat3 out = {0};

    const float a = mat.m0;
    const float b = mat.m1;
    const float c = mat.m2;
    const float d = mat.m3;
    const float e = mat.m4;
    const float f = mat.m5;
    const float g = mat.m6;
    const float h = mat.m7;
    const float i = mat.m8;

    out.m0 =   e * i - f * h;
    out.m1 = -((b * i) - (h * c));
    out.m2 =   b * f - e * c;
    out.m3 = -((d * i) - (g * f));
    out.m4 =   a * i - c * g;
    out.m5 = -((a * f) - (d * c));
    out.m6 =   d * h - g * e;
    out.m7 = -((a * h) - (g * b));
    out.m8 =   a * e - b * d;

    const float det = 1.0F / (a * out.m0 + b * out.m3 + c * out.m6);

    out.m0 *= det;
    out.m1 *= det;
    out.m2 *= det;
    out.m3 *= det;
    out.m4 *= det;
    out.m5 *= det;
    out.m6 *= det;
    out.m7 *= det;
    out.m8 *= det;

    return out;
}

static TapVec3 tap_mat3_mulv(const TapMat3 mat, const TapVec3 vec)
{
    TapVec3 out = {0};
    out.x = mat.m0 * vec.x + mat.m3 * vec.y + mat.m6 * vec.z;
    out.y = mat.m1 * vec.x + mat.m4 * vec.y + mat.m7 * vec.z;
    out.z = mat.m2 * vec.x + mat.m5 * vec.y + mat.m8 * vec.z;
    return out;
}

static float tap_mat3_det(const TapMat3 mat)
{
  const float a = mat.m0, b = mat.m1, c = mat.m2,
        d = mat.m3, e = mat.m4, f = mat.m5,
        g = mat.m6, h = mat.m7, i = mat.m8;

  return (a * (e * i - h * f)) - (d * (b * i - c * h)) + (g * (b * f - c * e));
}

static TapMat4 tap_mat4_add(const TapMat4 left, const TapMat4 right)
{
    TapMat4 out = {0};
    out.m0 = left.m0 + right.m0;
    out.m1 = left.m1 + right.m1;
    out.m2 = left.m2 + right.m2;
    out.m3 = left.m3 + right.m3;
    out.m4 = left.m4 + right.m4;
    out.m5 = left.m5 + right.m5;
    out.m6 = left.m6 + right.m6;
    out.m7 = left.m7 + right.m7;
    out.m8 = left.m8 + right.m8;
    out.m9 = left.m9 + right.m9;
    out.m10 = left.m10 + right.m10;
    out.m11 = left.m11 + right.m11;
    out.m12 = left.m12 + right.m12;
    out.m13 = left.m13 + right.m13;
    out.m14 = left.m14 + right.m14;
    out.m15 = left.m15 + right.m15;
    return out;
}

static TapMat4 tap_mat4_sub(const TapMat4 left, const TapMat4 right)
{
    TapMat4 out = {0};
    out.m0 = left.m0 - right.m0;
    out.m1 = left.m1 - right.m1;
    out.m2 = left.m2 - right.m2;
    out.m3 = left.m3 - right.m3;
    out.m4 = left.m4 - right.m4;
    out.m5 = left.m5 - right.m5;
    out.m6 = left.m6 - right.m6;
    out.m7 = left.m7 - right.m7;
    out.m8 = left.m8 - right.m8;
    out.m9 = left.m9 - right.m9;
    out.m10 = left.m10 - right.m10;
    out.m11 = left.m11 - right.m11;
    out.m12 = left.m12 - right.m12;
    out.m13 = left.m13 - right.m13;
    out.m14 = left.m14 - right.m14;
    out.m15 = left.m15 - right.m15;
    return out;
}

static TapMat4 tap_mat4_mul(const TapMat4 left, const TapMat4 right)
{
    TapMat4 out = {0};
    out.m0 = left.m0 * right.m0 + left.m1 * right.m4 + left.m2 * right.m8 + left.m3 * right.m12;
    out.m1 = left.m0 * right.m1 + left.m1 * right.m5 + left.m2 * right.m9 + left.m3 * right.m13;
    out.m2 = left.m0 * right.m2 + left.m1 * right.m6 + left.m2 * right.m10 + left.m3 * right.m14;
    out.m3 = left.m0 * right.m3 + left.m1 * right.m7 + left.m2 * right.m11 + left.m3 * right.m15;
    out.m4 = left.m4 * right.m0 + left.m5 * right.m4 + left.m6 * right.m8 + left.m7 * right.m12;
    out.m5 = left.m4 * right.m1 + left.m5 * right.m5 + left.m6 * right.m9 + left.m7 * right.m13;
    out.m6 = left.m4 * right.m2 + left.m5 * right.m6 + left.m6 * right.m10 + left.m7 * right.m14;
    out.m7 = left.m4 * right.m3 + left.m5 * right.m7 + left.m6 * right.m11 + left.m7 * right.m15;
    out.m8 = left.m8 * right.m0 + left.m9 * right.m4 + left.m10 * right.m8 + left.m11 * right.m12;
    out.m9 = left.m8 * right.m1 + left.m9 * right.m5 + left.m10 * right.m9 + left.m11 * right.m13;
    out.m10 = left.m8 * right.m2 + left.m9 * right.m6 + left.m10 * right.m10 + left.m11 * right.m14;
    out.m11 = left.m8 * right.m3 + left.m9 * right.m7 + left.m10 * right.m11 + left.m11 * right.m15;
    out.m12 = left.m12 * right.m0 + left.m13 * right.m4 + left.m14 * right.m8 + left.m15 * right.m12;
    out.m13 = left.m12 * right.m1 + left.m13 * right.m5 + left.m14 * right.m9 + left.m15 * right.m13;
    out.m14 = left.m12 * right.m2 + left.m13 * right.m6 + left.m14 * right.m10 + left.m15 * right.m14;
    out.m15 = left.m12 * right.m3 + left.m13 * right.m7 + left.m14 * right.m11 + left.m15 * right.m15;
    return out;
}

static TapMat4 tap_mat4_scale(const TapMat4 mat, const float s)
{
    TapMat4 out = {0};
    out.m0 = mat.m0 * s;
    out.m1 = mat.m1 * s;
    out.m2 = mat.m2 * s;
    out.m3 = mat.m3 * s;
    out.m4 = mat.m4 * s;
    out.m5 = mat.m5 * s;
    out.m6 = mat.m6 * s;
    out.m7 = mat.m7 * s;
    out.m8 = mat.m8 * s;
    out.m9 = mat.m9 * s;
    out.m10 = mat.m10 * s;
    out.m11 = mat.m11 * s;
    out.m12 = mat.m12 * s;
    out.m13 = mat.m13 * s;
    out.m14 = mat.m14 * s;
    out.m15 = mat.m15 * s;
    return out;
}

static TapMat4 tap_mat4_transpose(const TapMat4 mat)
{
    TapMat4 out = {0};
    out.m0 = mat.m0;
    out.m1 = mat.m4;
    out.m2 = mat.m8;
    out.m3 = mat.m12;
    out.m4 = mat.m1;
    out.m5 = mat.m5;
    out.m6 = mat.m9;
    out.m7 = mat.m13;
    out.m8 = mat.m2;
    out.m9 = mat.m6;
    out.m10 = mat.m10;
    out.m11 = mat.m14;
    out.m12 = mat.m3;
    out.m13 = mat.m7;
    out.m14 = mat.m11;
    out.m15 = mat.m15;
    return out;
}

static TapVec4 tap_mat4_mulv(const TapMat4 mat, const TapVec4 vec)
{
    TapVec4 out = {0};
    out.x = mat.m0 * vec.x + mat.m4 * vec.y + mat.m8 * vec.z + mat.m12 * vec.w;
    out.y = mat.m1 * vec.x + mat.m5 * vec.y + mat.m9 * vec.z + mat.m13 * vec.w;
    out.z = mat.m2 * vec.x + mat.m6 * vec.y + mat.m10 * vec.z + mat.m14 * vec.w;
    out.w = mat.m3 * vec.x + mat.m7 * vec.y + mat.m11 * vec.z + mat.m15 * vec.w;
    return out;
}

#endif /* TAP_MAT_H */
