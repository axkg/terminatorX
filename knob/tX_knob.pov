// terminatorX - realtime audio scratching software
// Copyright (C) 1999-2004  Alexander König <alex@lisas.de>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
// 
// This is a povray scene that renders a knob for the terminatorX user
// interface.

// -w320 -h240 -icon
// -w1600 -h1280 +a0.3

#include "colors.inc"
#include "metals.inc"
#include "textures.inc"

global_settings { assumed_gamma 2.2 }

#declare chrome = texture { T_Chrome_2E }
#declare silver = texture { Silver_Metal }

#declare knob_height=2;

#declare black_plastic =
texture {
    pigment { color rgb <0, 0, 0> }
    finish {ambient 0.1 diffuse 0.8 phong 0.5 phong_size 100 }
}

#declare orange_plastic =
texture {
    pigment { color rgb <1, 0.2, 0> }
    finish {ambient 0.1 diffuse 0.8 phong 0.5 phong_size 100 }
}

#declare white_plastic =
texture {
    pigment { color rgb <1, 1, 1> }
    finish {ambient 0.1 diffuse 0.8 phong 0.5 phong_size 100 }
}

#if(KNOB_VIEW=1)
  camera { orthographic location -z*3.3 look_at 0 }
#else
	camera { location <+1.6,-1,-4> look_at 0 }
//  camera { orthographic location -x*5 look_at 0 }
#end

plane {
  z, 0.01
  hollow on
#if(KNOB_VIEW=1)
  pigment {BACKGROUND}
#else
  texture { chrome }
#end
}

light_source { <-60, 80, -500> color Gray85}
light_source { <50,  10, -900> color Gray65}

#declare zyl=0;

union {
	difference {
		intersection {
			union {
				cylinder {<0,0,0>,<0,0,-knob_height>,1}

#declare w=0.2;
#declare h=2.2;

#declare A = 0;
#while (A<9)
				box {
					<-w/2,-h/2,0>,<w/2,h/2,-knob_height>
					rotate z*(A*20)
				}
#declare A=A+1;
#end
			}

			sphere {
				<0,0,0>,knob_height
			}
		
		}
#if(zyl=1)		
		cylinder {<0,0,0>,<0,0,-2*knob_height>,0.75}
#end
		
		texture { silver }
	}

	difference {
		cylinder {<0,0,0>,<0,0,-knob_height>,0.9}
		cylinder {<0,0,-1>,<0,0,-knob_height-1>,0.75}
		texture { silver }
	}

#if(zyl=1)
	intersection {
#if(1=1)	
		cylinder {<0,-1,-.7>,<0,1,-.7>,1.3}
#else
		union {
#declare A = 0;
#while (A<9)
			torus {
				A*0.1, 0.1
				rotate -90*x
				translate <0,0,-knob_height*0.8>
			}
#declare A=A+1;
#end
		}
#end
		cylinder {<0,0,0>,<0,0,-2*knob_height>,0.75}
		texture { silver }
	}
#end
  
	box {
		<-0.1,0,0>,<0.1,1.1,-knob_height-.01>
		texture {orange_plastic}
	}

	rotate z*ROT_ANGLE
	MASKOPTION
}

difference {
	cylinder {
		<0,0,0>,<0,0,-0.005>,1.5
	}
	
	cone {
		<0, 0, 0>, 0    // Center and radius of one end
		<0, -1.5, 0>, 0.85    // Center and radius of other end
	}
	
	texture {black_plastic} 
}

union {
#declare tiny_height=0.25;
#declare tiny_width=0.1;

#declare big_height=0.3;
#declare big_width=0.2;

#declare A = 1;
#while (A<16)
	box {
#if((A=1) | (A=8) | (A=15))
		<-big_width/2,-big_height/2,-0.01>,<big_width/2,big_height/2,-0.01>
#else
		<-tiny_width/2,-tiny_height/2,-0.01>,<tiny_width/2,tiny_height/2,-0.01>
#end
		translate <0,1.3,0>
		rotate z*(A*20+20+180)
	}
#declare A=A+1;
#end
		texture {white_plastic} 
}
