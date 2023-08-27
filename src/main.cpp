/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Module:       main.cpp                                                  */
/*    Author:       richie                                                    */
/*    Created:      2/24/2023, 2:52:50 PM                                     */
/*    Description:  Brain Renderer                                            */
/*                                                                            */
/*----------------------------------------------------------------------------*/

#include "vex.h"
#include <vector>
using namespace vex;

// A global instance of competition
competition Competition;
brain Brain;

#include <iostream>
#include <chrono>

// #include "gfx_math.h"
// #include "gfx.h"
#include "renderer.h"
#include "model.h"
#include "intense_milk.h"
#include "ritvexu.h"
#include "brought_to_you.h"

#include "animation_controller.h"

#include <stdio.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.141592
#endif

#define WIDTH (4 * 120)
#define HEIGHT (1 * 240)
const Vec3 clear_color = {.2, .2, .2};

const render_params params = {
    3.0,                        //  fov
    1.0,                        //  near
    50.0,                       //  far
    true,                       //  do_backface_culling
    2.2,                        //  screen gamma
    Vec3(-2, 5, 4),             //  .light_pos
    Vec3(-2, 5, 4).Normalize(), //  .light_dir
};

RenderTarget viewport(WIDTH, HEIGHT);

double projection_time;
double clear_time;
double blit_time;

bool show_stats = true;
bool pan = false;

void printTextCenteredAt(int x, int y, const char *str)
{
  Brain.Screen.printAt(x - Brain.Screen.getStringWidth(str) / 2, y - Brain.Screen.getStringHeight(str) / 2, str);
}

vex::controller main_controller;

bool demo_mode = false;
Vec3 focus_point = {0, 0, 0};



void usercontrol(void)
{
  brought_model.init();
  rit_vex_u_model.init();
  intense_milk_model.init();

  static double full_frame_time = 0.0;

  double rx = 0;
  double ry = -M_PI / 10;
  double z = .55;

  const int slide_frames = 80;
  auto rit_slide = Slide({-3, 0, 0}, {3, 0, 0}, slide_frames, ease_middle);
  auto word_slide = Slide({3, 0, 0}, {-3, 0, 0}, slide_frames, ease_middle);

  auto milk_appear = [](uint tick, AnimationController &ac)
  {
    float t = (float)tick / (float)slide_frames;
    t = ease_end(t);
    t *= 8;
    t -= 8;
    Vec3 p = {0, -.8f, t + 2.6f};
    float angle = t * M_PI / 2.0 + 2.325 * M_PI / 2.0;
    Mat4 m = RotateY(angle);
    m.SetPos(p);
    ac.SetModelMatrix(m);
  };

  auto do_nothing = [](uint tick, AnimationController &ac) {};

  AnimationController ac;
  ac.Setup({{SetModel(&rit_vex_u_model), 0},
            {SetPosition({-4.0, 0, 0}), 0},
            {rit_slide, slide_frames},
            {do_nothing, 10},

            {SetModel(&brought_model), 0},
            {SetPosition({4.0, 0, 0}), 0},
            {word_slide, slide_frames},
            {do_nothing, 10},

            {SetModel(&intense_milk_model), 0},
            {SetPosition({0, 0, 4.0}), 0},
            {milk_appear, slide_frames},
            {do_nothing, 10},

            {do_nothing, 200}});

  while (true)
  {

    vex::timer tmr;
    tmr.reset();

    Brain.Screen.clearScreen(clear_color.toIntColor());
    clear_time = tmr.time(msec);

    if (demo_mode)
    {
      rx -= 0.05;
    }

    viewport.Clear(clear_color.toIntColor(), params.far + 1);

    Mat4 view = turntable_matrix(rx, ry, z * 10.f, focus_point);

    ac.tick(1.0 / 60.0, params, viewport, view);

    Brain.Screen.drawImageFromBuffer(viewport.color_buffer, (480 - WIDTH) / 2, 0, WIDTH, HEIGHT);

    double render_time_ms = tmr.time(timeUnits::msec);
    double render_time_s = tmr.time(timeUnits::sec);

    // left side stats
    if (show_stats)
    {
      Brain.Screen.setPenColor(vex::red);
      Brain.Screen.setFillColor(vex::white);
      Brain.Screen.setFont(mono15);
      Brain.Screen.printAt(10, 20, false, "Backface Cull: %d", params.do_backface_culling);
      Brain.Screen.printAt(10, 40, false, "render time: %.0fms", render_time_ms);
      Brain.Screen.printAt(10, 60, false, "fps: %.0f", (1.0 / render_time_s));

      Brain.Screen.printAt(10, 120, false, "frame time: %.0f", full_frame_time);
      Brain.Screen.printAt(10, 140, false, "focus: %.1f, %.1f, %.1f", focus_point.x, focus_point.y, focus_point.z);

      Brain.Screen.printAt(10, 170, false, "%d faces", ac.GetModel().num_faces); // + robot_model.num_faces);
      Brain.Screen.printAt(10, 190, false, "%d verts", ac.GetModel().num_verts); // + robot_model.num_verts);
    }

    /*
    {
      // right side buttons
      Brain.Screen.setFillColor(vex::color(60, 60, 60));
      Brain.Screen.setPenColor(vex::white);
      Brain.Screen.drawRectangle(60 * 7, 0, 60, 60);
      printTextCenteredAt(480 - 30, 30, "+");
      Brain.Screen.drawRectangle(60 * 7, 60, 60, 60);
      printTextCenteredAt(480 - 30, 90, "-");

      Brain.Screen.drawRectangle(60 * 7, 120, 60, 60);
      printTextCenteredAt(480 - 30, 150, pan ? "!trans!" : "trans");
      Brain.Screen.drawRectangle(60 * 7, 180, 60, 60);
      printTextCenteredAt(480 - 30, 210, pan ? "rot" : "!rot!");
    }
    */
    vexDelay(20 - tmr.time());
    full_frame_time = tmr.time();

    Brain.Screen.render();
  }

  while (1)
  {
    wait(20, msec);
  }
}

//
// Main will set up the competition functions and callbacks.
//

void pre_auton(void)
{
  printf("No preauto\n");
}

void autonomous(void)
{
  printf("No auto");
}

int main()
{
  // Set up callbacks for autonomous and driver control periods.
  Competition.autonomous(autonomous);
  Competition.drivercontrol(usercontrol);

  // Run the pre-autonomous function.
  pre_auton();

  // Prevent main from exiting with an infinite loop.
  while (true)
  {
    wait(100, msec);
  }
}
