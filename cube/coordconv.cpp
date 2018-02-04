// coordconv.cpp - Coordinate conversion
//
// This could just be in the glbox.cpp file, but I wanted to separate things up
// since I have all these functions that just serve to convert from 
// one numbering system to another. 
//
// John Davin, Terrence Wong, Jim Lundberg
// Copyright (c) 2002-2003 JRL Innovations  All Rights Reserved
// 3D5 - The FIVE level 3D tic-tac-toe game
//

#include "StdAfx.h"
#include "coordconv.h"

//
// Maps a square number to x,y,z coordinates
//
void CoordConv::mapsqr2coord( int BoardPositionNumber, double &x, double &y, double &z )
{
    // CALCULATE Y 
    int boardnum;
    if (BoardPositionNumber <= NUM_HOLES_PER_LEVEL - 1)
        boardnum = 0;
    else if (BoardPositionNumber <= (NUM_HOLES_PER_LEVEL * 2) - 1)
        boardnum = 1;
    else if (BoardPositionNumber <= (NUM_HOLES_PER_LEVEL * 3) - 1)
        boardnum = 2;
    else if (BoardPositionNumber <= (NUM_HOLES_PER_LEVEL * 4) - 1)
        boardnum = 3;
    else 
        boardnum = 4;

    switch (boardnum)
    {
    case 0:
        y = BOARD0_H; // height of board 0
        break;

    case 1:
        y = BOARD1_H;
        break;

    case 2:
        y = BOARD2_H;
        break;

    case 3:
        y = BOARD3_H;
        break;

    case 4:
        y = BOARD4_H;
        break;

    default:
        printf("[mapsqr2coord]: ERROR: encountered unknown board number\n");
        break;
    }

    // CALCULATE X
    int blah = BoardPositionNumber % NUM_HOLES_PER_LINE;
    switch (blah)
    {
    case 0:
        x = BOXSIZE / 2.0;
        break;

    case 1:
        x = BOXSIZE + BOXSIZE / 2.0; 
        break;

    case 2:
        x = 2.0 * BOXSIZE + BOXSIZE / 2.0;
        break;

    case 3:
        x = 3.0 * BOXSIZE + BOXSIZE / 2.0;
        break;

    case 4:
        x = 4.0 * BOXSIZE + BOXSIZE / 2.0;
        break;
    }

    // CALCULATE Z
    int index = BoardPositionNumber % NUM_HOLES_PER_LEVEL;

    if (index <= 4)
        z = BOXSIZE / 2.0;
    else if (index <= 9)
        z = BOXSIZE + BOXSIZE / 2.0;
    else if (index <= 14)
        z = 2.0 * BOXSIZE + BOXSIZE / 2.0;
    else if (index <= 19)
        z = 3.0 * BOXSIZE + BOXSIZE / 2.0;
    else
        z = 4.0*BOXSIZE + BOXSIZE / 2.0;
}

//
// Maps a square number to x,y,z indices for the game engine
// x,y,z range from 0 to 3. 
//
void CoordConv::mapsqr2gamecoord( int BoardPositionNumber, int &x, int &y, int &z )
{
    // CALCULATE Y 
    int boardnum;

    if (BoardPositionNumber <= NUM_HOLES_PER_LEVEL - 1)
        boardnum = 0;
    else if (BoardPositionNumber <= (NUM_HOLES_PER_LEVEL * 2) - 1)
        boardnum = 1;
    else if (BoardPositionNumber <= (NUM_HOLES_PER_LEVEL * 3) - 1)
        boardnum = 2;
    else if (BoardPositionNumber <= (NUM_HOLES_PER_LEVEL * 4) - 1)
        boardnum = 3;
    else 
        boardnum = 4;

    switch (boardnum)
    {
    case 0:
        z = 0; // height of board 0
        break;

    case 1:
        z = 1;
        break;

    case 2:
        z = 2;
        break;

    case 3:
        z = 3;
        break;

    case 4:
        z = 4;
        break;

    default:
        printf("[mapsqr2gamecoord]: ERROR: encountered unknown board number\n");
        break;
    }

    // CALCULATE X
    int blah = BoardPositionNumber % NUM_HOLES_PER_LINE;
    switch (blah)
    {
    case 0:
        x = 0;
        break;

    case 1:
        x = 1; 
        break;

    case 2:
        x = 2;
        break;

    case 3:
        x = 3;
        break;

    case 4:
        x = 4;
        break;
    }

    // CALCULATE Z
    int index = BoardPositionNumber % NUM_HOLES_PER_LEVEL;
    if (index < NUM_HOLES_PER_LINE)
        y = 0;
    else if (index < NUM_HOLES_PER_LINE * 2)
        y = 1;
    else if (index < NUM_HOLES_PER_LINE * 3)
        y = 2;
    else if (index < NUM_HOLES_PER_LINE * 4)
        y = 3;
    else
        y = 4;
}

