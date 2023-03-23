/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Module:       main.cpp                                                  */
/*    Author:       richie                                                    */
/*    Created:      2/24/2023, 2:52:50 PM                                     */
/*    Description:  Brain Renderer                                            */
/*                                                                            */
/*----------------------------------------------------------------------------*/

#include "vex.h"

using namespace vex;

// A global instance of competition
competition Competition;
brain Brain;

#include <iostream>
#include <chrono>

#include "gfx_math.h"
#include "gfx.h"
#include "model.h"

#ifndef M_PI
#define M_PI 3.141592
#endif

#define WIDTH (3 * 120)
#define HEIGHT (1 * 240)
const float width = (float)(WIDTH);
const float height = (float)(HEIGHT);
const float aspect = width / height;

const Vec3 ambientColor = {.0, .0, .0};
const float screenGamma = 2.2; // Assume the monitor is calibrated to the sRGB color space
const Vec3 lightColor = {1, 1, 1};
const float lightPower = 80.0;

const Vec3 light_pos = Vec3({2, 5, -2});
const Vec3 light_dir = light_pos.Normalize();
const float fov = 3.0; // in no units in particular. higher is 'more zoomed in'
const float near = 1.0;
const float far = 30.0;
const bool do_backface_culling = true; // seems to go faster with
const Vec3 view_dir_cam_space = Vec3(0, 0, -1.0);

// transforms from [-1, 1] to [0, Width]
inline Vec2 NDCtoPixels(const Vec2 &v)
{
  float newx = ((v.x / 2.0) + 0.5) * WIDTH;
  float newy = ((v.y / 2.0) + 0.5) * HEIGHT;
  return {newx, newy};
}
// pixel coordinates [0, WIDTH), [0, HEIGHT) to NDC [-1, 1], [-1, 1]
inline Vec3 PixelToNDC(const int x, const int y)
{
  const float xf = (float)x;
  const float yf = (float)y;
  Vec3 pixel_NDC = Vec3(2 * xf / width - 1.0f, 2 * yf / height - 1.0f, 0.0);
  return pixel_NDC;
}

uint32_t color_buffer1[HEIGHT][WIDTH];
float depth_buffer1[HEIGHT][WIDTH];

uint32_t color_buffer2[HEIGHT][WIDTH];
float depth_buffer2[HEIGHT][WIDTH];

const Vec3 clear_color = {1.0, 1.0, 1.0};

Vec3 reflect(Vec3 I, Vec3 N)
{
  return I - N * 2.0f * N.Dot(I);
}

/*
Vec3 shade_pixel(Vec3 vertPos, Vec3 normalInterp, Vec3 lightPos, Material mat)
{

  Vec3 normal = normalInterp.Normalize();
  Vec3 lightDir = lightPos - vertPos;
  float distance = lightDir.length();
  distance = distance * distance;
  lightDir = lightDir.Normalize();

  float lambertian = max(lightDir.Dot(normal), 0.0);
  float specular = 0.0;

  if (lambertian > 0.0)
  {

    Vec3 viewDir = (vertPos * -1.f).Normalize();

    // this is blinn phong
    Vec3 halfDir = (lightDir + viewDir).Normalize();
    float specAngle = max(halfDir.Dot(normal), 0.0);
    specular = pow(specAngle, mat.Ns); // shinninness

    // this is phong (for comparison)
    // if (true)
    //{
    //  Vec3 reflectDir = reflect(lightDir * -1, normal);
    //  specAngle = max(reflectDir.Dot(viewDir), 0.0);
    //  // note that the exponent is different here
    //  specular = pow(specAngle, mat.Ns / 4.0);
    //}
  }
  Vec3 colorLinear = ambientColor +
                     mat.diffuse * lambertian * lightColor * lightPower / distance +
                     mat.specular * specular * lightColor * lightPower / distance;
  // apply gamma correction (assume ambientColor, diffuseColor and specColor
  // have been linearized, i.e. have no gamma correction in them)
  Vec3 colorGammaCorrected = {powf(colorLinear.x, 1.0 / screenGamma), powf(colorLinear.y, 1.0 / screenGamma), powf(colorLinear.z, 1.0 / screenGamma)};
  // use the gamma corrected color in the fragment
  return colorGammaCorrected;
}
*/

