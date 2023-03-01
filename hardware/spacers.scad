$fn=50;
cut = 50;
motor_holder_hole_distance_horizontal = 10;

module ccube(size = [1,1,1], center = false)
{
  sz = (len(size) == 3)?size:[size,size,size];
  if (len(center) == 1)
    cube(size, center);
  else
    translate([center[0]?-sz[0]/2:0,center[1]?-sz[1]/2:0,center[2]?-sz[2]/2:0])
    cube(size);
}
()
module bottom_spacer() {
    difference() {
        union() {
            for (i=[-1,1]) {
                translate([i*motor_holder_hole_distance_horizontal/2, 0, 0]) difference() {
                    cylinder(d=7, h=3);
                }   
            }
            ccube([10+7, 10, 3], center=[1, 0,0]);
        }
        union() {
            translate([0, -5, 3-2.5]) ccube([2, 10, cut], center=[1, 0, 0]);
            for (i=[-1,1]) {
                translate([i*motor_holder_hole_distance_horizontal/2, 0, 0]) difference() {
                    cylinder(d=4, h=cut, center=true);
                }   
            }
        }
    }
}


module top_spacer() {
    difference() {
        union() {
            for (i=[-1,1]) {
                translate([i*motor_holder_hole_distance_horizontal/2, 0, 0]) difference() {
                    cylinder(d=7, h=2);
                }   
            }
            ccube([10+7, 10, 2], center=[1, 0,0]);
        }
        union() {
            for (i=[-1,1]) {
                translate([i*motor_holder_hole_distance_horizontal/2, 0, 0]) difference() {
                    cylinder(d=4, h=cut, center=true);
                }   
            }
        }
    }   
}


