#pragma once

#include "gfx.h"
#include "gfx_math.h"
#include "stdlib.h"

struct Model
{
    const int num_materials;
    const Material *materials;

    const int num_verts;
    const Vec3 *verts;

    const int num_uvs;
    const Vec2 *uvs;

    const int num_normals;
    Vec3 *normals;

    const int num_faces;
    const Tri *faces;

    const int map_kd_width;
    const int map_kd_height;
    const uint32_t *map_kd;

    Vec3 *cam_projected_points;
    Vec3 *screen_points;

    void allocate_enough()
    {
        cam_projected_points = (Vec3 *)malloc(sizeof(Vec3) * num_verts);
        screen_points = (Vec3 *)malloc(sizeof(Vec3) * num_verts);
    }
    ~Model()
    {
        free(cam_projected_points);
        free(screen_points);
    }
    void calculate_normals()
    {
        // Precalculate world normals
        for (int i = 0; i < num_faces; i++)
        {
            Tri t = faces[i];
            Vec3 world_normal = TriNormal(verts[t.v1_index], verts[t.v2_index], verts[t.v3_index]).Normalize(); // normals[i]; //
            normals[i] = world_normal;
        }
    }
};