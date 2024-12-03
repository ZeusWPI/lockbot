$fn=100;

heatset_height = 5.8 + 0.2;
heatset_diam = 3.9 + 0.1;

m3_clearance=3.6;

wall=10;

cut=100;

inner_width = 85;
inner_length = 150;
inner_height = 45;

cover_wall = 4;

module ccube(size = [1,1,1], center = false)
{
  sz = (len(size) == 3)?size:[size,size,size];
  if (len(center) == 1)
    cube(size, center);
  else
    translate([center[0]?-sz[0]/2:0,center[1]?-sz[1]/2:0,center[2]?-sz[2]/2:0])
    cube(size);
}

module door_mount() {
    difference() {
        cube([wall+inner_length+wall, wall+inner_width+wall, wall]);
        union() {
            translate([wall, wall, -1]) cube([inner_length, inner_width, cut]);
            translate([-1, (2*wall+inner_width)/2 - 20/2, 1]) cube([50, 20, cut]);
            translate([inner_length, wall, -1]) cube([50, 20, 8+1]);

            for (dx = [0, wall+inner_length]) {
                for (dy = [0, wall+inner_width]) {
                    translate([wall/2+dx, wall/2+dy, 0]) union() {
                        translate([0, 0, wall-heatset_height]) cylinder(d=heatset_diam, h=10);
                        translate([0, 0, -1]) cylinder(d=m3_clearance, h=wall-heatset_height+1);
                    }
                }
            }

            for (dx = [20, wall+inner_length-20]) {
                for (dy = [0, wall+inner_width]) {
                    translate([wall/2+dx, wall/2+dy, 0]) union() {
                        translate([0, 0, -0.5-1]) cylinder(h=6, r=4.1/2);
                        translate([0, 0, 5.1-1]) cylinder(h=6, r1=4.1/2, r2=(4.1 + 0.76*6)/2);
                    }
                }
            }
        }
    }
}

module cover() {
    difference() {
        union() {
            cube([wall+inner_length+wall, wall+inner_width+wall, 2]);
            translate([wall-cover_wall, wall-cover_wall,0]) cube([cover_wall+inner_length+cover_wall, cover_wall+inner_width+cover_wall, inner_height+cover_wall]);
        }
        union() {
            translate([wall, wall, -1]) cube([inner_length, inner_width, inner_height+1]);
            translate([-1, (2*wall+inner_width)/2 - 20/2, -1]) cube([wall+1+0.5, 20, 35-wall+1]);
            for (dx = [0, wall+inner_length]) {
                for (dy = [0, wall+inner_width]) {
                    translate([wall/2+dx, wall/2+dy, 0]) union() {
                        translate([0, 0, 2]) cylinder(d=10, h=cut);
                        translate([0, 0, -1]) cylinder(d=3.6, h=cut);
                    }
                }
            }
        }
    }
    

    %union() {
        for (dx = [0, wall+inner_length]) {
                for (dy = [0, wall+inner_width]) {
                    translate([wall/2+dx, wall/2+dy, 0]) union() {
                        color([0.5, 0.5, 0.5]) translate([0, 0, 0]) cylinder(d=6, h=10);
                    }
                }
        }
    }
    
}

door_mount();
translate([0, 0, wall+1]) cover();
