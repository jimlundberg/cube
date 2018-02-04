// GLbox.h : This is a simple QGLWidget displaying an openGL wireframe box
//
// John Davin, Terrence Wong, Jim Lundberg
// Copyright (c) 2002-2004 JRL Innovations  All Rights Reserved
// 3D5 - The FIVE level 3D tic-tac-toe game
//

#ifndef GLBOX_H
#define GLBOX_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <QList>
#include <QListView>
#include "globals.h"

using namespace std;

struct Line;

#define PI 3.141592653589793238

// for square map picking: 
#define RENDER	1
#define SELECT	2

// The width of a single square on the board
#define BOXSIZE  1.0
#define BOXSIZE4 4.0        // BOXSIZE * 4
#define BOXSIZE5 5.0        // BOXSIZE * 5

// Size of the marbles displayed
#define OLD_MOVE_SIZE 0.35
#define NEW_MOVE_SIZE 0.5

#define INIT_LRANGLE  45    // Initial left-right angle
#define INIT_UDANGLE -64.5  // Initial up-down angle

// Height of board 0 (the top board)
#define BOARD0_H      10.0
#define BOARD1_H      7.5
#define BOARD2_H      5.0
#define BOARD3_H      2.5
#define BOARD4_H      0.0

// the radius of the circular hole for each square on the board
#define BOARD_HOLE_RADIUS 0.3

// Define rgb colors for player pieces
#define P1_r 1.0 // RedDIFFICULTY_LEVELS
#define P1_g 0.0
#define P1_b 0.0

#define P2_r 0.0
#define P2_g 1.0 // Green
#define P2_b 0.0

// Player #3 
#define P3_r 1.0 //
#define P3_g 1.0 // Yellow?
#define P3_b 0.0 //

// Free Spaces
#define PF_r 1.0 //
#define PF_g 1.0 // White
#define PF_b 1.0 //

// Display list numBox bers:DIFFICULTY_LEVELS
#define BOARD_DLIST 10
#define BOARD_DLIST_COLOR 11

// This determines the distance the ball is drawn above its center axis on each board level
// (ie, it raises the ball up by the specified amount)
#define BALL_ELEVATION_OFFSET 0.15

typedef enum
{
    HUMAN,
    COMPUTER,
    FREE_SPACE_1,
    FREE_SPACE_2,
    FREE_SPACE_3,
    FREE_SPACE_4,
    FREE_SPACE_5,
    FREE_SPACE_6,
    FREE_SPACE_7,
    FREE_SPACE_8,
    FREE_SPACE_9,
    FREE_SPACE_10,
    FREE_SPACE_11,
    FREE_SPACE_12,
    FREE_SPACE_13
}
PLAYERS;

//
// Stores the x,y,z coordinates for a point
//
struct Point
{
    double x, y, z;

    Point() {x = 0.0; y = 0.0; z = 0.0;}

    Point(double x_i, double y_i, double z_i)
    {
        x = x_i;
        y = y_i;
        z = z_i;
    }
};

// 
// Stores the parameters to draw a line
//
struct Line
{
    Point* orig;
    double  xangle, yangle, zangle;
    double  length;

    Line(double x1, double y1, double z1, double xa, double ya, double za, double len)
    {
        orig = new Point( x1, y1, z1 );
        xangle = xa;
        yangle = ya;
        zangle = za;
        length = len;
    }

    Line(Point* p, double xa, double ya, double za, double len)
    {
        orig = new Point(p->x, p->y, p->z);
        xangle = xa;
        yangle = ya;
        zangle = za;
        length = len;
    }
  
    ~Line()
    {
        delete orig;
        printf("[~Line]: deleted orig\n");
    }
};

//
// A wrapper struct to hold the parameters for the comp move thread
// 
class GLBox;
struct params
{
	int * curplayer;
	GLBox * glbox;
};

// GLBox
///////////////////////////////////////////////////////////////////////////////

class GLBox
{
public:
    GLBox(void *dlg, QListView *listView);
	~GLBox();

    void initializeGL();
    void paintGL(int gamecoord[]);
    void resizeGL(int w, int h);
	void updateGL();
 
    void changeText(const string);

