$fn=360;

//chapa para la base del ventilador de portatil tacens

//glass
glass_height=1.5;
*hull() {
    cylinder(r=2, h=glass_height);
    translate([13,0,0]) cylinder(r=2, h=glass_height);
    translate([0,40.5,0]) cylinder(r=2, h=glass_height);
    translate([13,40.5,0]) cylinder(r=2, h=glass_height);
}

//mask
mask_height = 0.9;
difference() {
    hull() {
        cylinder(r=2, h=mask_height);
        translate([13,0,0]) cylinder(r=2, h=mask_height);
        translate([0,40.5,0]) cylinder(r=2, h=mask_height);
        translate([13,40.5,0]) cylinder(r=2, h=mask_height);
    }
    union() {
        translate([12,17.25,-1]) rotate([0,0,90]) linear_extrude(height = mask_height+2, center = false, convexity = 10, twist = 0, slices = 20, scale = 1.0) text("k",11,"Cardinal:style=Regular");
    }
}