$fn=50;
WALL= 0.4*3;
S_WALL= 0.2;
TOLLERANCE= 0.03;

// no belt if 0
BELT_THICKNESS= 0.7;
BELT_W= 20;
BELT_ANGLE= 45;

VOLUME_2= "COMBO"; // "MIN", "MID", "MAX", "COMBO"


BATT_D= 14.5 + TOLLERANCE*2 + BELT_THICKNESS*2;
BATT_L= 50.5 + TOLLERANCE*2;
CAPACITOR_D= 14.5 + TOLLERANCE*2;
CAPACITOR_L= 22 + TOLLERANCE*2;
ESP_W= 26 + TOLLERANCE*2;
ESP_L= 35 + TOLLERANCE*2;
ESP_H= 10 + TOLLERANCE*2;
ESP_RH= 2;

CONT_D= 1.2;
CONT_M= 5 - CONT_D;
CONT_P= 3 - CONT_D;

BUTTON_PLATFORM= 10;
BUTTON_GAP= 4.2;
BUTTON_PRESS= 4;
BUTTON_OVERLAP= 0.4;
BUTTON_WALL_GAP=4;

LED_WINDOW= 12;



BOX_W= max(5*WALL + BATT_D + CAPACITOR_D,
           3*WALL + ESP_W);
BOX_L= max(4*WALL + BATT_L,
           2*WALL + CAPACITOR_L,
           2*WALL + ESP_L);
BOX_H= 4*WALL + max(BATT_D, CAPACITOR_D) + ESP_H;

BUTTON_WINDOW_L= (BUTTON_WALL_GAP*2 + BUTTON_PLATFORM -2*WALL);
BUTTON_WINDOW_W= BOX_W-6*WALL;

// cutting tool for cut_cube
module cuting_side(x,y,a)
{
    h= x*tan(a);
    rotate([90,0,0])
    linear_extrude(height=y, center=true)
    polygon(points = [[0,0],[x,0],[x,h]]);
}

// cube with well-printed "supports"
module cut_cube(dim, angle)
{
    down= max(dim[0], dim[1])/2*tan(angle);
    h= dim[2] + down;
    translate([0, 0 , +h/2 -(down + dim[2]/2)])
    difference()
    {
        cube([dim[0], dim[1], h], center=true);
        translate([0, 0 , -h/2])
        cuting_side(x= dim[0]/2, y= dim[1], a=angle);
        translate([0, 0 , -h/2])
        mirror([1,0,0])
        cuting_side(x= dim[0]/2, y= dim[1], a=angle);
        translate([0, 0 , -h/2])
        rotate([0,0,90])
        cuting_side(x= dim[1]/2, y= dim[0], a=angle);
        translate([0, 0 , -h/2])
        mirror([0,1,0])
        rotate([0,0,90])
        cuting_side(x= dim[1]/2, y= dim[0], a=angle);
    }
}

// long and overcomplicated way to make a barierr
module cubic_barrier(h,l,r,cut)
{
    hh= sqrt(h*h/2);
    difference()
    {
        translate([0,0,h/2])
        rotate([0,45,0])
        cube([hh,l,hh], center= true);
        if (cut)
        {
            translate([(r? -hh : 0),-(l+1)/2,-1])
            cube([hh,l+1,h+2]);
        }
    }
}

// something 1-like shaped
module limit_w_barrier(w,l,h1,h2,r)
{
    translate([-w/2,-l/2,0])
    cube([w,l,h1+h2]);
    translate([(r?w/2:-w/2),0,h1])
    cubic_barrier(h=h2,l=l,r=r, cut=false);
}

// Cage for 6x6mm button
module button_66cage(platform, gap)
{
    // 6x6mm button parameters
    BUTTON_SL= 3.5;
    BUTTON_S= 6;
    BUTTON_SH= 4;
    BUTTON_B= 0.5;
    translate([0, 0, gap/2])
    cut_cube([platform, BUTTON_SL, gap +0.01], angle=45);
    translate([0, 0, gap/2])
    cut_cube([BUTTON_SL, platform, gap +0.01], angle=45);
        
    // real botton cage
    barier= (platform-BUTTON_S)/2 - TOLLERANCE;
        
    translate([(platform/2 - barier/2), 0, gap - 0.01])
    limit_w_barrier(l= BUTTON_SL, w= barier, h1= BUTTON_SH + 0.01, h2=BUTTON_B,r=false);

    translate([-(platform/2 - barier/2), 0, gap - 0.01])
    limit_w_barrier(l= BUTTON_SL, w= barier, h1= BUTTON_SH + 0.01, h2= BUTTON_B, r= true);
        
    translate([0, (platform/2 - barier/2), gap - 0.01])
    rotate(90)
    limit_w_barrier(l= BUTTON_SL, w= barier,h1= BUTTON_SH + 0.01, h2= BUTTON_B, r= false);
        
