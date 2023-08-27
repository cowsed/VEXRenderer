#include "animation_controller.h"



void AnimationController::Setup(std::vector<std::pair<anim_func, uint>> parts)
{
    steps = parts;
}

const Model &AnimationController::GetModel() { return *model; }

void AnimationController::SetModelMatrix(Mat4 m)
{
    mat = m;
}
Mat4 AnimationController::GetModelMatrix()
{
    return mat;
}
void AnimationController::SetModel(Model *mod)
{
    model = mod;
}
bool AnimationController::tick(float dt, const render_params &params, RenderTarget &rt, Mat4 view)
{
    if (curr_anim >= steps.size())
    {
        curr_anim = 0;
        curr_tick = 0;
        return false;
    }
    steps[curr_anim].first(curr_tick, *this);
    auto pos = mat.GetPos();

    if (steps[curr_anim].second == 0)
    {
        curr_anim++;
        return true;
    }

    if (model != NULL)
    {
        render(params, *model, rt, view, mat);
    }

    if (steps[curr_anim].second == 0)
    {
        curr_anim++;
    }

    curr_tick++;
    if (curr_tick == steps[curr_anim].second)
    {
        curr_tick = 0;
        curr_anim++;
    }
    return true;
}

auto SetPosition(Vec3 pos) -> anim_func
{
    return [pos](int tick, AnimationController &ac)
    {
        Mat4 m = ac.GetModelMatrix();
        m.X03 = pos.x;
        m.X13 = pos.y;
        m.X23 = pos.z;

        ac.SetModelMatrix(m);
    };
}

auto SetModel(Model *mod) -> anim_func
{
    return [mod](int tick, AnimationController &ac)
    {
        ac.SetModel(mod);
    };
}

anim_func Slide(Vec3 from, Vec3 to, uint length_ticks, ease_func easer)
{
  return [from, to, length_ticks, easer](uint tick, AnimationController &ac)
  {
    // float dist = (from - to).length();
    // Vec3 dir = (to - from).Normalize();
    float t = (float)tick / (float)length_ticks;
    t = easer(t);

    Vec3 p = Vec3::lerp(from, to, t);

    Mat4 m = Mat4Identity();
    m.SetPos(p);
    ac.SetModelMatrix(m);

  };
}



/*    Easing Functions   */
// faster towards 0 and 1, slow in middle
float ease_middle(float t)
{
  return powf(t*2-1, 3.0)/2.0 + .5;
}

// faster towards 0, slower towards 1
float ease_end(float t)
{
  return powf(t, .9);
}

float ease_both(float t)
{
  return -(cosf(M_PI * t) - 1) / 2;
}
