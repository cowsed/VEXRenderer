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

#include <stdio.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.141592
#endif

#define WIDTH (3 * 120)
#define HEIGHT (1 * 240)
const Vec3 clear_color = {1.0, 1.0, 1.0};

const render_params params = {
    3.0,                        //  fov
    1.0,                        //  near
    50.0,                       //  far
    true,                       //  do_backface_culling
    2.2,                        //  screen gamma
    Vec3(2, 5, -2),             //  .light_pos
    Vec3(2, 5, -2).Normalize(), //  .light_dir
};

RenderTarget viewport(WIDTH, HEIGHT);

Model &model = monkey_model;

Vec3 reflect(Vec3 I, Vec3 N)
{
  return I - N * 2.0f * N.Dot(I);
}

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

Vec3 robot_pos = {0, 0, 0};
double robot_heading = 0.0;
bool demo_mode = false;
Vec3 focus_point = {0, .2, 0};


void usercontrol(void)
{
  printf("Rendering\n");
  model.init();
  static double full_frame_time = 0.0;

  double rx = -M_PI / 4;
  double ry = M_PI / 4;
  double z = .75;

  bool was_pressing = false;

  int last_mx = -1;
  int last_my = -1;

  while (true)
  {

    vex::timer tmr;
    tmr.reset();

    Brain.Screen.clearScreen(clear_color.toIntColor());
    clear_time = tmr.time(msec);

    bool pressing = Brain.Screen.pressing();

    int mx = Brain.Screen.xPosition();
    int my = Brain.Screen.yPosition();
    {
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
      viewport.Clear(clear_color.toIntColor(), params.far + 1);

      Mat4 view = turntable_matrix(rx, ry, z * 10.f, focus_point);

      render(params, model, viewport, view, Mat4Identity());

      Brain.Screen.drawImageFromBuffer(viewport.color_buffer, (480 - WIDTH) / 2, 0, WIDTH, HEIGHT);
    }

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

      Brain.Screen.printAt(10, 170, false, "%d faces", model.num_faces); // + robot_model.num_faces);
      Brain.Screen.printAt(10, 190, false, "%d verts", model.num_verts); // + robot_model.num_verts);
    }

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

      if (pressing)
      {
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
    }
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