    translate([0, -(platform/2 - barier/2), gap - 0.01])
    rotate(90)
    limit_w_barrier(l= BUTTON_SL, w= barier,h1= BUTTON_SH + 0.01, h2= BUTTON_B, r= true);
}

// D shaped cut to put cylinder in it
module cylinder_cut(d, l, h)
{
    cylinder(d=d, h=l, center= true);
    translate([0, -h/2, 0])
    cube([d, h, l], center= true);
}

// "swift-tail" :)
module s_tail(w1,w2,l,h)
{
    rotate([90,0,0])
    linear_extrude(height=l, center=true)
    polygon(points = [[-w1/2,-h/2],
                      [-w2/2,+h/2],
                      [+w2/2,+h/2],
                      [+w1/2,-h/2]]);
}

// slider-cover shape
module slider_cover(h, w, l, top)
{
    ww= w + 2*h;
    difference()
    {
        union()
        {
            translate([0, 0, h/2])
            if (top)
                s_tail(w1=w, w2=ww, l=l, h=h);
            else
                s_tail(w1=ww, w2=w, l=l, h=h);
            translate([0, +h - l/2, -h/2 +0.01])
            cube([ww, h*2, h], center=true);
            translate([0, -h + l/2, -h/2 +0.01])
            cube([ww, h*2, h], center=true);
            translate([0, 0, -h/2 +0.01])
            cube([w, l - h*4, h], center=true);
        }
        translate([-h/2 + ww/2, -h + l/2, 0])
        cube([h, h*2, 8*h], center=true);
        translate([+h/2 - ww/2, -h + l/2, 0])
        cube([h, h*2, 8*h], center=true);
    }
}

// Cut for battery contacts
module battery_cont(cd, centers_d, ch, cut)
{
    translate([+centers_d/2, 0, 0])
    cylinder(d= cd, h= ch, center=true);
    translate([-centers_d/2, 0, 0])
    cylinder(d= cd, h= ch, center=true);
    translate([0, 0, +cut/2 - ch/2])
    cube([centers_d, cd, cut], center=true);
}

// The box without cuts for its battery compartment
module button_top_cut_box()
{
    battery_comp= 3*WALL + BATT_D;
    difference()
    {
        // box
        cube([BOX_W, BOX_L, BOX_H], center= true);

        // Cut for electronics
        translate([0, 0, -(BOX_H - battery_comp)/2 + BOX_H/2 + 0.01])
        cube([BOX_W - 4*WALL, BOX_L - 2*WALL, BOX_H -  battery_comp], center= true);
        translate([+(CAPACITOR_D/2) - (BOX_W/2 - 2*WALL), 0, - (CAPACITOR_D/2 + BOX_H/2) + (VOLUME_2 == "MIN" ? (battery_comp + ESP_RH) : (CAPACITOR_D + WALL))])
        if (VOLUME_2 == "MIN" || VOLUME_2 == "MID")
        {
            rotate([-90, 0, 0])
            cylinder_cut(d= CAPACITOR_D, l= BOX_L - 2*WALL, h= BOX_H);
        }
        else
        {
            //if there is belt, lets mame its cut even
            cl= min(CAPACITOR_L, (BELT_THICKNESS > 0 ? (BOX_L - 2*WALL - BELT_W - 2*WALL)/2 : BOX_L));
            l= BOX_L - 2*WALL - (VOLUME_2 == "MAX" ? 0: cl);
            translate([ 0, (VOLUME_2 == "MAX" ? 0 : - -cl/2), BOX_H/2 - CAPACITOR_D/2])
            difference()
            {
                cube([CAPACITOR_D, l, BOX_H], center=true);
                translate([ CAPACITOR_D/4, 0, -BOX_H/2])
                rotate([90,0,0])
                linear_extrude(height= l, center=true)
                polygon(points = [[0,0],[CAPACITOR_D/4, 0],[CAPACITOR_D/4,CAPACITOR_D/4]]);
            }
            if (VOLUME_2 != "MAX")
            {
                translate([0, -l/2 +WALL/2, (battery_comp + ESP_RH)
                - (CAPACITOR_D + WALL)])
                rotate([-90, 0, 0])
                cylinder_cut(d= CAPACITOR_D, l= cl+ WALL, h= BOX_H);
            }
        }
        
    }
    translate([0, BOX_L/2- WALL - BUTTON_PLATFORM/2 -BUTTON_GAP, -BOX_H/2 + BATT_D + 3*WALL])
    button_66cage(platform= BUTTON_PLATFORM, gap=BUTTON_GAP);
}