//
// returns the point in GL coordinates
// Note that the input (Point game) has parameters stored as doubles, but 
// they must be integers since it is in game coordinates (0 to 3). 
//
Point* CoordConv::mapGame2gl( Point *game )
{
    double x,y,z;

    mapGamecoord2gl( (int)game->x, (int)game->y, (int)game->z, x, y, z );

    Point *p = new Point( x, y, z );
    return p;
}

//
// Maps game coordinates (0 to 3) to 3d gl coordinates
//
void CoordConv::mapGamecoord2gl( int game_x, int game_y, int game_z, double &x, double &y, double &z )
{
    // CALCULATE Y 
    switch (game_z)
    {
    case 0:
        y = BOARD0_H; // height of board 0
        break;

    case 1:
        y = BOARD1_H;
        break;

    case 2:
        y = BOARD2_H;
        break;

    case 3:
        y = BOARD3_H;
        break;

    case 4:
        y = BOARD4_H;
        break;

    default:
        printf("[mapGamecoord2gl]: ERROR: encountered unknown board number %d\n", game_z);
        break;
    }

    // CALCULATE X
    switch (game_x)
    {
    case 0:
        x = BOXSIZE / 2.0;
        break;

    case 1:
        x = 1.0 * BOXSIZE + BOXSIZE / 2.0; 
        break;

    case 2:
        x = 2.0 * BOXSIZE + BOXSIZE / 2.0;
        break;

    case 3:
        x = 3.0 * BOXSIZE + BOXSIZE / 2.0;
        break;

    case 4:
        x = 4.0 * BOXSIZE + BOXSIZE / 2.0;
        break;

    default:
        printf("[mapGamecoord2gl]: ERROR: encountered unknown board number %d\n", game_x);
        break;
    }

    // CALCULATE Z
    switch (game_y)
    {
    case 0:
        z = BOXSIZE / 2.0;
        break;

    case 1:
        z = 1.0 * BOXSIZE + BOXSIZE / 2.0; 
        break;

    case 2:
        z = 2.0 * BOXSIZE + BOXSIZE / 2.0;
        break;

    case 3:
        z = 3.0 * BOXSIZE + BOXSIZE / 2.0;
        break;

    case 4:
        z = 4.0 * BOXSIZE + BOXSIZE / 2.0;
        break;

    default:
        printf("[mapGamecoord2gl]: ERROR: encountered unknown board number %d\n", game_y);
        break;
    }
}

//
// Maps 3d gl coordinates to game coordinates (0 to 3)
//
void CoordConv::mapGL2gamecoord(double x, double y, double z, int &game_x, int &game_y, int &game_z)
{
    // CALCULATE Y 
    if (y ==  BOARD0_H)
        game_z = 0; // height of board 0
    else if (y == BOARD1_H )
        game_z = 1;
    else if (y == BOARD2_H)
        game_z = 2;
    else if (y == BOARD3_H)
        game_z = 3;
    else if (y == BOARD4_H)
        game_z = 4;
    else
        printf("[mapGL2gamecoord]: ERROR: encountered unknown board height %f\n", y);

    // CALCULATE X
    if (x == (BOXSIZE / 2.0))
        game_x = 0;
    else if (x == (1.0 * BOXSIZE + BOXSIZE / 2.0))
        game_x = 1; 
    else if (x == (2.0 * BOXSIZE + BOXSIZE / 2.0))
        game_x = 2;
    else if (x == (3.0 * BOXSIZE + BOXSIZE / 2.0))
        game_x = 3;
    else if (x == (4.0 * BOXSIZE + BOXSIZE / 2.0))
        game_x = 4;
    else
        printf("[mapGL2gamecoord]: ERROR: encountered unknown board number %f\n", x);

    // CALCULATE Z
    if (z == (BOXSIZE / 2.0))
        game_y = 0;
    else if (z == (1.0 * BOXSIZE + BOXSIZE / 2.0))
        game_y = 1; 
    else if (z == (2.0 * BOXSIZE + BOXSIZE / 2.0))
        game_y = 2;
    else if (z == (3.0 * BOXSIZE + BOXSIZE / 2.0))
        game_y = 3;
    else if (z == (4.0 * BOXSIZE + BOXSIZE / 2.0))
        game_y = 4;
    else
        printf("[mapGamecoord2gl]: ERROR: encountered unknown board number %f\n", z);
}
