// This is the module that attaches the motor to the door
// all units in mm

include <lockbot.scad>;

$fn=100;


module ccube(size = [1,1,1], center = false)
{
  sz = (len(size) == 3)?size:[size,size,size];
  if (len(center) == 1)
    cube(size, center);
  else
    translate([center[0]?-sz[0]/2:0,center[1]?-sz[1]/2:0,center[2]?-sz[2]/2:0])
    cube(size);
}


motor_holder_to_door_distance = 53;
gear_to_door_distance=45;

motor_holder_hole_distance_horizontal = 10;
motor_holder_hole_diameter = 4;
motor_holder_hole_distance_vertical = 49.5;
motor_case_vertical = 40;
motor_case_horizontal = 20;
distance_shaft_to_case = 30; // or 10, from other side
cut=100;

module foot() {
    difference() {
        ccube([10, 10, 5], center=[1, 1, 0]);
        union() {
            translate([0, 0, -1]) cylinder(d=4.5, h=cut);
        }
    }
    
    
}

module_vertical=60;
module_horizontal=100;

module attachment_to_door() {
    difference() {
    translate([15, -10, 0]) union() {

        ccube([module_horizontal, module_vertical, motor_holder_to_door_distance], center=[true, true, false]);
        translate([-module_horizontal/2-5, -module_vertical/2+5, 0]) foot();
        translate([module_horizontal/2+5, -module_vertical/2+5, 0]) foot();
        translate([-module_horizontal/2-5, module_vertical/2-5, 0]) foot();
        translate([module_horizontal/2+5, module_vertical/2-5, 0]) foot();
    }
    union() {
        translate([0, 0, -1]) {
            translate([15, -10, 0]) ccube([module_horizontal-10, module_vertical+10, gear_to_door_distance+4+1], center=[1, 1, 0]);
        }
        translate([gear_distance, 0, -1]) cylinder(d=11, h=cut);
        translate([0, -10, -1]) ccube([motor_case_horizontal+1, motor_case_vertical+1, cut], center=[1, 1, 0]);
        translate([motor_holder_hole_distance_horizontal/2, -(distance_shaft_to_case+5), -1]) cylinder(d=4, h=cut);
        translate([-motor_holder_hole_distance_horizontal/2, -(distance_shaft_to_case+5), -1]) cylinder(d=4, h=cut);
        translate([motor_holder_hole_distance_horizontal/2, (motor_case_vertical-distance_shaft_to_case)+5, -1]) cylinder(d=4, h=cut);
        translate([-motor_holder_hole_distance_horizontal/2, (motor_case_vertical-distance_shaft_to_case)+5, -1]) cylinder(d=4, h=cut);
        translate([0, -10, motor_holder_to_door_distance-2]) ccube([2, 50, cut], center=[1, 1, 0]);
    }
}
% translate([0, 0, 45-25]) main_gear();
% translate([gear_distance, 0, 45-25]) small_gear();
}