// The box body
module button_box()
{
    difference()
    {
        button_top_cut_box();

        // Battery compartment
        translate([-(BATT_D/2) + (BOX_W/2 - 2*WALL), 0, (BATT_D/2) - (BOX_H/2 - 2*WALL)])
        rotate([90, 0, 0])
        cylinder_cut(d= BATT_D, l= BATT_L, h= BATT_D/2+ 2*WALL + 0.01);
        // Cut for battery cover
        translate([-(BATT_D + 2*WALL)/2 + BOX_W/2 - WALL, 0, +WALL - BOX_H/2])
        slider_cover(h= WALL+TOLLERANCE*2, w= BATT_D, l= BOX_L, top= false);
        // top cover
        translate([0, 0, -WALL + BOX_H/2])
        rotate([0, 180, 0])
        slider_cover(h= WALL+TOLLERANCE*2, w= BOX_W-4*WALL, l= BOX_L, top= true);
        // Cut battery contacts
        translate([ (BOX_W/2 - 2*WALL) - BATT_D/2, +(BATT_L + CONT_D)/2 - 0.01, 0])
        battery_cont(cd= CONT_D, centers_d= CONT_P, ch=BOX_H-8*WALL, cut= BATT_D - 4*WALL);
        translate([ (BOX_W/2 - 2*WALL) - BATT_D/2, -(BATT_L + CONT_D)/2 + 0.01, 0])
        battery_cont(cd= CONT_D, centers_d= CONT_M, ch=BOX_H-8*WALL, cut= BATT_D - 4*WALL);
        // Minus
        translate([-(BOX_W/2 - 2*WALL) + BATT_D/2, -BOX_L/4, -BOX_H/2 + S_WALL/2 -0.01])
        cube([WALL, 3*WALL, S_WALL], center=true);
        // Plus
        translate([-(BOX_W/2 - 2*WALL) + BATT_D/2, +BOX_L/4, -BOX_H/2 + S_WALL/2 -0.01])
        cube([3*WALL, WALL, S_WALL], center=true);
        translate([-(BOX_W/2 - 2*WALL) + BATT_D/2, +BOX_L/4, -BOX_H/2 + S_WALL/2 -0.01])
        cube([WALL, 3*WALL, S_WALL], center=true);

        if (BELT_THICKNESS > 0)
        {
            l= (2*WALL + CAPACITOR_D/2)/cos(BELT_ANGLE);
            translate([0, 0, -BOX_H/2 + 3*WALL + BELT_THICKNESS/2])
            rotate([0, BELT_ANGLE, 0])
            translate([-l/2 + 2*WALL, 0, 0])
            cube([l, BELT_W, BELT_THICKNESS], center=true);
        }
    }
}

// half-cylinder
module h_cylinder(d,h)
{
    difference()
    {
        cylinder(d=d, h=h, center=true);
        translate([0, d/2, 0])
        cube([d,d,2*h],center=true);
    }
}

module battery_cover()
{
    difference()
    {
        rotate([0, 180, 0])
        slider_cover(h= WALL, w= BATT_D, l= BOX_L, top= false);
        // cuts to make sliding easy
        translate([0, +(BOX_L/2 - 2*WALL), WALL/2 +0.01])
        h_cylinder(d= BATT_D - 2*WALL,h=WALL);
        translate([0, -(BOX_L/2 - 2*WALL), WALL/2 +0.01])
        rotate([0, 0, 180])
        h_cylinder(d= BATT_D - 2*WALL,h=WALL);
    }
}

module top_cover()
{
    h= WALL*2 - S_WALL;
    difference()
    {
        union()
        {
            difference()
            {
                slider_cover(h= WALL, w= BOX_W-4*WALL, l= BOX_L, top= true);
                // window for the button press area
                translate([0, BOX_L/2- WALL - BUTTON_PLATFORM/2 -BUTTON_GAP, WALL*2-h])
                cube([BUTTON_WINDOW_W, BUTTON_WINDOW_L, h], center= true);
                // window for the ESP8266 diode
                translate([0, -BOX_H*2/3, WALL*2-h])
                cube([LED_WINDOW, LED_WINDOW, h], center= true);
            }
            // cylinder which press the button
            translate([0, BOX_L/2- WALL - BUTTON_PLATFORM/2 -BUTTON_GAP, -WALL])
            cylinder(d= BUTTON_PRESS, h= WALL*2-        BUTTON_OVERLAP, center=false);
        }
        // press pointer
        translate([0, BOX_L/2- WALL - BUTTON_PLATFORM/2 -BUTTON_GAP, -WALL])
        cylinder(d1= min(BUTTON_PRESS- 2*S_WALL, (WALL*2 - BUTTON_OVERLAP - WALL)*2), d2=0, h= WALL*2 - BUTTON_OVERLAP - WALL, center=false);
    }
}

button_box();
translate([BOX_W, 0, +WALL -BOX_H/2])
battery_cover();
translate([-BOX_W, 0, +WALL -BOX_H/2])
top_cover();
