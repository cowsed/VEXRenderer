#pragma once

#include <vector>
#include <functional>
#include "gfx_math.h"
#include "gfx.h"
#include "model.h"
#include "renderer.h"

class AnimationController;

using anim_func = std::function<void(int, AnimationController &)>;

class AnimationController
{
public:
    void Setup(std::vector<std::pair<anim_func, uint>> parts);
    const Model &GetModel();
    void SetModelMatrix(Mat4 m);
    Mat4 GetModelMatrix();
    void SetModel(Model *mod);
    bool tick(float dt, const render_params &params, RenderTarget &rt, Mat4 view);
    
private:
    Mat4 mat = Mat4Identity();
    Model *model = NULL;
    std::vector<std::pair<anim_func, uint>> steps;

    int curr_tick = 0;

    int curr_anim = 0;
};

using ease_func = float(*)(float);

auto SetPosition(Vec3 pos) -> anim_func;
auto SetModel(Model *mod) -> anim_func;
auto Slide(Vec3 from, Vec3 to, uint length_ticks, ease_func easer) -> anim_func;


// fast at 0, slow at .5, fast at 1
float ease_middle(float t);

// faster towards 0, slower towards 1
float ease_end(float t);

// fast at 0, slow at .5, fast at 1
float ease_both(float t);