use <gears.scad>;

$fn=200;
width_back=33 + 1;
width_front=31 + 2;
height_front=14.5 + 2;
depth=17.8;


big_gear_teeth=25;
small_gear_teeth=15;
modul=2;
gear_distance=(big_gear_teeth+small_gear_teeth)*modul/2;



// knob can turn 1.5 turns in total, let's make that 2
// potentiometer turns 10 turns

potentiometer_shaft_diameter=6.35 + 0.5;
potentiometer_shaft_depth=7;
potentiometer_groove_width=0.6;
potentiometer_groove_depth=1;


// 13 clearance from width_back to bar keeping door in place

chest_bottom=7 + 2;

module centered_cube(size = [1,1,1], center = false)
{
  sz = (len(size) == 3)?size:[size,size,size];
  if (len(center) == 1)
    cube(size, center);
  else
    translate([center[0]?-sz[0]/2:0,center[1]?-sz[1]/2:0,center[2]?-sz[2]/2:0])
    cube(size);
}


module knob() {
  linear_extrude(height = depth, convexity = 10, scale=width_front/width_back)
  polygon(points=[[-chest_bottom/2,-width_front/2],[chest_bottom/2,-width_front/2],[height_front/2,0],[chest_bottom/2,width_front/2],[-chest_bottom/2,width_front/2], [-height_front/2,0]]);
}

module main_gear() {
  difference() {
    union() {
      spur_gear(modul, big_gear_teeth, depth+6, width_front+2, optimized=false);
      cylinder(r=width_back/2+5, h=depth+6);
    }

    union() {
      scale([1.3, 1.2, 1]) translate([0, 0, -0.01]) knob();
      translate([0, 0, depth-0.02]) centered_cube(size=[40, 13, 20], center=[1, 1, 0]);
      translate([-13, 0, 0]) cylinder(r=0.7, h=100);
      translate([13, 0, 0]) cylinder(r=0.7, h=100);
    }
  }

}

module small_gear() {
  extra_height=15;
  spur_gear(modul, small_gear_teeth, potentiometer_shaft_depth+extra_height, potentiometer_shaft_diameter);
  cylinder(r=5, h=extra_height-potentiometer_groove_depth);
  color([1, 0.5, 0]) intersection() {
    cylinder(r=5, h=extra_height);
    translate([0, 0, extra_height-potentiometer_groove_depth]) centered_cube([10, potentiometer_groove_width, potentiometer_groove_depth], center=[true, true, false]);
  }
}

module support() {
  difference() {
    union() {
      minkowski() {
        centered_cube([20, 20 + 10, 3], [1, 0, 0]);
        cylinder(r=2, h=1);
      }
    }
    union() {
      // Hole for potentiometer
      translate([0, 10, -1]) cylinder(h=20, r=11/2);
      // Holes for M3 screws
      translate([-5, 25, -1]) cylinder(h=20, d=3.5);
      translate([5, 25, -1]) cylinder(h=20, d=3.5);
    }
  }
}


/* main_gear(); */
/* translate([0, gear_distance, depth-potentiometer_shaft_depth+1]) small_gear(); */
translate([50, 0, 0]) support();
