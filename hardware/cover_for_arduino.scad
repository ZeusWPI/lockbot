$fn=100;

width=75;
inner_height=80;
inner_depth=50;

module ccube(size = [1,1,1], center = false)
{
  sz = (len(size) == 3)?size:[size,size,size];
  if (len(center) == 1)
    cube(size, center);
  else
    translate([center[0]?-sz[0]/2:0,center[1]?-sz[1]/2:0,center[2]?-sz[2]/2:0])
    cube(size);
}

// deksel (afneembaar)
difference() {
    union() {
        ccube([width, inner_height+4, inner_depth+2], center=[1, 1, 0]);
        translate([0, inner_height/2+2-0.5, 5]) rotate([0, 90, 0]) translate([0, 0, -25]) cylinder(d=5, h=50);
        translate([0, -inner_height/2-2+0.5, 5]) rotate([0, 90, 0]) translate([0, 0, -25]) cylinder(d=5, h=50);
    }
    translate([0, 0, -1]) ccube([width+2, inner_height, inner_depth+1], center=[1, 1, 0]);
}

// houder (2x printen, wordt in deur gevezen)
difference() {
    ccube([width, 10, 10], center=[1, 1, 0]);
    union() {
        translate([0, 5, 5]) rotate([0, 90, 0]) translate([0, 0, -25.5]) cylinder(d=5.5, h=51);
        translate([width/2-5, 0, 5.1]) cylinder(h=5, r1=4.1/2, r2=7.9/2);
        translate([width/2-5, 0, -0.5]) cylinder(h=6, r=4.1/2);
        translate([-width/2+5, 0, -0.5]) cylinder(h=6, r=4.1/2);
        translate([-width/2+5, 0, 5.1]) cylinder(h=5, r1=4.1/2, r2=7.9/2);
    }
}
