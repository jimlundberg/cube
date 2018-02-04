// coordconv,h : coordinate conversion
//
// This could just be in the glbox.cpp file, but I wanted to separate things up
// since I have all these functions that just serve to convert from 
// one numbering system to another. 
//
// "game coordinates" refer to the coordinates of the board as defined by the 
// tic tac toe engine (engine.cpp).  Game coordinates are 0 to 3 with x going
// from left to right, y going from back to front, and z going from the top board 
// to the bottom board. 
// "GL coordinates" are the actual x,y,z positions for drawing something in the 
// 3-d OpenGL environment. 
//
// John Davin, Terrence Wong, Jim Lundberg
// Copyright (c) 2002-2003 JRL Innovations  All Rights Reserved
// 3D5 - The FIVE level 3D tic-tac-toe game
//

#ifndef COORCONV_H
#define COORCONV_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "glbox.h"

class CoordConv
{
public:
	static void     mapGamecoord2gl( int game_x, int game_y, int game_z, double &x, double &y, double &z );
	static void     mapsqr2gamecoord( int sqrnum, int &x, int &y, int &z );
	static void     mapsqr2coord( int sqrnum, double &x, double &y, double &z );
	static Point*   mapGame2gl( Point* game ); // returns the point in GL coordinates
	static void     mapGL2gamecoord( double x, double y, double z, int &game_x, int &game_y, int &game_z );
};

#endif
