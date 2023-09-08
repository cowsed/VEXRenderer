#include "renderer.h"
#include "assert.h"

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

    // Clear all q
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

inline Vec3 NDCtoPixelsZ(const Vec3 &v, const RenderTarget &rt)
{
    float newx = ((v.x / 2.0) + 0.5) * rt.width;
    float newy = ((v.y / 2.0) + 0.5) * rt.height;
    return {newx, newy, v.z};
}

// pixel coordinates [0, WIDTH), [0, HEIGHT) to NDC [-1, 1], [-1, 1]
inline Vec3 PixelToNDC(const int x, const int y, const RenderTarget &rt)
{
    const float xf = (float)x;
    const float yf = (float)y;
    Vec3 pixel_NDC = Vec3(2 * xf / rt.width - 1.0f, 2 * yf / rt.height - 1.0f, 0.0);
    return pixel_NDC;
}

Vec3 depthToCol(float depth)
{
    return {my_clamp(1.0 / (depth - 5), 0.0, 10.0), 0.0, 0.0};
}

const Vec3 db_palet[] = {{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}, {1.0, 1.0, 0.0}, {1.0, 0.0, 1.0}, {0.0, 1.0, 1.0}, {.5, .5, .5}};
inline void fill_tri_flat_top(const render_params &params, Model &m, int i, Vec3 v1, Vec3 v2, Vec3 v3, Vec2 uv1, Vec2 uv2, Vec2 uv3, const RenderTarget &rt)
{
    v1.z = 1.0 / v1.z;
    v2.z = 1.0 / v2.z;
    v3.z = 1.0 / v3.z;

    // make v1 leftmost
    if (v1.x < v2.x)
    {
        // nothing to be done
    }
    else
    {
        Vec3 tv = v1;
        Vec2 tuv = uv1;
        v1 = v2;
        uv1 = uv2;
        v2 = tv;
        uv2 = tuv;
    }

    if (v3.y - v1.y < .5)
    {
        return;
    }

    Vec3 left = v1;
    Vec3 right = v2;

    int top = v1.y;
    int bot = v3.y;

    // fprintf(stdout, "top. bot = %d %d\n", top, bot);

    float lx_per_y = (v3.x - v1.x) / (v3.y - v1.y);
    float rx_per_y = (v3.x - v2.x) / (v3.y - v2.y);

    float lz_per_y = (v3.z - v1.z) / (v3.y - v1.y);
    float rz_per_y = (v3.z - v2.z) / (v3.y - v2.y);

    const Vec3 world_normal = m.normals[i];

    const Material mat = m.materials[m.faces[i].matID];

    const float amb = .2;

    for (int y = top; y <= bot; y++)
    {

        float dz = (right.z - left.z) / (right.x - left.x);
        float z = left.z;
        if (left.x >= rt.width || right.x < 0)
        {
            goto next_y;
        }
        if (y < 0)
        {
            goto next_y;
        }
        if (y >= rt.height)
        {
            return;
        }
        for (int x = (int)(left.x); x < (int)(right.x); x++)
        {
            z += dz;
            if (x < 0)
            {
                continue;
            }
            if (x >= rt.width)
            {
                goto next_y;
            }
            float depth = 1.0 / z;
            // assert(left.x <= right.x); //, "gotta go in order man");
            // fprintf(stdout, "%d, %d : %f %f \n", x, y, left.x, right.x);
            if (depth > params.near && depth < params.far && depth < rt.depth_buffer[y * rt.width + x])
            {
                rt.depth_buffer[y * rt.width + x] = depth;

                const Vec3 pre_col = mat.diffuse * (amb + (1 - amb) * my_clamp(world_normal.Dot(params.light_dir), 0, 1.0));
                const Vec3 col = {powf(pre_col.r, 1 / params.screen_gamma), powf(pre_col.g, 1 / params.screen_gamma), powf(pre_col.b, 1 / params.screen_gamma)};
                // rt.color_buffer[y * rt.width + x] = col.toIntColor();
                rt.color_buffer[y * rt.width + x] = depthToCol(depth).toIntColor();

                // Vec3 col = db_palet[i % 7];
                // rt.color_buffer[y * rt.width + x] = col.toIntColor();
                // rt.color_buffer[y * rt.width + x] = (Vec3(0, 0, 0) + depthToCol(z)).toIntColor();
            }

            // vexDelay(1);
        }

    // step to next scanline
    next_y:
        left.x += lx_per_y;
        right.x += rx_per_y;
        left.z += lz_per_y;
        right.z += rz_per_y;
        left.y++;
        right.y++;
    }
}

