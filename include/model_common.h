#pragma once

#include "gfx.h"
#include "gfx_math.h"

struct Model
{
    const int num_materials;
    const Material *materials;

    const int num_verts;
    const Vec3 *verts;

    const int num_normals;
    Vec3 *normals;

    const int num_faces;
    const Tri *faces;
};