inline void fill_tri(Model &m, int i, Vec3 v1, Vec3 v2, Vec3 v3, uint32_t color_buf[HEIGHT][WIDTH], float depth_buf[HEIGHT][WIDTH])
{

  Vec3 maybe_mid = {(m.verts[m.faces[i].v1_index].x + m.verts[m.faces[i].v2_index].x + m.verts[m.faces[i].v3_index].x) / 3.f, (m.verts[m.faces[i].v1_index].y + m.verts[m.faces[i].v2_index].y + m.verts[m.faces[i].v3_index].y) / 3.f, (m.verts[m.faces[i].v1_index].z + m.verts[m.faces[i].v2_index].z + m.verts[m.faces[i].v3_index].z) / 3.f};

  // pre-compute 1 over z
  v1.z = 1 / v1.z;
  v2.z = 1 / v2.z;
  v3.z = 1 / v3.z;

  const Rect bb = bounding_box2d(v1.toVec2(), v2.toVec2(), v3.toVec2());
  const Rect bb_pixel = Rect{.min = NDCtoPixels(bb.min), .max = NDCtoPixels(bb.max)};

  // dont even try if we're entirely off screen (good for zoomed in)
  if (bb_pixel.min.x > WIDTH || bb_pixel.max.x < 0)
  {
    return;
  }
  if (bb_pixel.min.y > HEIGHT || bb_pixel.max.y < 0)
  {
    return;
  }

  // indices into buffer that determine the bounding box
  const int minx = (int)max(0, bb_pixel.min.x - 1);
  const int miny = (int)max(0, bb_pixel.min.y - 1);
  const int maxx = (int)min(bb_pixel.max.x + 1, WIDTH);
  const int maxy = (int)min(bb_pixel.max.y + 1, HEIGHT);

  // We don't interpolate vertex attributes, we're filling only one tri at a time -> all this stuff is constant
  const Vec3 world_normal = m.normals[i];
  const float amt = world_normal.Dot(light_pos);

  const Material mat = m.materials[m.faces[i].matID];

  const float amb = .2;

  const Vec3 pre_col = mat.diffuse * (amb + (1 - amb) * my_clamp(world_normal.Dot(light_dir), 0.0, 1.0)); // shade_pixel(maybe_mid, world_normal, light_pos, mat);
  const Vec3 col = {powf(pre_col.r, 1 / screenGamma), powf(pre_col.g, 1 / screenGamma), powf(pre_col.b, 1 / screenGamma)};

  for (int y = miny; y < maxy; y += 1)
  {
    for (int x = minx; x < maxx; x += 1)
    {

      const tri_info ti = insideTri(v1, v2, v3, PixelToNDC(x, y));
      if (ti.inside)
      {
        float depth = 1 / (ti.w1 * v1.z + ti.w2 * v2.z + ti.w3 * v3.z);
        if (depth > near && depth < far && depth < depth_buf[y][x])
        {
          color_buf[y][x] = col.toIntColor(); // Vec3ToColor({fabs(world_normal.x), fabs(world_normal.y), fabs(world_normal.z)}); //
          depth_buf[y][x] = depth;
        }
      }
    }
  }
}
void precalculate(Model &m)
{
  // Precalculate world normals
  for (int i = 0; i < m.num_faces; i++)
  {
    Tri t = m.faces[i];
    Vec3 world_normal = TriNormal(m.verts[t.v1_index], m.verts[t.v2_index], m.verts[t.v3_index]).Normalize(); // normals[i]; //
    m.normals[i] = world_normal;
  }
}

void clear_buffers(uint32_t color[HEIGHT][WIDTH], float depth[HEIGHT][WIDTH])
{
  // Clear all pixels
  for (int y = 0; y < HEIGHT; y++)
  {
    for (int x = 0; x < WIDTH; x++)
    {
      color[y][x] = clear_color.toIntColor();
    }
  }
  for (int y = 0; y < HEIGHT; y++)
  {
    for (int x = 0; x < WIDTH; x++)
    {
      depth[y][x] = far + 1;
    }
  }
}

Vec3 focus_point = {0, 1, 0};

double projection_time;
double clear_time;
double blit_time;

Vec3 *screen_points;
Vec3 *cam_projected_points;

