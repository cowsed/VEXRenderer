#include "model_common.h"
#include <iostream>

// index is byte index, not 4 byte index
uint32_t read_uint32(uint8_t *buf, int index)
{
    return *(uint32_t *)(buf + index);
}

Model ModeFromFile(const char *fname)
{
    vex::brain::sdcard sd;

    int32_t buf_size = sd.size(fname);
    std::cout << "buf size: " << buf_size << '\n';
    uint8_t *file_buffer = (uint8_t *)malloc(buf_size);
    int32_t read = sd.loadfile(fname, file_buffer, buf_size);
    if (read == 0)
    {
        printf("MODEL NOT FOUND\n");

        Model m = Model{
            .num_materials = 0,
            .materials = 0,

            .num_verts = 0,
            .verts = 0,

            .num_uvs = 0,
            .uvs = 0,

            .num_normals = 0,
            .normals = 0,

            .num_faces = 0,
            .faces = 0,

            .map_kd_width = 0,
            .map_kd_height = 0,
            .map_kd = 0,
            .file_buffer = 0};
        return m;
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

    std::cout << "uv offset " << cur_index<<'\n';
    const Vec2 *uvs = (Vec2 *)(file_buffer + cur_index);
    cur_index += sizeof(Vec2) * num_uvs;

    const Tri *tris = (Tri *)(file_buffer + cur_index);
    cur_index += sizeof(Tri) * num_faces;

    /// MATERIALSSSSS
    const Material *mats = (Material *)(file_buffer + cur_index);
    cur_index += sizeof(Material) * num_materials;

    const uint32_t *map_kd = (uint32_t *)(file_buffer + cur_index);

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
    std::cout << " uvs[0] " << uvs[0].x << ", " << uvs[0].y << '\n';

    Model m = Model{
        .num_materials = num_materials,
        .materials = mats,

        .num_verts = num_verts,
        .verts = verts,

        .num_uvs = num_uvs,
        .uvs = uvs,

        .num_normals = num_faces,
        .normals = (Vec3 *)malloc(sizeof(Vec3) * num_faces),

        .num_faces = num_faces,
        .faces = tris,

        .map_kd_width = kd_width,
        .map_kd_height = kd_height,
        .map_kd = map_kd,
        .file_buffer = file_buffer};

    m.init();
    return m;
}
