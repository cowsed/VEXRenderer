#pragma once
#include "gfx_math.h"
#include <stdint.h>
#include <math.h>
#include "stdio.h"

#ifdef NEON_ACCELERATED
#include "arm_neon.h"
#endif


const Vec3 up_vec = {0, 1, 0};

// ccw ordering
// normal calculation as such
struct Tri
{
    uint32_t v1_index;
    uint32_t v2_index;
    uint32_t v3_index;

    uint32_t uv1_index;
    uint32_t uv2_index;
    uint32_t uv3_index;

    uint32_t matID;
};

Vec3 TriNormal(const Vec3 &v1, const Vec3 &v2, const Vec3 &v3);

struct Material
{
    Vec3 ambient;
    Vec3 diffuse;
    Vec3 specular;
    float Ns; // 0-1000
    bool owns_kd;
};

/// @brief A camera represents the point that the scene is rendered from
struct Camera
{
    Vec3 pos;
    Vec3 target;

    Mat4 LookAtMatrix(Vec3 up);
};

/// @brief calculate the bounding box of 3 vertices in screen space
/// @param v0 one vertice of a tri
/// @param v1 another one
/// @param v2 another on
/// @return a rectangle representing the axis aligned bounding box of those vertices
Rect bounding_box2d(const Vec2 &v0, const Vec2 &v1, const Vec2 &v2);

struct tri_info
{
    bool inside;
    float area;
    float w1;
    float w2;
    float w3;
};

inline float edgeFunction(const Vec3 &a, const Vec3 &b, const Vec3 &c)
{
    return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}

inline const tri_info insideTri(const Vec3 &v1, const Vec3 &v2, const Vec3 &v3, const Vec3 &pixel_NDC)
{
    float area = edgeFunction(v1, v2, v3);
    float w1 = edgeFunction(v2, v3, pixel_NDC) / area;
    float w2 = edgeFunction(v3, v1, pixel_NDC) / area;
    float w3 = edgeFunction(v1, v2, pixel_NDC) / area;
    bool inside = (w1 >= 0) && (w2 >= 0) && (w3 >= 0);
    return {inside, area, w1, w2, w3};
}


Vec3 get_tex(float u, float v, int w, int h, const uint32_t *tex);