    void setDepth(DIFFICULTY_LEVELS d); // Called to set difficulty level
    void SetPlayer1(int player);
    void SetPlayer2(int player);
    void SetPlayer3(int player);enum

    void SetPlayerColor(int colorIndex);

    void startNewGame();
    void setSound(bool state);
    void ShowWin(int moves);
    void BlockWin(int gamecoord[]);

	// Input functions: 
    void handleLeftClick(int x, int y);
    void handleRightButtonUp(int x, int y);
    void handleRightButtonDown(int x, int y);
    void handleMiddleButtonUp(int x, int y);
    void handleMiddleButtonDown(int x, int y);
    void handleMouseMove(int x, int y);

	// Thread function:
//  UINT CompMoveProc(LPVOID myparams);
	void makeComputerMove();

    bool GettingCompMove;   // Set to true when we are getting the computer's move
    int  CurPlayer;         // 0 if it is Player1's turn and 1 if it is Player2

    void DrawLevel(double y, bool colorpickmode);
    void LoadTextures();
    void DrawBox(double x0, double x1, double y0, double y1,
                 double z0, double z1, enum type);

    void SaveMove(int x, int y, int z);
    void DrawBoard(bool colorpickmode);

    void DrawMarble(double x, double y, double z, float r, float g, float b, double size);
    void DrawMarbles();
    void DrawSupportColumns();

    void DrawAxes(double len);
    void DrawWinningLine();
    void AddWinLine(int row, int winner);
    void UndoMove();
    void GetPrevMove(int& x, int& y, int& z);
    void RotateRight();
    void RotateLeft();
    void RotateUp();
    void RotateDown();

    void PlayGameSound(char *snd);

    void startPicking();
    void stopPicking(int gamecoord[]);

    void DrawNormalSquare(int i, int j, float y);
    void DrawSquare(int i, int j, float y);

    static void     mapGamecoord2gl(int game_x, int game_y, int game_z, double &x, double &y, double &z);
    static void     mapsqr2gamecoord(int sqrnum, int &x, int &y, int &z);
    static void     mapsqr2coord(int sqrnum, double &x, double &y, double &z);
    static Point*   mapGame2gl(Point* game); // returns the point in GL coordinates
    static void     mapGL2gamecoord(double x, double y, double z, int &game_x, int &game_y, int &game_z);

	void zoomIn();
	void zoomOut();

    GLubyte getLevelColor(double y) const;  // A helper function

    void *tttdlg;  // Tic-Tac-Toe Dialog handle

protected:
	float		ZoomFactor;                 // Zoom In/Out Factor
    int         cursorX;                    // Mouse X Pixel
    int         cursorY;                    // Mouse Y Pixel

    bool        turn_on_axes;               // Axis display flag

	bool        Buildmap;                   // Build a mouse map flag

	int         player1;                    // Player indexed type selection
	int         player2;
	int         player3;

    int         m_PlayerColor;              // Player index color selection

    bool        RightButtonDown;            // Set to true when the right mouse button is down.
    bool        MiddleButtonDown;           // Set to true when the middle mouse button is down.

    float       midbutton_y;                // the y coordinate of the last midbutton position (used to modulate zoom)

//  Engine      *game;                      // The game engine
    CListBox    *m_ListBox;					// List Box window handle
    Line        *winline;                   // The line we draw across pieces on win
};


class Points
{
protected:
    double x;
    double y;   // Coordinates for the point
    double z;

    float  r;
    float  g;   // RGB values for the color of the dot
    float  b; 

    Points *next;
//  Points *prev;

public:
    Points();
    virtual ~Points();
    Points(double ix, double iy, double iz);
    Points(double ix, double iy, double iz, float ir, float ig, float ib);

    double getx() {return x;}
    double gety() {return y;}
    double getz() {return z;}

    float  getr() {return r;}
    float  getg() {return g;}
    float  getb() {return b;}

    Points *add(double ix, double iy, double iz);
    Points *add(double ix, double iy, double iz, float ir, float ig, float ib);

    //BOOL del( Points * pPt );
    Points *deleteHead();
    void deleteall();

    bool contains(double xi, double yi, double zi, float r_i, float g_i, float b_i);
    int count();

//  Points *GetPrev();
    Points *GetNext();
};

#endif // GLBOX_H
