// globals.h : Global definitions and macros
//
// John Davin, Terrence Wong, Jim Lundberg
// Copyright (c) 2002-2004 JRL Innovations  All Rights Reserved
// 3D5 - The FIVE level 3D tic-tac-toe game
//

#ifndef GLOBALS_H
#define GLOBALS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// The dimensions of the GL drawing area. Changing these may distort the board 
// and would invalidate the current mouse mapping (a new one would need to be created).
#define GLAREA_WIDTH  600
#define GLAREA_HEIGHT 1100

//#define printf TRACE  // Because printf's don't show up in windows debugging

// Position tracking masks
#define PLAYER1_MASK        0x001F
#define PLAYER2_MASK        0x03E0
#define PLAYER3_MASK        0x7C00
#define FREE_SPACE_MASK     0x8000

// Name of the file that holds the mouse coordinate mapping
// used to map clicks to board coordinates (used in CoordMap.cpp). 
#define MOUSE_MAP_FILE "coords5.map"

// The number of possible win rows on the board 
// (ie, this is the length of our board array)
#define NUM_COMBOS          109
#define NUM_HOLES_PER_GAME  125
#define NUM_HOLES_PER_LEVEL 25
#define NUM_HOLES_PER_LINE  5
#define NUM_BOARDS          5

// Depth values for difficulty levels
typedef enum
{
    RANDOM_DIFF   = 1,
    EASY_DIFF     = 2,
    MEDIUM_DIFF   = 3,
    HARD_DIFF     = 4,
    VERYHARD_DIFF = 5
}
DIFFICULTY_LEVELS;

// Number of free Spaces
typedef enum
{
    FREE_SPACES_1,
    FREE_SPACES_2,
    FREE_SPACES_3,
    FREE_SPACES_4,
    FREE_SPACES_5,
    FREE_SPACES_6,
    FREE_SPACES_7,
    FREE_SPACES_8,
    FREE_SPACES_9,
    FREE_SPACES_10,
    FREE_SPACES_11,
    FREE_SPACES_12,
    FREE_SPACES_13,
    MIDDLE_FREE_SPACES_3,
    MIDDLE_FREE_SPACES_5,
    MIDDLE_FREE_SPACES_13
}
FREE_SPACES;

#define ALPHABETA_DEPTH     4 // 2              // Default alpha beta search depth
#define RANDOM_MOVES        1 // 1              // Specifies the # of initial moves the computer will move randomly 
#define DEFAULT_LEVEL       0 // 2              // Default to random placement

// The default starting players
#define DEFAULT_PLAYER1     COMPUTER
#define DEFAULT_PLAYER2     COMPUTER
#define DEFAULT_PLAYER3     FREE_SPACES_3

// Scalefactor is the quantity used in the gldisplay function to scale board size
#define SCALEFACTOR 2.3

// #define FLAT_SHADING                     // uncomment to use flat shading model

// Define DEBUG_COLORMAP if you want to see the mouse color map which is 
// used to select a square from click coordinates. The color map will be displayed
// after you click in the viewing area and then refresh the screen (by alt-tabbing out or some such)
// #define DEBUG_COLORMAP

// the circle stepsize sets the angular distance (in radians) between each point
// that is calculated for the circles at the center of each square. 
// So if the circles don't appear smooth, decreasing this value might help.
#define CIRCLE_STEPSIZE 0.5

// the radius of the four corner support columns:
#define COLUMN_RADIUS 0.1

// texture files and id's: 
// IMPORTANT NOTE: if you add any new texture files, they must be RGB jpgs with 
// image dimensions *in a power of 2*. So a 512x512 or 128x128 image is okay. 
// define the file to be "" if you don't want to use a texture. 

//#define BALL_TEXTURE_FILE "./Arghona.jpg"
//#define BALL_TEXTURE_FILE "./Beanco.jpg"
//#define BALL_TEXTURE_FILE "./Blkrose.jpg"
//#define BALL_TEXTURE_FILE "./Breccio.jpg"
//#define BALL_TEXTURE_FILE "./Bumpplat.jpg"
//#define BALL_TEXTURE_FILE "./Charred.jpg"
//#define BALL_TEXTURE_FILE ""
#define BALL_TEXTURE_FILE "./Texture.jpg"

//#define COL_TEXTURE_FILE "./Bumpplat.jpg"
//#define COL_TEXTURE_FILE "./Beebees.jpg"
#define COL_TEXTURE_FILE ""

#define BALL_TEXTURE_ID 1
#define COL_TEXTURE_ID  2

// Zoom distances
#define ZOOM_FACTOR     3
#define MIN_ZOOM       -13.0
#define MAX_ZOOM        40.0
// Note: MIN_ZOOM can't be set lower unless the GL bounding box area is increased 
//       (otherwise the board gets clipped at the back)

#endif
