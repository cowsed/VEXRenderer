#include "model_common.h"
#include <iostream>

// index is byte index, not 4 byte index
uint32_t read_uint32(uint8_t *buf, int index)
{
    return *(uint32_t *)(buf + index);
}

Model ModeFromFile(const char *fname)
{
    Model badm = Model(
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0);

    vex::brain::sdcard sd;

    if (!sd.exists(fname))
    {
        printf("No file %s exists\n", fname);
        fflush(stdout);
        vex::wait(.1, vex::sec);
        return badm;
    }
    int32_t buf_size = sd.size(fname);

    uint8_t *file_buffer = (uint8_t *)malloc(buf_size);
    int32_t read = sd.loadfile(fname, file_buffer, buf_size);
    if (read == 0)
    {
        printf("MODEL NOT FOUND\n");

        return badm;
    }

    uint32_t num_materials = read_uint32(file_buffer, 0);
    uint32_t num_verts = read_uint32(file_buffer, 4);
    uint32_t num_uvs = read_uint32(file_buffer, 8);
    uint32_t num_faces = read_uint32(file_buffer, 12);
    uint32_t kd_width = read_uint32(file_buffer, 16);
    uint32_t kd_height = read_uint32(file_buffer, 20);
    uint32_t cur_index = 24;

    const Vec3 *verts = (Vec3 *)(file_buffer + cur_index);
    cur_index += sizeof(Vec3) * num_verts;

    const Vec2 *uvs = (Vec2 *)(file_buffer + cur_index);
    cur_index += sizeof(Vec2) * num_uvs;

    const Tri *tris = (Tri *)(file_buffer + cur_index);
    cur_index += sizeof(Tri) * num_faces;

    /// MATERIALSSSSS
    const Material *mats = (Material *)(file_buffer + cur_index);
    cur_index += sizeof(Material) * num_materials;

    uint32_t *map_kd = (uint32_t *)(file_buffer + cur_index);

    /*
    4 byte num materials
    4 byte num verts.
    4 byte num uvs
    4 byte num faces
    4 byte kd_width
    4 byte kd_height
    [ verts ]
    [ uvs ]
    [ faces ]
    [ materials ]

    [ kd map ]

    */

    Model m = Model(
        num_materials,
        mats,
        num_verts,
        verts,
        num_uvs,
        uvs,
        num_faces,                                // normals
        (Vec3 *)malloc(sizeof(Vec3) * num_faces), // normals
        num_faces,
        tris,
        kd_width,
        kd_height,
        map_kd,
        0, 0,
        file_buffer);

    m.init();
    return m;
}