inline void fill_tri_flat_bot(const render_params &params, Model &m, int i, Vec3 v1, Vec3 v2, Vec3 v3, Vec2 uv1, Vec2 uv2, Vec2 uv3, const RenderTarget &rt)
{
    v1.z = 1.0 / v1.z;
    v2.z = 1.0 / v2.z;
    v3.z = 1.0 / v3.z;

    // make v2 leftmost
    if (v2.x < v3.x)
    {
        // nothing to be done
    }
    else
    {
        Vec3 tv = v2;
        Vec2 tuv = uv2;
        v2 = v3;
        uv2 = uv3;
        v3 = tv;
        uv3 = tuv;
    }

    if (v3.y - v1.y < .5)
    {
        return;
    }

    int top = (int)(v1.y);
    int bot = (int)(v3.y);

    Vec3 left = v1;
    Vec3 right = v1;

    float lx_per_y = (v2.x - v1.x) / (v2.y - v1.y);
    float rx_per_y = (v3.x - v1.x) / (v3.y - v1.y);

    float lz_per_y = (v2.z - v1.z) / (v2.y - v1.y);
    float rz_per_y = (v3.z - v1.z) / (v3.y - v1.y);

    // do barycentric interpolation linearly

    const Vec3 world_normal = m.normals[i];
    const Material mat = m.materials[m.faces[i].matID];
    const float amb = .2;

    for (int y = top; y < bot; y++)
    {
        float dz = (right.z - left.z) / (right.x - left.x);
        float z = left.z;
        if (left.x >= rt.width || right.x < 0)
        {
            goto next_y;
        }
        if (y < 0)
        {
            goto next_y;
        }
        if (y >= rt.height)
        {
            return;
        }
        for (int x = (int)(left.x); x < (int)right.x; x++)
        {
            if (x < 0)
            {
                continue;
            }
            if (x >= rt.width)
            {
                goto next_y;
            }
            z += dz;

            float depth = 1.0 / z;
            if (depth > params.near && depth < params.far && depth < rt.depth_buffer[y * rt.width + x])
            {
                rt.depth_buffer[y * rt.width + x] = depth;

                // Vec3 col = db_palet[i % 7];
                // rt.color_buffer[y * rt.width + x] = col.toIntColor();
                // rt.color_buffer[y * rt.width + x] = (Vec3(0, 0, 0) + depthToCol(z)).toIntColor();

                const Vec3 pre_col = mat.diffuse * (amb + (1 - amb) * my_clamp(world_normal.Dot(params.light_dir), 0, 1.0));
                const Vec3 col = {powf(pre_col.r, 1 / params.screen_gamma), powf(pre_col.g, 1 / params.screen_gamma), powf(pre_col.b, 1 / params.screen_gamma)};
                // rt.color_buffer[y * rt.width + x] = col.toIntColor();
                rt.color_buffer[y * rt.width + x] = depthToCol(depth).toIntColor();
            }
        }
    next_y:
        left.x += lx_per_y;
        right.x += rx_per_y;
        left.z += lz_per_y;
        right.z += rz_per_y;
        left.y++;
        right.y++;
    }
}

