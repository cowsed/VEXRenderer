#pragma once

#include "gfx.h"
#include "gfx_math.h"
#include "stdlib.h"
#include "vex.h"
#include "vex_brain.h"

#include <vector>
#include "span.hpp"

struct Model
{
    struct Texture
    {
        uint32_t width;
        uint32_t height;
        uint32_t *data;
    };
    tcb::span<Material> materials = {};
    tcb::span<Vec3> verts = {};
    tcb::span<Vec2> uvs = {};
    tcb::span<Tri> faces = {};
    Texture kd;

    std::vector<Vec3> normals;
    std::vector<Vec3> cam_projected_points;
    std::vector<Vec3> screen_points;

    bool ready = false;

    Model(bool valid){
        ready = valid;
    }
    bool is_ready(){
        return ready;
    }

    Model(tcb::span<Material> m_mat, tcb::span<Vec3> m_verts, tcb::span<Vec2> m_uvs, tcb::span<Tri> m_faces, Texture m_kd)
    {

        vex::brain Brain;
        Brain.Screen.print("Stack:) %x\n", &Brain);

        materials = m_mat;
        verts = m_verts;
        uvs = m_uvs;
        faces = m_faces;
        kd = m_kd;

        normals = std::vector<Vec3>(faces.size());
        cam_projected_points = std::vector<Vec3>(verts.size());
        screen_points = std::vector<Vec3>(verts.size());

        Brain.Screen.print("inside model: %x\n", &screen_points);

        calculate_normals();

        Brain.Screen.print("made normals\n");
        ready = true;
    }
    // Model(uint8_t *file_buf, int file_size)
    // {
    // }

    void calculate_normals()
    {
        // Precalculate world normals
        vex::brain Brain;
        Brain.Screen.newLine();
        Brain.Screen.newLine();
        Brain.Screen.newLine();
        Brain.Screen.newLine();
        Brain.Screen.newLine();
        Brain.Screen.newLine();
        Brain.Screen.print("faces:) %x\n", faces.data());

        Brain.Screen.print("normals: %x\n", normals.data());

        for (int i = 0; i < faces.size(); i++)
        {
            Tri t = faces[i];
            Vec3 world_normal = TriNormal(verts[t.v1_index], verts[t.v2_index], verts[t.v3_index]).Normalize();
            normals[i] = world_normal;
        }

        Brain.Screen.print("was normal\n");
    }
};

Model ModeFromFile(const char *fname);
