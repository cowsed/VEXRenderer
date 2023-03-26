#include "renderer.h"

RenderTarget::RenderTarget(int width, int height) : width(width), height(height)
{
    color_buffer = (uint32_t *)malloc(sizeof(uint32_t) * width * height);
    depth_buffer = (float *)malloc(sizeof(float) * width * height);
}

RenderTarget::~RenderTarget()
{
    free(color_buffer);
    free(depth_buffer);
}
void RenderTarget::Clear(uint32_t col, float depth)
{

    // Clear all pixels
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            color_buffer[y * width + x] = col;
        }
    }
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            depth_buffer[y * width + x] = depth;
        }
    }
}

// transforms from [-1, 1] to [0, Width]
inline Vec2 NDCtoPixels(const Vec2 &v, const RenderTarget &rt)
{
  float newx = ((v.x / 2.0) + 0.5) * rt.width;
  float newy = ((v.y / 2.0) + 0.5) * rt.height;
  return {newx, newy};
}
// pixel coordinates [0, WIDTH), [0, HEIGHT) to NDC [-1, 1], [-1, 1]
inline Vec3 PixelToNDC(const int x, const int y, const RenderTarget &rt)
{
  const float xf = (float)x;
  const float yf = (float)y;
  Vec3 pixel_NDC = Vec3(2 * xf / rt.width - 1.0f, 2 * yf / rt.height - 1.0f, 0.0);
  return pixel_NDC;
}


inline void fill_tri(const render_params &params, Model &m, int i, Vec3 v1, Vec3 v2, Vec3 v3, Vec2 uv1, Vec2 uv2, Vec2 uv3, const RenderTarget &rt)
{

    Vec3 maybe_mid = {(m.cam_projected_points[m.faces[i].v1_index].x + m.cam_projected_points[m.faces[i].v2_index].x + m.cam_projected_points[m.faces[i].v3_index].x) / 3.f,
                      (m.cam_projected_points[m.faces[i].v1_index].y + m.cam_projected_points[m.faces[i].v2_index].y + m.cam_projected_points[m.faces[i].v3_index].y) / 3.f,
                      (m.cam_projected_points[m.faces[i].v1_index].z + m.cam_projected_points[m.faces[i].v2_index].z + m.cam_projected_points[m.faces[i].v3_index].z) / 3.f};

    uv1 = uv1 / v1.z;
    uv2 = uv2 / v2.z;
    uv3 = uv3 / v3.z;

    // pre-compute 1 over z
    v1.z = 1 / v1.z;
    v2.z = 1 / v2.z;
    v3.z = 1 / v3.z;

    const Rect bb = bounding_box2d(v1.toVec2(), v2.toVec2(), v3.toVec2());
    const Rect bb_pixel = Rect{.min = NDCtoPixels(bb.min, rt), .max = NDCtoPixels(bb.max, rt)};

    // dont even try if we're entirely off screen (good for zoomed in)
    if (bb_pixel.min.x > rt.width || bb_pixel.max.x < 0)
    {
        return;
    }
    if (bb_pixel.min.y > rt.height || bb_pixel.max.y < 0)
    {
        return;
    }

    // indices into buffer that determine the bounding box
    const int minx = (int)max(0, bb_pixel.min.x - 1);
    const int miny = (int)max(0, bb_pixel.min.y - 1);
    const int maxx = (int)min(bb_pixel.max.x + 1, rt.width);
    const int maxy = (int)min(bb_pixel.max.y + 1, rt.height);

    // We don't interpolate vertex attributes, we're filling only one tri at a time -> all this stuff is constant
    const Vec3 world_normal = m.normals[i];
    const float amt = world_normal.Dot(params.light_pos);

    const Material mat = m.materials[m.faces[i].matID];

    const float amb = .2;

    const Vec3 pre_col = mat.diffuse * (amb + (1 - amb) * my_clamp(world_normal.Dot(params.light_dir), 0, 1.0));
    const Vec3 col = {powf(pre_col.r, 1 / params.screen_gamma), powf(pre_col.g, 1 / params.screen_gamma), powf(pre_col.b, 1 / params.screen_gamma)};
    for (int y = miny; y < maxy; y += 1)
    {
        for (int x = minx; x < maxx; x += 1)
        {

            const tri_info ti = insideTri(v1, v2, v3, PixelToNDC(x, y, rt));
            if (ti.inside)
            {

                float depth = 1 / (ti.w1 * v1.z + ti.w2 * v2.z + ti.w3 * v3.z);

                if (depth > params.near && depth < params.far && depth < rt.depth_buffer[y * rt.width + x])
                {
                    rt.depth_buffer[y * rt.width + x] = depth;

                    if (mat.owns_kd)
                    {
                        Vec2 UV = depth * ((uv1 * ti.w1) + (uv2 * ti.w2) + (uv3 * ti.w3));

                        Vec3 col2 = get_tex(UV.u, UV.v, m.map_kd_width, m.map_kd_height, m.map_kd);
                        const Vec3 pre_col = col2 * (amb + (1 - amb) * my_clamp(world_normal.Dot(params.light_dir), 0, 1.0));
                        const Vec3 col = {powf(pre_col.r, 1 / params.screen_gamma), powf(pre_col.g, 1 / params.screen_gamma), powf(pre_col.b, 1 / params.screen_gamma)};

                        rt.color_buffer[y * rt.width + x] = col.toIntColor();
                    }
                    else
                    {
                        rt.color_buffer[y * rt.width + x] = col.toIntColor();
                    }
                }
            }
        }
    }
}

