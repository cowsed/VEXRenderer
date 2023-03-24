// #include "vex.h"
// #include <vector>
// #include "gfx_math.h"

// struct position_t
// {
//     double x;
//     double y;
//     double rot;//degree
// };
// class Vector2D
// {
// public:
//     struct point_t
//     {
//         double x;
//         double y;
//     };
// };
// position_t cur_robot_pose = {0, 0, 0};

// class AutoCommand
// {
// public:
//     AutoCommand() {}
//     AutoCommand *withTimeout(double y)
//     {
//         return this;
//     }
// };
// class CommandController
// {
// public:
//     std::vector<Vec3> points;
//     Vec3 last_pos(){
//         if (points.size()==0){
//             return {0, 0, 0};
//         }
//         return points[points.size()-1];
//     }
//     void add(AutoCommand *ac, float timeout = 1.0)
//     {
//     }
//     void add(std::vector<AutoCommand *> cmds)
//     {
//         for (AutoCommand *cmd : cmds)
//         {
//             add(cmd);
//         }
//     }
// };

// CommandController mycc;

// class OdometryTank
// {
// };

// class OdomSetPosition : public AutoCommand
// {
// public:
//     OdomSetPosition(OdometryTank &o, position_t p) {}
// };

// class SpinRollerCommand : public AutoCommand
// {
// public:
//     SpinRollerCommand(position_t r) {}
// };

// OdometryTank odometry_sys;

// AutoCommand *DriveForwardFast(float dist, vex::directionType d)
// {
//     double ang = mycc.last_pos().z;
//     mycc.points.push_back({mycc.last_pos().x+cos(ang), mycc.last_pos().y + sin(ang)*dist, ang});
//     return new AutoCommand{}
// }

// #define StartIntake (new AutoCommand())

// using namespace vex;
// CommandController prog_skills_loader_side()
// {
//     int glbl_vision_center = 125;
//     bool target_red = true;
//     // flapup_solenoid.set(false);
//     position_t start_pos = position_t{.x = 36.0, .y = 12.2, .rot = -90};

//     position_t roller_in_pos = {.x = 36.0, .y = 4.16, .rot = -90};

//     CommandController lss;
//     lss.add(new OdomSetPosition(odometry_sys, start_pos)); // #1

//     // spin -90 degree roller
//     lss.add(new SpinRollerCommand(roller_in_pos), 5.0);
//     lss.add(DriveForwardFast(6, reverse));

//     Vector2D::point_t corner_disk_point = {.x = 10, .y = 12};

//     // intake corner disk
//     lss.add({
//         TurnToPoint(corner_disk_point)->withTimeout(1.5),        // #5
//         StartIntake,                                             // #6
//         DriveToPointSlowPt(corner_disk_point)->withTimeout(1.5), // #7
//         DriveForwardFast(4, reverse)->withTimeout(1.5),          // #8
//     });

//     Vector2D::point_t shoot_point = {.x = 10.5, .y = 93};

//     // align to 180 degree roller
//     lss.add(new SpinRPMCommand(flywheel_sys, 2900)); // #21

//     Vector2D::point_t roller_out_pos2 = {.x = 14, .y = 33};
//     position_t roller_in_pos2 = {.x = 4.20, .y = 33, .rot = 180};

//     lss.add({
//         TurnToPoint(roller_out_pos2)->withTimeout(1.5),
//         DriveToPointFastPt(roller_out_pos2)->withTimeout(1.5), // #12
//         TurnToHeading(180)->withTimeout(1.5),                  // #13

//         StopIntake,

//         // spin 180 degree roller
//         (new SpinRollerCommand(roller_in_pos2))->withTimeout(6.0),
//         DriveForwardFast(4.0, reverse)->withTimeout(1.5),
//         // drive to shoot point
//         TurnToPoint(shoot_point)->withTimeout(1.5), // #17
//         // PrintOdomContinous->withTimeout(18888),
//         DriveToPointFastPt(shoot_point)->withTimeout(2.0) // #19
//     });

//     // Shoot
//     lss.add(TurnToHeading(84), 0.85); // #20

//     lss.add_delay(1000);

//     lss.add(WaitForFW, 1.0);
//     lss.add(ShootDisk);
//     lss.add_delay(1000);

//     lss.add(WaitForFW, 1.0);
//     lss.add(ShootDisk);
//     lss.add_delay(1000);

//     lss.add(WaitForFW, 1.0);
//     lss.add(CLEAR_DISKS);

//     // lss.add(PrintOdomContinous); /// the guy youre looking for =================================================================================> :)

//     // DISKS AGAINST L PIECE
//     Vector2D::point_t disk_pos1 = {.x = 24.0, .y = 81.5};
//     Vector2D::point_t disk_pos2 = {.x = 32.0, .y = 81.5};
//     Vector2D::point_t disk_pos3 = {.x = 42.0, .y = 81.5};

//     Vector2D::point_t disk_prep_pos1 = {.x = 25, .y = 70};
//     Vector2D::point_t disk_prep_pos2 = {.x = 28, .y = 70};
//     Vector2D::point_t disk_prep_pos3 = {.x = 33, .y = 70};

//     // disks against L piece
//     lss.add({
//         // farthest
//         DriveToPointFastPtRev(disk_prep_pos1)->withTimeout(2.0), // #27
//         StartIntake,                                             // #26
//         TurnToPoint(disk_pos1)->withTimeout(1.5),                // #28
//         DriveToPointSlowPt(disk_pos1)->withTimeout(2.0),         // #29

