#pragma once

#include "gfx.h"
#include "gfx_math.h"
#include "stdlib.h"
#include "vex_brain.h"

struct Model
{
    const uint32_t num_materials;
    const Material *materials;

    const uint32_t num_verts;
    const Vec3 *verts;

    const uint32_t num_uvs;
    const Vec2 *uvs;

    const uint32_t num_normals;
    Vec3 *normals;

    const uint32_t num_faces;
    const Tri *faces;

    const uint32_t map_kd_width;
    const uint32_t map_kd_height;
    uint32_t *map_kd;

    Vec3 *cam_projected_points;
    Vec3 *screen_points;

    uint8_t *file_buffer = nullptr;

    void allocate_enough()
    {
        cam_projected_points = (Vec3 *)malloc(sizeof(Vec3) * num_verts);
        screen_points = (Vec3 *)malloc(sizeof(Vec3) * num_verts);
    }
    // Model();
    ~Model()
    {
        free(cam_projected_points);
        free(screen_points);
        if (file_buffer != NULL)
        {
            free(file_buffer);
        }
    }
    void calculate_normals()
    {
        // Precalculate world normals
        for (int i = 0; i < num_faces; i++)
        {
            Tri t = faces[i];
            Vec3 world_normal = TriNormal(verts[t.v1_index], verts[t.v2_index], verts[t.v3_index]).Normalize();
            normals[i] = world_normal;
        }
    }
    void init()
    {
        allocate_enough();
        calculate_normals();
    }
};


Model ModeFromFile(const char *fname);