void render(const render_params &params, Model &m, RenderTarget &rt, const Mat4 view, const Mat4 model)
{
    const float aspect = (float)rt.width / (float)rt.height;
    const float fov = params.fov;
    const float far = params.far;
    const float near = params.near;
    const bool do_backface_culling = params.do_backface_culling;
    const Vec3 view_dir_cam_space = Vec3(0, 0, -1.0);

    // Project all the points to screen space
    Mat4 transform = (view * model);

    for (int i = 0; i < m.num_verts; i++)
    {
        Vec4 p = m.verts[i].toVec4(1.0);

        Vec4 p_cam = transform.Mul4xV4(p);
        m.cam_projected_points[i] = p_cam.toVec3();

        // divide by depth. things farther away are smaller
        Vec4 p_NDC = {fov * near * p_cam.x / -p_cam.z, fov * near * p_cam.y / -p_cam.z, -p_cam.z};

        // negate Y (+1 is up in graphics, +1 is down in image)
        //  do correction for aspect ratio
        Vec3 p_screen = {p_NDC.x, -p_NDC.y * aspect, p_NDC.z};
        m.screen_points[i] = p_screen;
    }

    // Assemble triangles and color they pixels
    for (int i = 0; i < m.num_faces; i++)
    {
        const Tri t = m.faces[i];
        const Vec3 v1 = m.screen_points[t.v1_index];
        const Vec3 v2 = m.screen_points[t.v2_index];
        const Vec3 v3 = m.screen_points[t.v3_index];

        const Vec2 uv1 = m.uvs[t.uv1_index];
        const Vec2 uv2 = m.uvs[t.uv2_index];
        const Vec2 uv3 = m.uvs[t.uv3_index];

        if (do_backface_culling)
        {
            Vec3 cam_space_normal = TriNormal(m.cam_projected_points[t.v1_index], m.cam_projected_points[t.v2_index], m.cam_projected_points[t.v3_index]);
            if ((view_dir_cam_space.Dot(cam_space_normal) >= 0.1))
            {
                // backface cull this dude
                continue;
            }
        }
        // remove tris with any points behind camera (really long tris will clip early but thats a sacrifice im willing to make)
        if (v1.z < 0 || v2.z < 0 || v3.z < 0)
        {
            continue;
        }

        fill_tri(params, m, i, v1, v2, v3, uv1, uv2, uv3, rt);
    }
}
