#pragma once

#include "gfx.h"
#include "gfx_math.h"
#include "stdlib.h"
#include "vex.h"
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
    const uint32_t *map_kd;

    Vec3 *cam_projected_points;
    Vec3 *screen_points;

    uint8_t *file_buffer = nullptr;

    Model(const uint32_t num_materials,
          const Material *materials,
          const uint32_t num_verts,
          const Vec3 *verts,
          const uint32_t num_uvs,
          const Vec2 *uvs,
          const uint32_t num_normals,
          Vec3 *normals,
          const uint32_t num_faces,
          const Tri *faces,
          const uint32_t map_kd_width,
          const uint32_t map_kd_height,
          const uint32_t *map_kd,
          Vec3 *cam_projected_points,
          Vec3 *screen_points,
          uint8_t *file_buffer = nullptr) : num_materials(num_materials), materials(materials), num_verts(num_verts), verts(verts), num_uvs(num_uvs), uvs(uvs), num_normals(num_normals), normals(normals), num_faces(num_faces), faces(faces), map_kd_width(map_kd_width), map_kd_height(map_kd_height), map_kd(map_kd), cam_projected_points(cam_projected_points), screen_points(screen_points), file_buffer(file_buffer)
    {
    }

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
