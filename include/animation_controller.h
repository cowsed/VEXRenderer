#include <vector>
#include <functional>
#include "gfx_math.h"

class AnimationController;

using anim_func = std::function<void(int, AnimationController &)>;

class AnimationController
{
public:
    void Setup(std::vector<std::pair<anim_func, uint>> parts)
    {
        steps = parts;
    }
    const Model &GetModel() { return *model; }
    void SetModelMatrix(Mat4 m)
    {
        mat = m;
    }
    Mat4 GetModelMatrix()
    {
        return mat;
    }
    void SetModel(Model *mod)
    {
        model = mod;
    }
    bool tick(float dt, const render_params &params, RenderTarget &rt, Mat4 view)
    {
        if (curr_anim >= steps.size())
        {
            return false;
        }
        steps[curr_anim].first(curr_tick, *this);
        auto pos = mat.GetPos();

        if (steps[curr_anim].second == 0)
        {
            curr_anim++;
            return true;
        }


        printf("curr_anim: %d, vec3: %f, %f, %f\n", curr_anim, pos.x, pos.y, pos.z);

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

private:
    Mat4 mat = Mat4Identity();
    // Mat4 view = turntable_matrix(0, 0, 0, {0, 0, 0});
    Model *model = NULL;
    std::vector<std::pair<anim_func, uint>> steps;

    int curr_tick = 0;

    int curr_anim = 0;
};
//

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

// class SetMatrix : public AnimationStep
// {
//     Mat4 v;
//     int frames;

// public:
//     // SetPosition(Vec3 v, int frames) : v(v), frames(frames) {}

//     void apply(int delta_tick, AnimationController &ac) override
//     {
//         // ac.SetModelPosition(v);
//     }
// };

// class SetPosition : public AnimationStep
// {
//     Vec3 v;

// public:
//     SetPosition(Vec3 m_v) : v(m_v) {}
//     void apply(int delta_tick, AnimationController &ac) override
//     {
//         ac.SetModelPosition(v);
//     }
// };
