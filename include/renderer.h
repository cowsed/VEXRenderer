#include "math.h"
#include "gfx.h"
#include "gfx_math.h"
#include "model_common.h"
#include "stdlib.h"
#include "vex.h"

struct RenderTarget
{
    RenderTarget(int width, int height);
    ~RenderTarget();

    void Clear(uint32_t col, float depth);
    const int width;
    const int height;
    uint32_t *color_buffer;
    float *depth_buffer;
};

struct render_params
{
    const float fov;
    const float near;
    const float far;
    const bool do_backface_culling;
    const float screen_gamma;
    const Vec3 light_pos;
    const Vec3 light_dir;
};

Mat4 turntable_matrix(float x, float y, float zoom, Vec3 focus_point);

void render(const render_params &params, Model &m, RenderTarget &rt, const Mat4 view, const Mat4 model);