//         // middle
//         DriveToPointFastPtRev(disk_prep_pos2)->withTimeout(2.0), // #30
//         TurnToPoint(disk_pos2)->withTimeout(1.5),                // #31
//         DriveToPointSlowPt(disk_pos2)->withTimeout(2.0),         // #32

//         // closest disk
//         DriveToPointFastPtRev(disk_prep_pos3)->withTimeout(2.0), // #33
//         TurnToPoint(disk_pos3)->withTimeout(1.5),                // #34
//         DriveToPointSlowPt(disk_pos3)->withTimeout(2.0),         // #35
//         DriveToPointFastPtRev(disk_prep_pos3)->withTimeout(2.0), // #36
//     });
//     lss.add(new SpinRPMCommand(flywheel_sys, 3100)); // #40

//     // Vector2D::point_t pre_shoot_point_other = {.x = 10.5, .y = 70};
//     // Vector2D::point_t shoot_point_other = {.x = 10.5, .y = 90};

//     // lss.add(TurnToPoint(pre_shoot_point_other), 1.5);        // #37
//     // lss.add(DriveToPointFastPt(pre_shoot_point_other), 4.0); // #38
//     // lss.add(TurnToPoint(shoot_point_other), 1.5);            // #37
//     // lss.add(DriveToPointFastPt(shoot_point_other), 4.0);     // #38
//     lss.add(new FlapDownCommand());

//     // lss.add(TurnToHeading(75), 0.5); // #39
//     Vector2D::point_t hoop_point_early = {.x = 17, .y = 123};
//     lss.add(TurnToPoint(hoop_point_early));

//     add_single_shot_cmd(lss);
//     add_single_shot_cmd(lss);
//     add_single_shot_cmd(lss);

//     // lss.add(WaitForFW, 1.0);
//     // lss.add(ShootDisk);
//     // lss.add_delay(750);

//     // lss.add(WaitForFW, 1.0);
//     // lss.add(ShootDisk);
//     // lss.add_delay(750);

//     // lss.add(WaitForFW, 1.0);
//     // lss.add(CLEAR_DISKS);

//     // lss.add(DriveForwardFast(4, reverse));
//     //  Wall align
//     //  lss.add(new WallAlignCommand(drive_sys, odometry_sys, bumper_dist, NO_CHANGE, 0, -1, 2.0));

//     // Arrow 3 -------------------------

//     Vector2D::point_t start_of_line = {.x = 34.5, .y = 49};
//     Vector2D::point_t end_of_line = {.x = 65, .y = 82};
//     Vector2D::point_t out_of_way_point = {.x = 73, .y = 118};

//     lss.add(new SpinRPMCommand(flywheel_sys, 3200)); // #40

//     // go to line and collect line
//     lss.add({
//         // DriveToPointFastPtRev(disk_prep_pos1)->withTimeout(2.0), // #27
//         StartIntake,
//         // Start of line
//         TurnToPoint(start_of_line)->withTimeout(2.0), // #39
//         // StartIntake,                       // #40
//         DriveToPointFastPt(start_of_line)->withTimeout(2.0), // #41
//         // DriveForwardFast(2, reverse),      // #42

//         // Drive to End of line
//         TurnToPoint(end_of_line)->withTimeout(2.0),        // #43
//         DriveToPointFastPt(end_of_line)->withTimeout(2.0), // $44

//     });

//     Vector2D::point_t hoop_point = {.x = 17, .y = 113};
//     lss.add(TurnToPoint(hoop_point), 1.0);
//     lss.add(DriveForwardFast(6, reverse));

//     lss.add(new FlapDownCommand());
//     add_single_shot_cmd(lss);
//     add_single_shot_cmd(lss);
//     add_single_shot_cmd(lss);

//     /*
//     lss.add(TurnToPoint(out_of_way_point), 2.5);        // #49
//     lss.add(DriveToPointFastPt(out_of_way_point), 2.5); // #50
//     lss.add(StopIntake);

//     // drive to shooting point
//     Vector2D::point_t shoot_point3 = {.x = 46, .y = 118};
//     lss.add(TurnToPoint(shoot_point3), 1.5);        // #49
//     lss.add(DriveToPointFastPt(shoot_point3), 1.5); // #50
//     // face hoop and fire
//     lss.add(TurnToHeading(190)); // #51

//     lss.add(WaitForFW, 1.0);
//     lss.add(ShootDisk);
//     lss.add_delay(500);

//     lss.add(WaitForFW, 1.0);
//     lss.add(ShootDisk);
//     lss.add_delay(500);

//     lss.add(WaitForFW, 1.0);
//     lss.add(CLEAR_DISKS);


//     Vector2D::point_t backup_point = {.x = 72, .y = 110};
//     lss.add(DriveToPointFastPtRev(backup_point));
//     */

//     // Arrow 4 -------------------------
//     Vector2D::point_t endgame_point = {.x = 116.36, .y = 106.23};

//     lss.add(TurnToPoint(endgame_point), 1.0);        // [measure]
//     lss.add(DriveToPointFastPt(endgame_point), 4.0); //[measure]
//     lss.add(TurnToHeading(48), 3.0);

//     lss.add(new EndgameCommand(endgame_solenoid));
//     lss.add(PrintOdom);

//     return lss;
// }