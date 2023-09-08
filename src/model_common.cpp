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

    if (!sd.isInserted()){
        printf("No SD Card Inserted\n");
        return Model(false);
    }

    if (!sd.exists(fname))
    {
        printf("File not found\n");
        return Model(false);
    }

    int32_t buf_size = sd.size(fname);

    if (buf_size == 0)
    {
        printf("File is empty\n");
        return Model(false);
    }

    uint8_t *file_buffer = (uint8_t *)malloc(buf_size);

    if (file_buffer == NULL)
    {
        printf("Vex won't give me memory :(\n");
        return Model(false);
    }

    int32_t read = sd.loadfile(fname, file_buffer, buf_size);
    if (read == 0)
    {
        printf("File not read\n");
        return Model(false);
    }

    uint32_t num_materials = read_uint32(file_buffer, 0);
    uint32_t num_verts = read_uint32(file_buffer, 4);
    uint32_t num_uvs = read_uint32(file_buffer, 8);
    uint32_t num_faces = read_uint32(file_buffer, 12);
    uint32_t kd_width = read_uint32(file_buffer, 16);
    uint32_t kd_height = read_uint32(file_buffer, 20);
    uint32_t cur_index = 24;

    Vec3 *verts = (Vec3 *)(file_buffer + cur_index);
    cur_index += sizeof(Vec3) * num_verts;

    Vec2 *uvs = (Vec2 *)(file_buffer + cur_index);
    cur_index += sizeof(Vec2) * num_uvs;

    Tri *tris = (Tri *)(file_buffer + cur_index);
    cur_index += sizeof(Tri) * num_faces;

    /// MATERIALSSSSS
    Material *mats = (Material *)(file_buffer + cur_index);
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

    return Model(tcb::span<Material>(mats, num_materials),
                 tcb::span<Vec3>(verts, num_verts),
                 tcb::span<Vec2>(uvs, num_uvs),
                 tcb::span<Tri>(tris, num_faces),
                 Model::Texture{kd_width, kd_height, map_kd});
}