inline void fill_tri_f(const render_params &params, Model &m, int i, Vec3 v1, Vec3 v2, Vec3 v3, Vec2 uv1, Vec2 uv2, Vec2 uv3, const RenderTarget &rt)
{
    // Sort by y
    // starting from 3, move smaller things to left
    // lesser of 3,2 goes to 2
    if (v2.y > v3.y)
    {
        Vec3 tv = v2;
        Vec2 tuv = uv2;
        v2 = v3;
        uv2 = uv3;
        v3 = tv;
        uv3 = tuv;
    }
    // lesser of 2, 1 goes to 1
    if (v1.y > v2.y)
    {

        Vec3 tv = v1;
        Vec2 tuv = uv1;
        v1 = v2;
        uv1 = uv2;
        v2 = tv;
        uv2 = tuv;
    }

    if (v2.y > v3.y)
    {

        Vec3 tv = v2;
        Vec2 tuv = uv2;
        v2 = v3;
        uv2 = uv3;
        v3 = tv;
        uv3 = tuv;
    }

    Vec3 p1 = NDCtoPixelsZ(v1, rt);
    Vec3 p2 = NDCtoPixelsZ(v2, rt);
    Vec3 p3 = NDCtoPixelsZ(v3, rt);

    if ((int)p1.y == (int)p2.y)
    {
        // flat top triangle
        fill_tri_flat_top(params, m, i, p1, p2, p3, uv1, uv2, uv3, rt);
        return;
    }

    if ((int)p2.y == (int)p3.y)
    {
        // flat bottom triangle
        fill_tri_flat_bot(params, m, i, p1, p2, p3, uv1, uv2, uv3, rt);
        return;
    }

    // find point along line p1-p3 that is at the y of v2
    /* general case - split the triangle in a topflat and bottom-flat one */
    float t = (v2.y - v1.y) / (v3.y - v1.y);

    Vec3 p4 = Vec3::lerp(p1, p3, t);
    Vec2 uv4 = Vec2::lerp(uv1, uv3, t);

    fill_tri_flat_bot(params, m, i, p1, p2, p4, uv1, uv2, uv4, rt);
    fill_tri_flat_top(params, m, i, p2, p4, p3, uv2, uv4, uv3, rt);
}

inline void fill_tri(const render_params &params, Model &m, int i, Mat4 model_mat, Vec3 v1, Vec3 v2, Vec3 v3, Vec2 uv1, Vec2 uv2, Vec2 uv3, const RenderTarget &rt)
{

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
    const Vec3 world_normal = model_mat.GetNoPos().Mul4xV3(m.normals[i]);
    const Material mat = m.materials[m.faces[i].matID];

    const float amb = .2;

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

                        Vec3 col2 = get_tex(UV.u, UV.v, m.kd.width, m.kd.height, m.kd.data);
                        const Vec3 pre_col = col2 * (amb + (1 - amb) * my_clamp(world_normal.Dot(params.light_dir), 0, 1.0));
                        Vec3 col = {powf(pre_col.r, 1 / params.screen_gamma), powf(pre_col.g, 1 / params.screen_gamma), powf(pre_col.b, 1 / params.screen_gamma)};

                        // col.x = UV.x;
                        // col.y = UV.y;
                        // col.z = 0.0;
// 

                        rt.color_buffer[y * rt.width + x] = col.toIntColor();
                    }
                    else
                    {

                        const Vec3 pre_col = mat.diffuse * (amb + (1 - amb) * my_clamp(world_normal.Dot(params.light_dir), 0, 1.0));
                        const Vec3 col = {powf(pre_col.r, 1 / params.screen_gamma), powf(pre_col.g, 1 / params.screen_gamma), powf(pre_col.b, 1 / params.screen_gamma)};
                        // rt.color_buffer[y * rt.width + x] = col.toIntColor();
                        rt.color_buffer[y * rt.width + x] = col.toIntColor(); // depthToCol(depth).toIntColor();
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
    const float near = params.near;
    const bool do_backface_culling = params.do_backface_culling;
    const Vec3 view_dir_cam_space = Vec3(0, 0, -1.0);

    // Project all the points to screen space
    Mat4 transform = (view * model);

    for (int i = 0; i < m.verts.size(); i++)
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
    for (int i = 0; i < m.faces.size(); i++)
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
        // remove tris who are past far
        if (v1.z > params.far && v2.z > params.far && v3.z > params.far)
        {
            continue;
        }

        // fill_tri(params, m, i, v1, v2, v3, uv1, uv2, uv3, rt);

        fill_tri(params, m, i, model, v1, v2, v3, uv1, uv2, uv3, rt);
    }
}
Mat4 turntable_matrix(float x, float y, float zoom, Vec3 focus_point)
{
    Mat4 trans = Translate3D({0, -0, -(float)zoom});
    const Mat4 rotx = RotateX(y);
    const Mat4 roty = RotateY(x);
    const Mat4 move = Translate3D(focus_point * -1);
    return trans * rotx * roty * move;
    ;
}