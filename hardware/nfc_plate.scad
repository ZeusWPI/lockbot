card_width=54;
card_length=86;
card_height=0.8;
$fn=100;
layer_thickness=0.3;

module ccube(size = [1,1,1], center = false)
{
  sz = (len(size) == 3)?size:[size,size,size];
  if (len(center) == 1)
    cube(size, center);
  else
    translate([center[0]?-sz[0]/2:0,center[1]?-sz[1]/2:0,center[2]?-sz[2]/2:0])
    cube(size);
}

module rectangle_plate(w, l, h, r=5) {
    hull() {
        translate([w/2-r, l/2-r, 0]) cylinder(r=r, h=h);
        translate([-w/2+r, l/2-r, 0]) cylinder(r=r, h=h);
        translate([w/2-r, -l/2+r, 0]) cylinder(r=r, h=h);
        translate([-w/2+r, -l/2+r, 0]) cylinder(r=r, h=h);
    }   
}

module card_outline() {
    color([1, 0, 0]) difference(){
        rectangle_plate(card_width+2, card_length+2, 2*layer_thickness);
        translate([0, 0, -1]) rectangle_plate(card_width, card_length, 3);
    }
}

module orange() {
    mirror([0, 1, 0]) {
        translate([0, 0, 2*layer_thickness]) rectangle_plate(card_width*2+10*3, card_length+2*10, layer_thickness, r=5);
        translate([-card_width/2-5, 0, 0]) {
            card_outline();
            linear_extrude(2*layer_thickness) text("Close", font="Liberation Mono", halign="center", valign="center");
        } 
        translate([card_width/2+5, 0, 0]) {
            card_outline();
            linear_extrude(2*layer_thickness) text("Open", font="Liberation Mono", halign="center", valign="center");
        } 
    }
    
}

module black() {
    difference() {
        rectangle_plate(card_width*2+10*3, card_length+2*10, 2*layer_thickness, r=5);
        orange();
    }
}

// orange();
// black();