void render(Model &m, uint32_t color[HEIGHT][WIDTH], float depth[HEIGHT][WIDTH], double x, double y, double z)
{

  vex::timer thing_tmr;
  thing_tmr.reset();
  clear_buffers(color, depth);
  clear_time = thing_tmr.time(msec);
  // Project all the points to screen space
  thing_tmr.reset();

  Mat4 trans = Translate3D({0, -0, (float)(-10.0) * (float)z});
  const Mat4 rotx = RotateX(y);
  const Mat4 roty = RotateY(x);
  const Mat4 move = Translate3D(focus_point * -1);
  Mat4 transform = (trans * rotx * roty * move);

  // std::cerr << "transform:\n"
  // 		  << transform << std::endl;

  for (int i = 0; i < m.num_verts; i++)
  {
    Vec4 p = m.verts[i].toVec4(1.0);

    Vec4 p_cam = transform.Mul4xV4(p);
    cam_projected_points[i] = p_cam.toVec3();

    // divide by depth. things farther away are smaller
    Vec4 p_NDC = {fov * near * p_cam.x / -p_cam.z, fov * near * p_cam.y / -p_cam.z, -p_cam.z};

    // negate Y (+1 is up in graphics, +1 is down in image)
    //  do correction for aspect ratio
    Vec3 p_screen = {p_NDC.x, -p_NDC.y * aspect, p_NDC.z};
    screen_points[i] = p_screen;
  }
  projection_time = thing_tmr.time(msec);
  thing_tmr.reset();

  // Assemble triangles and color they pixels
  for (int i = 0; i < m.num_faces; i++)
  {
    const Tri t = m.faces[i];
    const Vec3 v1 = screen_points[t.v1_index];
    const Vec3 v2 = screen_points[t.v2_index];
    const Vec3 v3 = screen_points[t.v3_index];

    if (do_backface_culling)
    {
      Vec3 cam_space_normal = TriNormal(cam_projected_points[t.v1_index], cam_projected_points[t.v2_index], cam_projected_points[t.v3_index]);
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

    // fill_tri_tiled(i, v1, v2, v3); // 53ms
    fill_tri(m, i, v1, v2, v3, color, depth); // 40ms 30ms when inlined
  }
  blit_time = thing_tmr.time(msec);
}

bool show_stats = true;
bool demo_mode = false; // if true rotate in circles
bool pan = true;

void printTextCenteredAt(int x, int y, const char *str)
{
  Brain.Screen.printAt(x - Brain.Screen.getStringWidth(str) / 2, y - Brain.Screen.getStringHeight(str) / 2, str);
}

void usercontrol(void)
{
  printf("Rendering\n");

  screen_points = (Vec3 *)malloc(robot_lowpoly_model.num_verts * sizeof(Vec3));
  cam_projected_points = (Vec3 *)malloc(robot_lowpoly_model.num_verts * sizeof(Vec3));
  precalculate(robot_lowpoly_model);

  // increasing number of seconds to render with
  float animation_time = 0.0;

  double rx = 0.0;
  double ry = 0.0;
  double z = .95;

  double tx = 0.0;
  double ty = 0.0;
  double tz = 0.0;

  double dx = 0;
  double dy = 0;

  int start_x = 240;
  int start_y = 120;

  bool was_pressing = false;

  int last_mx = -1;
  int last_my = -1;

  while (true)
  {

    vex::timer tmr;
    tmr.reset();

    Brain.Screen.clearScreen(0xFFFFFFFF);
    z = my_clamp(z, 0.1, 2.0);
    if (demo_mode)
    {
      rx = animation_time;
      ry = .5;
      // z = 1.0;
    }

    bool pressing = Brain.Screen.pressing();

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
        if (!pan)
        {
          // rotate
          rx += (mx - last_mx) / -100.0;
          ry += (my - last_my) / 100.0;
        }
        else
        {
          // pan
          float dx = (float)(mx - last_mx) / 100.0;
          float dy = (float)(my - last_my) / 100.0;
          Vec3 d_focus = (RotateY(-rx - M_PI) * RotateX(ry)).Mul4xV3(Vec3(dx, dy, 0.0));
          focus_point = focus_point + d_focus;
        }
      }
    }

    last_mx = mx;
    last_my = my;

    was_pressing = pressing;

    render(robot_lowpoly_model, color_buffer1, depth_buffer1, rx, ry, z);

    double frame_time_ms = tmr.time(timeUnits::msec);
    double frame_time_s = tmr.time(timeUnits::sec);

    vexDelay(16 - tmr.time());

    Brain.Screen.drawImageFromBuffer(&color_buffer1[0][0], (480 - WIDTH) / 2, 0, WIDTH, HEIGHT);
    if (show_stats)
    {
      Brain.Screen.setPenColor(vex::red);
      Brain.Screen.setFillColor(vex::white);
      Brain.Screen.setFont(mono15);
      Brain.Screen.printAt(10, 20, false, "Backface Cull: %d", do_backface_culling);
      Brain.Screen.printAt(10, 40, false, "frametime: %.0fms", frame_time_ms);
      Brain.Screen.printAt(10, 60, false, "fps: %.0f", (1.0 / frame_time_s));

      Brain.Screen.printAt(10, 80, false, "clear time: %.0fms", clear_time);
      Brain.Screen.printAt(10, 100, false, "project time: %.0fms", projection_time);
      Brain.Screen.printAt(10, 120, false, "blit time: %.0f", blit_time);
      Brain.Screen.printAt(10, 140, false, "focus: %.1f, %.1f, %.1f", focus_point.x, focus_point.y, focus_point.z);

      Brain.Screen.printAt(10, 170, false, "%d faces", robot_lowpoly_model.num_faces);
      Brain.Screen.printAt(10, 190, false, "%d verts", robot_lowpoly_model.num_verts);
    }

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

    Brain.Screen.render();
    animation_time += 0.02;
  }

  free(cam_projected_points);
  free(screen_points);

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
