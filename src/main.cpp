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

#include <iostream>
#include <chrono>

#include "renderer.h"

#include <stdio.h>
#include <stdlib.h>
#include <cassert>

#include "3dmodel.h"

#ifndef M_PI
#define M_PI 3.141592
#endif

#define WIDTH (3 * 120)
#define HEIGHT (1 * 240)
const Vec3 clear_color = {1.0, 1.0, 1.0};

const render_params params = {
    3.0,                       //  fov
    1.0,                       //  near
    50.0,                      //  far
    true,                      //  do_backface_culling
    2.2,                       //  screen gamma
    Vec3(2, 5, 2),             //  .light_pos
    Vec3(2, 5, 2).Normalize(), //  .light_dir
};

RenderTarget viewport(WIDTH, HEIGHT);

using namespace vex;
competition Competition;
brain Brain;
vex::controller main_controller;
void printTextCenteredAt(int x, int y, const char *str)
{
  Brain.Screen.printAt(x - Brain.Screen.getStringWidth(str) / 2, y - Brain.Screen.getStringHeight(str) / 2, str);
}

bool demo_mode = false;
Vec3 focus_point = {0, .2, 0};
double rx = -M_PI / 4;
double ry = M_PI / 4;
double z = 1.15;
bool show_stats = true;
bool pan = false;

void do_cam_movement();
void switch_modes();
void draw_right_buttons(bool show_stats);
void draw_stats(bool show_stats, int render_time_ms, int full_frame_time, const Model &model);

void usercontrol(void)
{
  Model model = ModeFromFile("ritvexu.vobj");
  assert(model.is_ready());
  Model model2 = ModeFromFile("milk.vobj");
  assert(model2.is_ready());
  Model model3 = ModeFromFile("brought.vobj");
  assert(model3.is_ready());

  double full_frame_time = 0.0;
  double clear_time = 0.0;
  double blit_time = 0.0;

  while (true)
  {

    vex::timer tmr;
    tmr.reset();

    Brain.Screen.clearScreen(clear_color.toIntColor());
    clear_time = tmr.time(msec);

    do_cam_movement();

    viewport.Clear(clear_color.toIntColor(), params.far + 1);

    Mat4 view = turntable_matrix(rx, ry, z * 10.f, focus_point);

    // printf("boutta rendere\n");
    // fflush(stdout);
    // vex::wait(.4, vex::sec);
    //
    render(params, model, viewport, view, Mat4Identity());
    double render_time_ms = tmr.time(timeUnits::msec);

    Brain.Screen.drawImageFromBuffer(viewport.color_buffer, (480 - WIDTH) / 2, 0, WIDTH, HEIGHT);
    full_frame_time = tmr.time(timeUnits::msec);

    // draw_stats(show_stats, render_time_ms, full_frame_time, model);
    draw_right_buttons(show_stats);
    switch_modes();

    vexDelay(20 - tmr.time());

    Brain.Screen.render();
  }
}

void draw_stats(bool show_stats, int render_time_ms, int full_frame_time, const Model &model)
{
  double render_time_s = (double)render_time_ms / (double)(1000.0);

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

    Brain.Screen.printAt(10, 170, false, "%d faces", model.faces.size()); // + robot_model.num_faces);
    Brain.Screen.printAt(10, 190, false, "%d verts", model.verts.size()); // + robot_model.num_verts);
  }
}

void draw_right_buttons(bool show_stats)
{
  if (show_stats)
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
}
void switch_modes()
{
  bool pressing = Brain.Screen.pressing();
  static bool was_pressing = false;
  if (pressing)
  {
    if (Brain.Screen.xPosition() < 60 && Brain.Screen.yPosition() < 60)
    {
      demo_mode = !demo_mode;
      show_stats = !show_stats;
    }
    if (Brain.Screen.xPosition() > 60 * 7)
    {
      if (Brain.Screen.yPosition() < 60)
      {
        z -= .01;
      }
      else if (Brain.Screen.yPosition() < 120)
      {
        z += .01;
      }
      else if (Brain.Screen.yPosition() < 180)
      {
        pan = true;
      }
      else
      {
        pan = false;
      }
    }
  }
  was_pressing = pressing;
}

void do_cam_movement()
{
  bool pressing = Brain.Screen.pressing();
  static bool was_pressing = false;

  static int last_mx = -1;
  static int last_my = -1;
  int mx = Brain.Screen.xPosition();
  int my = Brain.Screen.yPosition();

  if (pressing && !was_pressing)
  {
    last_mx = -1;
    last_my = -1;
  }

  if (pressing)
  {
    if (last_mx != -1) // skip first frame
    {
      float dx = 0;
      float dy = 0;

      if (!pan)
      {
        // rotate
        rx += (mx - last_mx) / -100.0;
        ry += (my - last_my) / 100.0;
      }
      else
      {
        // pan
        dx += (float)(mx - last_mx) / 100.0;
        dy += (float)(my - last_my) / 100.0;

        Vec3 d_focus = (RotateY(-rx - M_PI) * RotateX(ry)).Mul4xV3(Vec3(dx, dy, 0.0));
        focus_point = focus_point + d_focus;
      }
    }
  }

  last_mx = mx;
  last_my = my;

  if (main_controller.ButtonL1.pressing())
  {
    float dx = 0;
    float dy = 0;

    dx += main_controller.Axis4.position() / -100.0;
    dy += main_controller.Axis3.position() / 100.0;
    Vec3 d_focus = (RotateY(-rx - M_PI) * RotateX(ry)).Mul4xV3(Vec3(dx, dy, 0.0));
    focus_point = focus_point + d_focus;
  }

  if (!main_controller.ButtonL1.pressing())
  {

    rx += main_controller.Axis4.position() / 200.0;
    ry += main_controller.Axis3.position() / 200.0;
  }

  if (main_controller.ButtonUp.pressing())
  {
    z -= 0.01;
  }
  if (main_controller.ButtonDown.pressing())
  {
    z += 0.01;
  }

  if (demo_mode)
  {
    rx += 0.05;
  }
  z = my_clamp(z, 0.1, 3.0);
  was_pressing = pressing;
}

//
// Main will set up the competition functions and callbacks.
//

void pre_auton(void)
{
  printf("No preauto\n");
  usercontrol();
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
