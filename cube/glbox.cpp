// GLbox.cpp - Implementation file for 3D tic-tac-toe OpenGL board. 
//
// John Davin, Terrence Wong, Jim Lundberg
// Copyright (c) 2002-2004 JRL Innovations  All Rights Reserved
// 3D5 - The FIVE level 3D tic-tac-toe game
//

#include <assert.h>
#include <math.h>
#include "glbox.h"
#include "engine.h"
#include "jpeglib.h"

using namespace std;

#ifdef DEBUG_COLORMAP
int Picking_mode = SELECT;
#else
int Picking_mode = RENDER;
#endif

extern HINSTANCE    g_hInst;
extern HWND         m_hWnd;

static int  lr_angle = (int)INIT_LRANGLE; // Left-right rotation angle
static int  ud_angle = (int)INIT_UDANGLE; // Up-down rotation angle
static int  prevx = 0, prevy = 0;

GLfloat     Light_Position[] = {0.0, 10.0, 5.0, 0.0};
bool        SOUND = true; // stores the sound status

Engine *    game;       // The game engine

//
// Create a GLBox widget
//
GLBox::GLBox(void *dlg, CListBox *aListBox)
{
    m_ListBox = aListBox;

    tttdlg = dlg;
    Buildmap = false;
    RightButtonDown = MiddleButtonDown = false;
    
    //  plist = NULL;
    winline = NULL;
    GettingCompMove = false;
    ZoomFactor = 0.0;
    
    // Set default player values
    CurPlayer = PLAYER1;
    player1 = DEFAULT_PLAYER1;
    player2 = DEFAULT_PLAYER2;
    player3 = DEFAULT_PLAYER3;
    
    turn_on_axes = false;
    
    // Start up the game engine:
    game = new Engine(aListBox);
}

//
// Release allocated resources
// 
GLBox::~GLBox()
{
    printf("[~GLBOX]: deconstructor called\n");
    
//  if (plist != NULL)
//  {
//	    plist->deleteall();
//  }
    
    if (winline != NULL)
    {
        delete winline;    
    }
    
    delete game;
    printf("[~GLBox]: done\n");
}

void GLBox::initializeGL()
{
    // Set up the rendering context, define display lists etc.:
#ifdef FLAT_SHADING
    glShadeModel(GL_FLAT);
#endif
    
    glEnable(GL_LINE_SMOOTH); // antialiasing on lines
    
    LoadTextures();
    
    // Draw the board into a display list: 
    glNewList(BOARD_DLIST, GL_COMPILE);
    DrawBoard(false);
    glEndList();
    
    // also make a color mapped version for our mouse picking mode
    glNewList(BOARD_DLIST_COLOR, GL_COMPILE);
    DrawBoard(true); // true indicates draw in color picking mode
    glEndList();
    
    // Set up lighting:
//  GLfloat specular[]  = {1.0, 1.0, 1.0, 1.0};
//  GLfloat shininess[] = {50.0};

//  GLfloat mat_ambient[] = {1.0, 1.0, 1.0, 1.0};
//  glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    
    GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
//  GLfloat lm_ambient[]   = {0.2, 0.2, 0.2, 1.0};
    
//  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lm_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 100.0);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_specular);
    
    GLfloat light_ambient[]  = {1.0, 1.0, 1.0, 1.0};
    GLfloat light_diffuse[]  = {1.0, 1.0, 1.0, 1.0};
    GLfloat light_specular[] = {1.0, 1.0, 1.0, 1.0};
    
//  glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
//  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, Light_Position);
    
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
    
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClearAccum(0.0, 0.0, 0.0, 0.0);
}

//
// Setup viewport, projection etc.:
//
void GLBox::resizeGL(int w, int h)
{
    // Prepare for and then
    glMatrixMode(GL_PROJECTION);
    
    // Define the projection
    glLoadIdentity();
    
    // Transformation
//  glFrustum(-1.0, 1.0, -1.0, 1.0, 1.5, 40.0);
    
    // Transformation
    // Use a 1:2 aspect ratio (so the height is twice the width)
    gluPerspective(90.0, 1./2., 0.05, 60.0);
    
    // Back to modelview matrix
    glMatrixMode(GL_MODELVIEW);
    
    // Define the viewport
//  glViewport(0, 0, w, h);
}

//
// Draw the scene:
//
void GLBox::paintGL(int gamecoord[])
{
    // Clear the matrix
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glLoadIdentity();
    
    if (Picking_mode == SELECT)
    {
        startPicking();
    }
    
    // Translate on the z axis to do zooming
    glTranslatef(0., 0., ZoomFactor);
    
    // Viewing transformation
    glTranslatef((float)0.0, (float)-6.6, (float)-22.0);
    gluLookAt(4.5, 9.5, 4.5, 0., 0., 0., -1., 1., -1.);
    
    // Set the scale factor
    glScalef((float)SCALEFACTOR, (float)SCALEFACTOR, (float)SCALEFACTOR);
    
    // Reset the light position
    glLightfv(GL_LIGHT0, GL_POSITION, Light_Position);
    
    // Rotate up or down
    glRotated(ud_angle, 1.,0., -1.);
    
    // Rotate left or right
    glRotated(lr_angle, 0, 1, 0);
    
    glTranslatef(-BOXSIZE5/2., -BOARD2_H, -BOXSIZE5/2.);
    //  printf( "%d  %d  %f  %f  %f  %f\n", lr_angle, ud_angle, c, s, sin( 45.0 / 180.0 * 3.14), sin( 55.0/180.0 * 3.14 ));
    
    // So that we can use normal glColor3f() on non-lighting objects
    glDisable(GL_LIGHTING);
    
    glColor3f(1.0, 0.0, 0.0);
    
    // Draw outlines of the x,y,z axes with a length of 20 for the lines
    if (turn_on_axes)
        DrawAxes(15.);
    
    if (Picking_mode == SELECT)
    {
        glCallList(BOARD_DLIST_COLOR); // draw the board
        if (gamecoord != NULL)
        {
            stopPicking(gamecoord);
        }
    }
    else
    {
        glCallList(BOARD_DLIST);
        glEnable(GL_LIGHTING);
        
        DrawSupportColumns(); // Draws the four vertical columns
        
        // Draw dots
        DrawMarbles();
        
        // Draw a win line if the game is over
        DrawWinningLine(); 
    }
    
    glFlush();
}

void GLBox::startPicking()
{
    glDisable(GL_DITHER);
}

void GLBox::stopPicking(int gamecoord[])
{
    GLint viewport[4];
    GLubyte pixel[3];
    
    // Figure out which color square we picked
    glGetIntegerv(GL_VIEWPORT, viewport);
    printf("[stopPicking]: viewport = %d, %d, %d, %d\n",
           viewport[0], viewport[1], viewport[2], viewport[3]);
    
    cursorX -= 12; // To adjust for the GUI margin to left of gl display
    viewport[3] = 654;
    
    glReadPixels(cursorX, viewport[3] - cursorY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, (void*)pixel);
    
    printf("[stopPicking]: glReadPixels(%d, %d, ...)\n", cursorX, viewport[3] - cursorY);
    printf("[stopPicking]: pixel = %d, %d, %d\n", pixel[0], pixel[1], pixel[2]);
    
    // For the first RGB value (the Red component, which specifies the board height), 
    // just pick the board level that is closest. 
    GLubyte closestdiff = 255;
    GLubyte diff;
    
    diff = abs(pixel[0] - getLevelColor(BOARD0_H));
    if (diff <= closestdiff)
    {
        gamecoord[2] = 0;
        closestdiff  = diff;
    }
    
    diff = abs(pixel[0] - getLevelColor(BOARD1_H));
    if (diff < closestdiff)
    {
        gamecoord[2] = 1;
        closestdiff  = diff;
    }
    
    diff = abs(pixel[0] - getLevelColor(BOARD2_H));
    if (diff < closestdiff)
    {
        gamecoord[2] = 2;
        closestdiff  = diff;
    }
    
    diff = abs( pixel[0] - getLevelColor(BOARD3_H));
    if (diff < closestdiff)
    {
        gamecoord[2] = 3;
        closestdiff  = diff;
    }

    diff = abs( pixel[0] - getLevelColor(BOARD4_H));
    if (diff < closestdiff)
    {
        gamecoord[2] = 4;
        closestdiff  = diff;
    }
    
    // If either of the other two pixels are 0 (black) then that means the click was 
    // out of bounds
    if (pixel[1] == 0 || pixel[2] == 0)
    {
        gamecoord[0] = gamecoord[1] = gamecoord[2] = -1;	
    }
    else
    {
        gamecoord[1] = (pixel[2] / 40) - 1;
        gamecoord[0] = (pixel[1] / 40) - 1;
    }
    
    glEnable( GL_DITHER );
    
#ifndef DEBUG_COLORMAP  // If we're not debugging, then go back to normal render mode
    Picking_mode = RENDER;
#endif
}

//
// This is a simple conversion / mapping function. 
// Input is a board height (must be one of the BOARD*_H)
// and output is an unsigned char which will be used as one component of 
// an RGB value for the mouse mapping
//
GLubyte GLBox::getLevelColor(double y) const
{
    switch ((int)y)
    {
    case (int)BOARD0_H:
        return 0;
        
    case (int)BOARD1_H:
        return 256 * 1/4;
        
    case (int)BOARD2_H:
        return 256 * 2/4;
        
    case (int)BOARD3_H:
        return 256 * 3/4;
        
    case (int)BOARD4_H:
        return 255;
        
    default:
        ASSERT(0);
        return 0;        
    }
}

//
// Load the textures in the init function
//
void GLBox::LoadTextures()
{
    if (BALL_TEXTURE_FILE != "")
    {
        tImageJPG *balltexture = LoadJPG(BALL_TEXTURE_FILE);
        if (balltexture == NULL)
        {
            return;
        }
        
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, BALL_TEXTURE_ID);
        
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        
        glTexImage2D(GL_TEXTURE_2D, 0, 3, balltexture->sizeX, balltexture->sizeY,
                     0, GL_RGB, GL_UNSIGNED_BYTE, balltexture->data);
        printf("loaded texture of size %d %d\n", balltexture->sizeX, balltexture->sizeY);
        
        delete balltexture->data;
        delete balltexture;
        glDisable(GL_TEXTURE_2D);
    }
    
    if (COL_TEXTURE_FILE != "")
    {
        tImageJPG *coltexture = LoadJPG(COL_TEXTURE_FILE);
        if (coltexture == NULL)
        {
            return;
        }
        
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, COL_TEXTURE_ID);
        
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1 );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        
        glTexImage2D(GL_TEXTURE_2D, 0, 3, coltexture->sizeX, coltexture->sizeY,
                     0, GL_RGB, GL_UNSIGNED_BYTE, coltexture->data);
        printf("loaded texture of size %d %d\n", coltexture->sizeX, coltexture->sizeY);

        delete coltexture->data;
        delete coltexture;
        glDisable(GL_TEXTURE_2D);
    }
}

//
// Sets the search depth to d - called to set difficulty level
// 
void GLBox::setDepth(DIFFICULTY_LEVELS d)
{ 
    game->setDepth(d);
}

//
// Slot that is called to set the game player1
// 
void GLBox::SetPlayer1(int player)
{
    ASSERT (player < 2);
    player1 = player;
}

void GLBox::SetPlayer2(int player)
{
    ASSERT (player < 2);
    player2 = player;
}

void GLBox::SetPlayer3(int player)
{
    ASSERT (player < 16);
    player3 = player;
}

void GLBox::SetPlayerColor(int colorIndex)
{
    ASSERT (colorIndex < 2);
    m_PlayerColor = colorIndex;
}

void GLBox::UndoMove()
{
    //int x, y, z;
    //game->GetPrevMove(x, y, z);
	
	// don't allow undoing of free space markers
    //  check if the move index is lower than the player3 number (# of free squares)
	//  if it is, don't do UndoMove because it would undo a free space
    if (game->GetMoveIndex() <= player3+1)
    {   // player3=0 indicates 1 free space
		return;
	}
    else if(GettingCompMove)
    {
		// need to wait until after computer thread finishes, because we can't 
		//  kill the child thread
		return;
	}

    bool success = game->UndoMove();
    if (success)
    {
		// delete the first (most recent) point:
		game->plist = game->plist->deleteHead();
		// switch the current player:
		switch (CurPlayer)
		{
            case PLAYER1: CurPlayer = PLAYER2; break;
            case PLAYER2: CurPlayer = PLAYER1; break;
		}
		updateGL();
	}
}

//
// This slot will set the SOUND variable to the state boolean
// (true is sound on, false is off)
//
void GLBox::setSound(bool state)
{
    SOUND = state;
}

void GLBox::startNewGame()
{
    double  op_x, op_y, op_z = 0.0;
    int x,  y,  z;
    int m1, m2, m3;

    CurPlayer = PLAYER1;
    
    game->NewGame();
    
    // Clear the piece list: 
    if (game->plist != NULL)
    {
        game->plist->deleteall();
        game->plist = NULL;
    }
    
    if (winline != NULL)
    {
        // delete the won line
        delete winline;
        winline = NULL;
    }


	char str[80] = {NULL};
    sprintf(str, "                         Player 1            Player 2");
	m_ListBox->ResetContent();
    m_ListBox->AddString(str);

    switch (player3)
    {
    case FREE_SPACES_1:
    case FREE_SPACES_2:
    case FREE_SPACES_3:
    case FREE_SPACES_4:
    case FREE_SPACES_5:
    case FREE_SPACES_6:
    case FREE_SPACES_7:
    case FREE_SPACES_8:
    case FREE_SPACES_9:
    case FREE_SPACES_10:
    case FREE_SPACES_11:
    case FREE_SPACES_12:
    case FREE_SPACES_13:
        {
			// Always add the free space as the middle of the 3rd layer
            mapGamecoord2gl(2, 2, 2, op_x, op_y, op_z);
            game->plist = new Points(op_x, op_y, op_z, P3_r, P3_g, P3_b);
            game->makeMove(PLAYER1, 2, 2, 2, true);
            game->makeMove(PLAYER2, 2, 2, 2, true);
            game->makeMove(PLAYER3, 2, 2, 2, true);
            game->SaveMove(2, 2, 2);

            for (int i=0; i < player3; i++)
            {
                m1 = m2 = m3 = 0;
                do
                {
                    x = rand() % NUM_HOLES_PER_LINE;
                    y = rand() % NUM_HOLES_PER_LINE;
                    z = rand() % NUM_HOLES_PER_LINE;

                    if ((x == 2) && (y == 2) && (z == 2))
                    {
                        continue;
                    }

                    // Add the free space to everyone's list
                    mapGamecoord2gl(x, y, z, op_x, op_y, op_z);
                    game->plist = game->plist->add(op_x, op_y, op_z, P3_r, P3_g, P3_b);

                    m1 = game->makeMove(PLAYER1, x, y, z, true);
                    m2 = game->makeMove(PLAYER2, x, y, z, true);
                    m3 = game->makeMove(PLAYER3, x, y, z, true);
					game->SaveMove(x, y, z);
                }
                while (!m1 && !m2 && !m3);
            }

        }
        break;
        
    case MIDDLE_FREE_SPACES_3:
        {
            for (int z = 1; z < 4; z++)
            {
                if (z == 2) continue;

                // Always add the free space as the middle of the 3rd layer
                mapGamecoord2gl(2, 2, z, op_x, op_y, op_z);
                game->plist = game->plist->add(op_x, op_y, op_z, P3_r, P3_g, P3_b);
                m1 = game->makeMove(PLAYER1, 2, 2, z, true);
                m2 = game->makeMove(PLAYER2, 2, 2, z, true);
                m3 = game->makeMove(PLAYER3, 2, 2, z, true);
				game->SaveMove(2, 2, z);
            }

            // Always add the free space as the middle of the 3rd layer
            mapGamecoord2gl(2, 2, 2, op_x, op_y, op_z);
            game->plist = game->plist->add(op_x, op_y, op_z, P3_r, P3_g, P3_b);
            game->makeMove(PLAYER1, 2, 2, 2, true);
            game->makeMove(PLAYER2, 2, 2, 2, true);
            game->makeMove(PLAYER3, 2, 2, 2, true);
            game->SaveMove(2, 2, 2);
        }
        break;

    case MIDDLE_FREE_SPACES_5:
        {
            for (int z = 0; z < 5; z++)
            {
                if (z == 2) continue;

                // Always add the free space as the middle of the 3rd layer
                mapGamecoord2gl(2, 2, z, op_x, op_y, op_z);
                game->plist = game->plist->add(op_x, op_y, op_z, P3_r, P3_g, P3_b);
                m1 = game->makeMove(PLAYER1, 2, 2, z, true);
                m2 = game->makeMove(PLAYER2, 2, 2, z, true);
                m3 = game->makeMove(PLAYER3, 2, 2, z, true);
                game->SaveMove(2, 2, z);
            }

            // Always add the free space as the middle of the 3rd layer
            mapGamecoord2gl(2, 2, 2, op_x, op_y, op_z);
            game->plist = game->plist->add(op_x, op_y, op_z, P3_r, P3_g, P3_b);
            game->makeMove(PLAYER1, 2, 2, 2, true);
            game->makeMove(PLAYER2, 2, 2, 2, true);
            game->makeMove(PLAYER3, 2, 2, 2, true);
            game->SaveMove(2, 2, 2);
        }
        break;

    case MIDDLE_FREE_SPACES_13:
        {
            for (int x = 0; x < 5; x++)
            {
                if (x == 2) continue;

                // Always add the free space as the middle of the 3rd layer
                mapGamecoord2gl(x, 2, 2, op_x, op_y, op_z);
                game->plist = game->plist->add(op_x, op_y, op_z, P3_r, P3_g, P3_b);
                m1 = game->makeMove(PLAYER1, x, 2, 2, true);
                m2 = game->makeMove(PLAYER2, x, 2, 2, true);
                m3 = game->makeMove(PLAYER3, x, 2, 2, true);
				game->SaveMove(x,2,2);
            }

            for (int y = 0; y < 5; y++)
            {
                if (y == 2) continue;

                // Always add the free space as the middle of the 3rd layer
                mapGamecoord2gl(2, y, 2, op_x, op_y, op_z);
                game->plist = game->plist->add(op_x, op_y, op_z, P3_r, P3_g, P3_b);
                game->makeMove(PLAYER1, 2, y, 2, true);
                game->makeMove(PLAYER2, 2, y, 2, true);
                game->makeMove(PLAYER3, 2, y, 2, true);
                game->SaveMove(2, y, 2);
            }

            for (int z = 0; z < 5; z++)
            {
                if (z == 2) continue;

                // Always add the free space as the middle of the 3rd layer
                mapGamecoord2gl(2, 2, z, op_x, op_y, op_z);
                game->plist = game->plist->add(op_x, op_y, op_z, P3_r, P3_g, P3_b);
                game->makeMove(PLAYER1, 2, 2, z, true);
                game->makeMove(PLAYER2, 2, 2, z, true);
                game->makeMove(PLAYER3, 2, 2, z, true);
				game->SaveMove(2,2,z);
            }

            // Always add the free space as the middle of the 3rd layer
            mapGamecoord2gl( 2, 2, 2, op_x, op_y, op_z );
            game->plist = game->plist->add(  op_x, op_y, op_z, P3_r, P3_g, P3_b);
            game->makeMove(PLAYER1, 2, 2, 2, true);
            game->makeMove(PLAYER2, 2, 2, 2, true);
            game->makeMove(PLAYER3, 2, 2, 2, true);
            game->SaveMove(2, 2, 2);
        }
        break;

	default:
		assert(!"Invalid player3 number");
		break;
    }

    GLBox::updateGL();

	TRACE("New game with player1=%d, player2=%d\n", player1, player2);

    if ((player1 == COMPUTER && player2 == COMPUTER))
    {
        int     game_x2, game_y2, game_z2;
    
		GettingCompMove = false;
        while (true) // break loop when game won.
        {
            Sleep(10);
            
            game->getMove(CurPlayer, game_x2, game_y2, game_z2);
            TRACE("[startNewGame]: getMove(%d) returned %d %d %d\n",
                    CurPlayer, game_x2, game_y2, game_z2);

            // Translate the game coords to GL coords: 
            mapGamecoord2gl(game_x2, game_y2, game_z2, op_x, op_y, op_z);
            
            if (game->plist == NULL)
            {
                game->plist = new Points(op_x, op_y, op_z, P1_r, P1_g, P1_b);
            }
            else
            {
                if (CurPlayer == PLAYER1)
                {
                    game->plist = game->plist->add(op_x, op_y, op_z, P1_r, P1_g, P1_b);
                }
                else if (CurPlayer == PLAYER2) 
                {
                    game->plist = game->plist->add(op_x, op_y, op_z, P2_r, P2_g, P2_b);
                }
            }
            
            GLBox::updateGL();
            
            if (int m = game->hasWon())
            {
                ShowWin( m );
                return;
            }
            
            // Flip to the other player
            switch (CurPlayer)
            {
            case PLAYER1: CurPlayer = PLAYER2; break;
            case PLAYER2: CurPlayer = PLAYER1; break;
            }
        }
    }
    else if (player1 == COMPUTER)
    {
        int     game_x1, game_y1, game_z1;
        double  op_x, op_y, op_z = 0.0;
        
        game->getMove(CurPlayer, game_x1, game_y1, game_z1);
        TRACE("[startNewGame]: getMove returned\n");
        
        // Translate the game coords to GL coords: 
        mapGamecoord2gl(game_x1, game_y1, game_z1, op_x, op_y, op_z);
        
		if (game->plist == NULL)
		{
            game->plist = new Points(op_x, op_y, op_z, P1_r, P1_g, P1_b);
		}
		else
		{
            game->plist = game->plist->add(op_x, op_y, op_z, P1_r, P1_g, P1_b);
		}
        
        GLBox::updateGL();
        
        // Flip to the next player
        switch (CurPlayer)
        {
        case PLAYER1:    CurPlayer = PLAYER2; break;
        case PLAYER2:    CurPlayer = PLAYER1; break;
        }
    }
}

//
// Update the GL display
//
void GLBox::updateGL()
{
    //  TRACE0("updateGL\r\n");
    
    // invalidate the entire window region so it will redraw
    // InvalidateRect(NULL, NULL, false);
    ((C3D5Dlg*)tttdlg)->Redraw();
    
//  UpdateWindow( m_hWnd );
//  RedrawWindow( NULL, NULL, NULL, RDW_INTERNALPAINT | RDW_UPDATENOW );
}

//
// A thread function for calculating the computer's move using the 
// alphabeta function. Since this could take a long time on high depths,
// if we didn't do it in a thread, the application would freeze while we processed. 
// The thread is started below in handleLeftClick()
//
UINT CompMoveProc(LPVOID myparams)
{
    int game_x, game_y, game_z;
    
    double myx = 0.0; // the coords to put the dot at
    double myy = 0.0;
    double myz = 0.0;
    
    int *CurPlayer = ((struct params *)myparams)->curplayer;
    GLBox   *glbox = ((struct params *)myparams)->glbox;
    
    TRACE("[CompMoveProc]: thread begun.\n");
    
    game->getMove(*CurPlayer, game_x, game_y, game_z);
    glbox->mapGamecoord2gl(game_x, game_y, game_z, myx, myy, myz);
    
    if (game->plist == NULL)
    {
        game->plist = new Points(myx, myy, myz, P1_r, P1_g, P1_b);
    }
    else
    {
        if (*CurPlayer == PLAYER1)
        {
            game->plist = game->plist->add(myx, myy, myz, P1_r, P1_g, P1_b);
        }
        else if (*CurPlayer == PLAYER2)
        {
            game->plist = game->plist->add(myx, myy, myz, P2_r, P2_g, P2_b);
        }
        else // then CurPlayer == PLAYER3
        {
            game->plist = game->plist->add(myx, myy, myz, P3_r, P3_g, P3_b);
        }
    }
    
    if (int m = game->hasWon())
    {
        glbox->ShowWin(m);
    }
    
    // Send a message to the dialog to repaint (updateGL)
    // we can't just call updateGL in here because it messes up... probably 
    // the thread does not have proper access to the parent's GL device context
    SendMessage( HWND_TOPMOST, WM_PAINT, NULL, NULL );
    
    switch (glbox->CurPlayer)
    {
    case PLAYER1: glbox->CurPlayer = PLAYER2; break;
    case PLAYER2: glbox->CurPlayer = PLAYER1; break;
    }
    printf("[CompMoveProc]: done\n");
    
    // Done getting comp's move, allow user to make another move
    glbox->GettingCompMove = false; 
//	((C3D5Dlg*)glbox->tttdlg)->EndWaitCursor();

	TRACE("thread GettingCompMove=%d\n", glbox->GettingCompMove);
    free(myparams);
  
    Beep(2800, 100);
    Sleep(100);
    Beep(2800, 100);

    return 0;
}

//
// Called from the left click handler in 3D5Dlg.cpp
//
void GLBox::handleLeftClick(int x, int y)
{
    // If the computer is busy calculating its move in a thread,
    // don't allow user to go again yet. 
    if (GettingCompMove)
    {
        printf("[handleLeftClick]: ignoring, comp thread is active\n");
        return;
    }
    else if ( winline != NULL)
    {
		TRACE("Game is over\n");
		return;
	}
    
    TRACE("[GLBox::handleLeftClick]: received (%d, %d)\n", x, y);
    TRACE("[GLBox::handleLeftClick]: CurPlayer=%d, player1=%d, player2=%d\n",
			CurPlayer, player1, player2);

    int gamecoord[3]; // the game coordinates of the click
    
    cursorX = x;
    cursorY = y;
    
    // draw the board in Select mode (with colormap)
    Picking_mode = SELECT;
    paintGL(gamecoord);
    
    // Now place a piece at the x,y coordinates (if they're valid)	
    double myx = 0.0; // The coords to put the dot at
    double myy = 0.0;
    double myz = 0.0;
    
    // Only do something if the current player is human
    if ((CurPlayer == PLAYER1 && player1 == HUMAN) ||
        (CurPlayer == PLAYER2 && player2 == HUMAN))
    {
        int game_x, game_y, game_z;
        
        game_x = gamecoord[0];
        game_y = gamecoord[1];
        game_z = gamecoord[2];
        TRACE("Got game coords %d, %d, %d\n", game_x, game_y, game_z);
        
        if (!game->makeMove(CurPlayer, game_x, game_y, game_z, false))
        {
            return; // move failed (was illegal)
        }

		char str[80] = {0};
		switch (CurPlayer)
		{
		case PLAYER1:
            sprintf(str, "                         %C%d%c", 'V' + game_z, game_y + 1, 'a' + game_x);
			break;

		case PLAYER2:
            sprintf(str, "                                                  %C%d%c", 'V' + game_z, game_y + 1, 'a' + game_x);
			break;
		}

        m_ListBox->AddString(str);

        mapGamecoord2gl(game_x, game_y, game_z, myx, myy, myz);
        TRACE("sqr mapped to (%f, %f, %f)\n", myx, myy, myz);
        
        if (game->plist == NULL)
        {
            game->plist = new Points(myx, myy, myz);
        }
        else
        {
            if (CurPlayer == PLAYER1)
            {
                game->plist = game->plist->add(myx, myy, myz, P1_r, P1_g, P1_b);
            }
            else if (CurPlayer == PLAYER2)
            {
                game->plist = game->plist->add(myx, myy, myz, P2_r, P2_g, P2_b);
            }
            else if (CurPlayer == PLAYER3)
            {
                game->plist = game->plist->add(myx, myy, myz, P3_r, P3_g, P3_b);
            }
        }
        
        GLBox::updateGL();
        
        if (int m = game->hasWon())
        {
            ShowWin( m );
            return;
        }
        
        if (int m = game->hasFourOfFive())
        {
            BlockWin( gamecoord );
            return;
        }
        
        switch (CurPlayer)
        {
        case PLAYER1: CurPlayer = PLAYER2; break;
        case PLAYER2: CurPlayer = PLAYER1; break;
        }
        
        // Now the computer moves in response. 
        if ((CurPlayer == PLAYER1 && player1 == COMPUTER) ||
            (CurPlayer == PLAYER2 && player2 == COMPUTER))
        {
            // Now get computer's move: 
			makeComputerMove();
        }
    }
}

//
// makeComputerMove() spawn off as a thread           
//
void GLBox::makeComputerMove()
{
    // Put the params struct on the heap so we don't have a race condition
    struct params *myparams;
    myparams = (struct params *) malloc( sizeof(struct params) );
    myparams->curplayer = &CurPlayer;
    myparams->glbox = this;

    GettingCompMove = true;
	//((C3D5Dlg*)tttdlg)->BeginWaitCursor();
	TRACE("[make]: GettingCompMove=%d\n", GettingCompMove);

    // Note on concurrency: We don't need to mutex the GettingCompMove var because
    // there can never be more than one worker thread running at a time because 
    // we don't allow a new one to start until the var is flagged to false
    // (ie, we can only have the computer working on one move at a time)    
    AfxBeginThread(CompMoveProc, myparams);
    
    TRACE("[handleLeftClick]: After creation of thread\n");
}

//
// Called from 3D5Dlg.cpp when right mouse button is pressed
// 
void GLBox::handleRightButtonDown(int x, int y)
{
    RightButtonDown = true;
    printf("[handleRightButtonDown]: RightButton down at %d, %d\n", x, y);
    
    // Set the starting position for display rotation: 
    prevx = x;
    prevy = y;
}

//
// Called from the right click handler in 3D5Dlg.cpp
// 
void GLBox::handleRightButtonUp(int x, int y)
{
    RightButtonDown = false;
    printf("[handleRightButtonUp]: RightButton at %d, %d\n", x, y);
}

//
// Called from 3D5Dlg.cpp when middle mouse button is pressed
// 
void GLBox::handleMiddleButtonDown(int x, int y)
{
    MiddleButtonDown = true;
//  printf("[handleMiddleButtonDown]: MiddleButton down at %d, %d\n", x, y);

    // Set the starting position for zooming control:
    midbutton_y = (float)y;
}

//
// Called from the middle click handler in 3D5Dlg.cpp
// 
void GLBox::handleMiddleButtonUp(int x, int y)
{
    MiddleButtonDown = false;    
//  printf("[handleMiddleButtonUp]: MiddleButton at %d, %d\n", x, y);
}

//
// Called from the OnMouseMove handler in 3D5Dlg.cpp
// If the right mouse button is down, we need to rotate the display. 
//
void GLBox::handleMouseMove(int x, int y)
{
    if (RightButtonDown)
    {
        lr_angle = (lr_angle + x - prevx) % 360;
        ud_angle = (ud_angle + y - prevy) % 360;
        
        if ((y < prevy) && (ud_angle < -98))
        {
            ud_angle = -98;
        }
        
        if ((y > prevy) && (ud_angle > 21))
        {
            ud_angle = 21;
        }
        
        prevx = x;
        prevy = y;
        
        GLfloat light_position[] = {0.0, 10.0, 0.0, 0.0};
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        
        GLBox::updateGL();
    }
    
    if (MiddleButtonDown)
    {
        // Only use the y coordinate to control zoom because it's the intuitive axis for zooming
        ZoomFactor += (float)((float)(y - midbutton_y) * 0.1);
        
        // Keep the number bounded: 
        if (ZoomFactor > MAX_ZOOM)
        {
            ZoomFactor = MAX_ZOOM;
        }
        else if (ZoomFactor < MIN_ZOOM)
        {
            ZoomFactor = MIN_ZOOM;
        }
        
        midbutton_y = (float)y;
        
        printf("ZoomFactor set to %f\n", ZoomFactor);
        GLBox::updateGL();
    }
}

void GLBox::zoomIn()
{
    ZoomFactor += ZOOM_FACTOR;
    
    if (ZoomFactor > MAX_ZOOM)
    {
        ZoomFactor = MAX_ZOOM;
    }
    else if (ZoomFactor < MIN_ZOOM)
    {
        ZoomFactor = MIN_ZOOM;
    }
    
    GLBox::updateGL();
}

void GLBox::zoomOut()
{
    ZoomFactor -= ZOOM_FACTOR;
    
    if (ZoomFactor > MAX_ZOOM)
    {   
        ZoomFactor = MAX_ZOOM;
    }
    else if (ZoomFactor < MIN_ZOOM)
    {
        ZoomFactor = MIN_ZOOM;
    }

    GLBox::updateGL();
}

//
// Parameters are x,y,z as the coordinate of the center of the sphere
// and r,g,b as the color to draw the dot as. 
//
void GLBox::DrawMarble(double x, double y, double z, GLfloat r, GLfloat g, GLfloat b, double size)
{
    //  TRACE( "[DrawMarble]: at (%f, %f, %f) with color (%f, %f, %f)\n", x, y, z, r, g, b );
    
    // Set to polygon fill mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    glColor3f(r, g, b );
    
    glPushMatrix();
    
    glTranslated(x, y + BALL_ELEVATION_OFFSET, z);
    
    if (m_PlayerColor > 0)
    {
        // Turn on texturing
        glEnable(GL_TEXTURE_2D );
        glBindTexture(GL_TEXTURE_2D, BALL_TEXTURE_ID);
    }
    
    GLUquadricObj* quadObj;
    
    quadObj = gluNewQuadric();
    gluQuadricDrawStyle(quadObj, GLU_FILL);
    gluQuadricNormals(quadObj, GLU_SMOOTH);
    gluQuadricOrientation(quadObj, GLU_OUTSIDE);
    gluQuadricTexture(quadObj, GL_TRUE);
    
    gluSphere(quadObj, size, 15, 15);
    
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

//  glColor4fv( params ); // restore the color
}

// array of widths for specifying the board drawing parameters. 
double widtharr[] = { 0., BOXSIZE, 2.*BOXSIZE, 3.*BOXSIZE, 4.*BOXSIZE, 5.*BOXSIZE };

//
// draw the four vertical columns at the board corners
//
void GLBox::DrawSupportColumns()
{
    glColor3f(1.f, 1.0f, 1.f);
    
    // Turn on texturing
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, COL_TEXTURE_ID);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    GLUquadricObj *col = gluNewQuadric();
    gluQuadricNormals(col, GLU_SMOOTH);
    gluQuadricTexture(col, GL_TRUE);
    
    GLfloat BEAD_HEIGHT = (float)0.2;
    
    // The height of each column will be the height of the boards plus TOPHEIGHT
    glPushMatrix();
    glTranslatef(5. * BOXSIZE, 0., 5. * BOXSIZE);
    glRotated(-90, 1., 0., 0.);
    gluCylinder(col, COLUMN_RADIUS, COLUMN_RADIUS, BOARD0_H - BOARD4_H, 15, 15);
    glTranslated(0., 0. , 0. - BEAD_HEIGHT);
    gluSphere(col, 0.2, 15, 15);
    glTranslated(0., 0. , BOARD0_H - BOARD4_H + 2 * BEAD_HEIGHT);
    gluSphere(col, 0.2, 15, 15);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef( 0., 0., 5. * BOXSIZE );
    glRotated(-90, 1., 0., 0.);
    gluCylinder(col, COLUMN_RADIUS, COLUMN_RADIUS, BOARD0_H - BOARD4_H, 15, 15);
    glTranslated(0., 0. , 0. - BEAD_HEIGHT);
    gluSphere(col, 0.2, 15, 15);
    glTranslated(0., 0. , BOARD0_H - BOARD4_H + 2 * BEAD_HEIGHT);
    gluSphere(col, 0.2, 15, 15);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(5. * BOXSIZE, 0., 0.);
    glRotated(-90, 1., 0., 0.);
    gluCylinder(col, COLUMN_RADIUS, COLUMN_RADIUS, BOARD0_H - BOARD4_H, 15, 15);
    glTranslated(0., 0. , 0. - BEAD_HEIGHT);
    gluSphere(col, 0.2, 15, 15);
    glTranslated(0., 0. , BOARD0_H - BOARD4_H + 2 * BEAD_HEIGHT);
    gluSphere(col, 0.2, 15, 15);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(0., 0., 0.);
    glRotated(-90, 1., 0., 0.);
    gluCylinder(col, COLUMN_RADIUS, COLUMN_RADIUS, BOARD0_H - BOARD4_H, 15, 15);
    glTranslated(0., 0. , 0. - BEAD_HEIGHT);
    gluSphere(col, 0.2, 15, 15);
    glTranslated(0., 0., BOARD0_H - BOARD4_H + 2 * BEAD_HEIGHT);
    gluSphere(col, 0.2, 15, 15);
    glPopMatrix();
    
    glDisable(GL_TEXTURE_2D);
}

//
// Go through the points list and draw at those points
//
void GLBox::DrawMarbles()
{
//	TRACE("[DrawMarbles]: we have %d points\n", game->plist->count());

    Points *marble = game->plist;
    bool first = true;
    
    while (marble != NULL)
    {
        if (first)
        { 
            // We want to draw the first dot (the most recent piece) extra big to make it stand out
            DrawMarble(marble->getx(), marble->gety(), marble->getz(), marble->getr(), marble->getg(), marble->getb(), NEW_MOVE_SIZE);
            first = false;
        }
        else
        {
            DrawMarble(marble->getx(), marble->gety(), marble->getz(), marble->getr(), marble->getg(), marble->getb(), OLD_MOVE_SIZE);
        }
        marble = marble->GetNext();
    }
}

//
// DrawBoard() - draws the board matrix
//
void GLBox::DrawBoard(bool colorpickmode)
{
    // The normal vector for all the boards will be just the y axis basis vector
    glNormal3f(0.0, 1.0, 0.0);
    
    // Draw five 5x5 levels to make up a 3D tic-tac-toe game board
    // We draw the levels at different height intervals to improve display in 3d view
    // modified to display each board in a slightly different shade - helps to 
    // distinguish the boards when you look through a hole. 
    glColor3f((GLfloat)0.0, (GLfloat)0.0, (GLfloat)0.6);
    DrawLevel(BOARD4_H, colorpickmode);
    
    glColor3f((GLfloat)0.0, (GLfloat)0.0, (GLfloat)0.7);
    DrawLevel(BOARD3_H, colorpickmode);
    
    glColor3f((GLfloat)0.0, (GLfloat)0.0, (GLfloat)0.8);
    DrawLevel(BOARD2_H, colorpickmode);
    
    glColor3f((GLfloat)0.0, (GLfloat)0.0, (GLfloat)0.9);
    DrawLevel(BOARD1_H, colorpickmode);

    glColor3f((GLfloat)0.0, (GLfloat)0.0, (GLfloat)1.0);
    DrawLevel(BOARD0_H, colorpickmode);
}

//
// DrawSquare() Draws a single square of the board. called from DrawLevel()
// OpenGL has trouble with drawing concave polygons, so I have to draw 
// this (the square inscribed with a circular hole) as a combination of 
// lots of polygons (tesselation). 
//
void GLBox::DrawSquare(int i, int j, GLfloat y)
{
    // Oigin is the center of the square
    GLfloat originX = GLfloat(widtharr[i] + BOXSIZE / 2.);
    GLfloat originY = GLfloat(widtharr[j] + BOXSIZE / 2.);
    GLfloat radius = (GLfloat)BOARD_HOLE_RADIUS;
    GLfloat circleX, circleY;
    double angle;
    double maxangle = (2.0f * 3.14159);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//  glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
//  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    GLfloat increment;
    increment = (GLfloat)(BOXSIZE / (((3*maxangle/8. - maxangle/8.) / CIRCLE_STEPSIZE) + 1));
    GLfloat curEdge = 0.0;
    
    for (angle = maxangle/8.; angle<= 3*maxangle/8.; angle += CIRCLE_STEPSIZE)
    {
        // to fix floating point error
        if (BOXSIZE - curEdge < 0.05)
        {
            curEdge = BOXSIZE;
        }
        
        glBegin( GL_POLYGON );
        
        circleX = originX + (radius * (float)sin((double)angle));
        circleY = originY + (radius * (float)cos((double)angle));
        glVertex3f(circleX, y, circleY); // vertex on circle
        
        glVertex3f((GLfloat)widtharr[i+1], y, (GLfloat)(widtharr[j+1] - curEdge)); // vertex on box edge
        curEdge += increment;
        
        if (BOXSIZE - curEdge < 0.05)
        {
            curEdge = BOXSIZE;
        }
        
        circleX = originX + (radius * (float)sin((double)angle + CIRCLE_STEPSIZE));
        circleY = originY + (radius * (float)cos((double)angle + CIRCLE_STEPSIZE));
        
        glVertex3f((GLfloat)widtharr[i+1], y, (GLfloat)(widtharr[j+1] - curEdge));
        glVertex3f(circleX, y, circleY);
        
        glEnd();
    }
    
    // Second quadrant... 
    increment = (GLfloat)(BOXSIZE / (((5*maxangle/8. - 3*maxangle/8.) / CIRCLE_STEPSIZE) + 1 ));
    curEdge = 0.0;
    for (angle = 3*maxangle/8.; angle<= 5*maxangle/8.; angle += CIRCLE_STEPSIZE)
    {
        // to fix floating point error
        if (BOXSIZE - curEdge < 0.05)
        {
            curEdge = BOXSIZE;
        }
        glBegin( GL_POLYGON );
        
        circleX = originX + (radius * (float)sin((double)angle ));
        circleY = originY + (radius * (float)cos((double)angle ));	
        glVertex3f(circleX, y, circleY); // vertex on circle
        
        glVertex3f((GLfloat)(widtharr[i+1] - curEdge), y, (GLfloat)widtharr[j]); // vertex on box edge
        curEdge += increment;
        if (BOXSIZE - curEdge < 0.05)
        {
            curEdge = BOXSIZE;
        }
        
        circleX = originX + (radius * (float)sin((double)angle + CIRCLE_STEPSIZE));
        circleY = originY + (radius * (float)cos((double)angle + CIRCLE_STEPSIZE));
        
        glVertex3f((GLfloat)(widtharr[i+1] - curEdge), y, (GLfloat)widtharr[j]);
        glVertex3f(circleX, y, circleY);
        
        glEnd();
    }
    
    // Third quadrant...
    increment = (GLfloat)(BOXSIZE / (((7*maxangle/8. - 5*maxangle/8.) / CIRCLE_STEPSIZE) + 1) );
    curEdge = 0.0;
    for (angle = 5*maxangle/8.; angle<= 7*maxangle/8.; angle += CIRCLE_STEPSIZE)
    {
        // to fix floating point error
        if (BOXSIZE - curEdge < 0.05)
        {
            curEdge = BOXSIZE;
        }
        glBegin(GL_POLYGON);
        
        circleX = originX + (radius * (float)sin((double)angle));
        circleY = originY + (radius * (float)cos((double)angle));
        glVertex3f( circleX, y, circleY ); // vertex on circle
        
        glVertex3f((GLfloat)widtharr[i], y, (GLfloat)(widtharr[j] + curEdge)); // vertex on box edge
        curEdge += increment;
        if (BOXSIZE - curEdge < 0.05)
        {
            curEdge = BOXSIZE;
        }
        
        circleX = originX + (radius * (float)sin((double)angle + CIRCLE_STEPSIZE));
        circleY = originY + (radius * (float)cos((double)angle + CIRCLE_STEPSIZE));
        
        glVertex3f((GLfloat)widtharr[i], y, (GLfloat)(widtharr[j] + curEdge));
        glVertex3f(circleX, y, circleY);
        
        glEnd();
    }
    
    // Fourth quadrant... (this one loops around the circle past angle 0)
    increment = (GLfloat)(BOXSIZE / (((7*maxangle/8. - 5*maxangle/8.)/CIRCLE_STEPSIZE)+1));
    curEdge = 0.0;
    for (angle = 7*maxangle/8.; angle<= 9*maxangle/8.; angle += CIRCLE_STEPSIZE)
    {
        // to fix floating point error
        if (BOXSIZE - curEdge < 0.05)
        {
            curEdge = BOXSIZE;
        }
        
        glBegin( GL_POLYGON );
        
        circleX = originX + (radius * (float)sin((double)angle));
        circleY = originY + (radius * (float)cos((double)angle));
        glVertex3f(circleX, y, circleY); // vertex on circle
        
        glVertex3f((GLfloat)(widtharr[i] + curEdge), y, (GLfloat)widtharr[j+1]); // vertex on box edge
        curEdge += increment;
        if (BOXSIZE - curEdge < 0.05)
        {
            curEdge = BOXSIZE;
        }
        
        circleX = originX + (radius * (float)sin((double)angle + CIRCLE_STEPSIZE));
        circleY = originY + (radius * (float)cos((double)angle + CIRCLE_STEPSIZE));
        
        glVertex3f((GLfloat)(widtharr[i] + curEdge), y, (GLfloat)widtharr[j+1]);
        glVertex3f(circleX, y, circleY);
        
        glEnd();
    }
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(1.0);
    
    // Draw a border around the square to eliminate dots from floating point error
    glBegin(GL_POLYGON);
    
    // Outline of the square: 
    glVertex3d(widtharr[i],   y, widtharr[j]);

    glVertex3d(widtharr[i+1], y, widtharr[j]);

    glVertex3d(widtharr[i+1], y, widtharr[j+1]);

    glVertex3d(widtharr[i],   y, widtharr[j+1]);

    glEnd();
}

//
// This is used for drawing a square of the board without a hole in the center
// It's used just for the colormap. 
//
void GLBox::DrawNormalSquare(int i, int j, GLfloat y)
{
    // Draw a border around the square to eliminate dots from floating point error
    glBegin( GL_POLYGON );
    
    // Outline of the square: 
    glVertex3d(widtharr[i],   y, widtharr[j]);
    glVertex3d(widtharr[i+1], y, widtharr[j]);
    glVertex3d(widtharr[i+1], y, widtharr[j+1]);
    glVertex3d(widtharr[i],   y, widtharr[j+1]);
    glEnd();
}

//
// Width is the width of a single box
// z is the z dimension to draw this level on
//
void GLBox::DrawLevel(double y, bool colorpickmode)
{
    // Draw squares
    if (!colorpickmode)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    // In colorpickmode we'll use the default, which is GL_FILL
    for (int i = 0; i < NUM_HOLES_PER_LINE; i++)
    {
        for (int j = 0; j < NUM_HOLES_PER_LINE; j++)
        {
            if (colorpickmode)
            {
                glColor3ub(getLevelColor(y), (i+1) * 40, (j+1) * 40);
                DrawNormalSquare(i, j, (GLfloat)y);
            }
            else
            {
                DrawSquare(i, j, (GLfloat)y);
            }		
        }
    }
}

//
// Draw lines along the x y and z axes.
// Simply useful for visualization during debugging.
//
void GLBox::DrawAxes(double len)
{
    GLfloat params[4];
    glGetFloatv(GL_CURRENT_COLOR, params);
    
    glBegin(GL_LINES);
    
    // Z:
    glColor3f(1.0, 0.0, 0.0);
    glVertex3d(0, 0, -len);
    glVertex3d(0, 0,  len);
    
    // Y:
    glColor3f(0.0, 1.0, 0.0);
    glVertex3d(0, -len, 0);
    glVertex3d(0,  len, 0);
    
    // X:
    glColor3f(0.0, 0.0, 1.0);
    glVertex3d(-len, 0, 0);
    glVertex3d( len, 0, 0);
    
    glEnd();
    
    // Restore the color
    glColor4fv( params );
}

void GLBox::RotateRight(void)
{
    lr_angle = (lr_angle + 10) % 360;
}

void GLBox::RotateLeft(void)
{
    lr_angle = (lr_angle - 10) % 360;
}

void GLBox::RotateUp(void)
{
    ud_angle = (ud_angle - 10) % 360;
}

void GLBox::RotateDown(void)
{
    ud_angle = (ud_angle + 10) % 360;
}

//
// ShowWin() function is called when game is over
// moves is the number of moves it took
//
void GLBox::ShowWin(int moves)
{
    char msg[40];
    char sound[20];
    
    TRACE("[ShowWin]: called with %d moves\n", moves - 1);
    
    sprintf(sound, "win.wav");
     
    if (moves == -1)
    {
        // Then it was a tie
        sprintf(msg, "It's a tie!\n");
        TRACE("%s", msg);
        GLBox::updateGL();
        PlayGameSound(sound);
        return;
    }
    
    AddWinLine(game->getWonRow(), CurPlayer);
    updateGL();
	// jdavin:had commented out updateGL because the 3D5Dlg OnPaint event was causing 
	// an assertion failure in wincore.cpp. I think this might be because this function
	// was getting called from the CompMoveProc thread, which might not have ownership 
	//  over the window. It is not necessary to updateGL here anyway, window will get
	//  updated back in the main thread. 
    
    sprintf(msg, "Player %d Wins!\nTotal moves: %d\n", CurPlayer + 1, moves - 3);
    PlayGameSound(sound);
    MessageBox(m_hWnd, msg, "Winner!", MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL);

    sprintf(msg, "Player %d wins in %d moves!", CurPlayer + 1, moves - 2);
    m_ListBox->AddString("");
    m_ListBox->AddString(msg);
}

//
// BlockWin() function a force move is required
//
void GLBox::BlockWin(int gamecoord[])
{
    TRACE("[BlockWin]: called\n");
    GLBox::updateGL();
}

//
// AddWinLine() Finds the proper win line and stores it
// int row is the number of the row that won
//
void GLBox::AddWinLine(int row, int winner)
{
    TRACE("\n[AddWinLine]: called with row = %d\n", row);
    
    // Uncomment this if decide to implement player colored lines
    float r, g, b;
    
    switch (winner)
    {
    case PLAYER1:
        r = P1_r;
        g = P1_g;
        b = P1_b;
        break;
        
    case PLAYER2:
        r = P2_r;
        g = P2_g;
        b = P2_b;
        break;
        
    case PLAYER3:
        r = P3_r;
        g = P3_g;
        b = P3_b;
        break;
    }
    
    Point *p, *gam;
    
    switch (row)
    {
    // Horizontal across rows
    case 0:  case 1:  case 2:  case 3:  case 4:
    case 5:  case 6:  case 7:  case 8:  case 9:
    case 10: case 11: case 12: case 13: case 14:
    case 15: case 16: case 17: case 18: case 19:
    case 20: case 21: case 22: case 23: case 24:
        gam = new Point(0, row % 5, row / 5); // row/5 will get rounded down to the nearest integer
        p = mapGame2gl(gam);
        p->y += BALL_ELEVATION_OFFSET; // So the the line is elavated a bit
        p->x = 0.5; // So that the line starts at the beginning of  the row
        winline = new Line(p, 0.0, 90.0, 0.0, BOXSIZE4);
        break;
        
    // Horizontal down rows
    case 25: case 26: case 27: case 28: case 29:
    case 30: case 31: case 32: case 33: case 34:
    case 35: case 36: case 37: case 38: case 39:
    case 40: case 41: case 42: case 43: case 44:
    case 45: case 46: case 47: case 48: case 49:
        gam = new Point(row % 5, 0, (row - 25) / 5);
        p = mapGame2gl(gam);
        p->y += BALL_ELEVATION_OFFSET; // So the the line is elavated a bit
        p->z += BALL_ELEVATION_OFFSET; // So that the line starts at the beginning of the marble
        winline = new Line(p, 0.0, 0.0, 1.0, BOXSIZE4);
        break;
        
    // Vertical down rows
    case 50: case 51: case 52: case 53: case 54:
    case 55: case 56: case 57: case 58: case 59:
    case 60: case 61: case 62: case 63: case 64:
    case 65: case 66: case 67: case 68: case 69:
    case 70: case 71: case 72: case 73: case 74:
        gam = new Point(row % 5, (row - 50) / 5, 0);
        p = mapGame2gl(gam);
        winline = new Line(p, 90.0, 0.0, 0.0, BOARD0_H); // Line len is height of board0
        break;
        
    // Down diagonals
    case 75: case 76: case 77: case 78: case 79:
        gam = new Point(0, 0, row % 5);
        p = mapGame2gl(gam);
        p->y += BALL_ELEVATION_OFFSET;
        p->x = p->z = 0.5; // So that the line starts inside the marble
        winline = new Line(p, 0.0, 45.0, 0.0, sqrt(pow(BOXSIZE4, 2) + pow(BOXSIZE4, 2)));
        break;
        
    // Across diagonals
    case 80: case 81: case 82: case 83: case 84:
        gam = new Point(0, 4, row % 5);
        p = mapGame2gl(gam);
        p->y += BALL_ELEVATION_OFFSET;
        p->x = 0.5; // So that the line starts inside the marble
        p->z = BOXSIZE5 - 0.5;
        winline = new Line(p, 0.0, 135.0, 0.0, sqrt(pow(BOXSIZE4, 2) + pow(BOXSIZE4, 2)));
        break;
        
    // Vertical diagonals along x axis
    case 85: case 86: case 87: case 88: case 89:
        gam = new Point(0, row % 5, 0);
        p = mapGame2gl( gam );
        winline = new Line(p, 90.0, 0.0, 0 + tan(BOXSIZE4 / BOARD0_H) * 180. / PI - 2, sqrt(pow(BOARD0_H, 2) + pow(BOXSIZE4, 2)));
        break;
        
    // Vertical diagonals going back along x axis
    case 90: case 91: case 92: case 93: case 94:
        gam = new Point(4, row % 5, 0);
        p = mapGame2gl( gam );
        winline = new Line(p, 90.0, 0.0, 0 - tan(BOXSIZE4 / BOARD0_H) * 180. / PI + 2, sqrt(pow(BOARD0_H, 2) + pow(BOXSIZE4, 2)));
        break;
        
    // Vertical diagonals going forward along z axis
    case 95: case 96: case 97: case 98: case 99:
        gam = new Point(row % 5, 0, 0);
        p = mapGame2gl( gam );
        winline = new Line(p, 90.0 - tan(BOXSIZE4 / BOARD0_H) * 180. / PI + 2, 0.0, 0.0, sqrt(pow(BOARD0_H, 2) + pow(BOXSIZE4, 2)));
        break;
        
    // Vertical diagonals going back along z axis
    case 100: case 101: case 102: case 103: case 104:
        gam = new Point(row % 5, 4, 0);
        p = mapGame2gl(gam);
        winline = new Line(p, 90.0 + tan(BOXSIZE4 / BOARD0_H) * 180. / PI - 2, 0.0, 0.0, sqrt(pow(BOARD0_H, 2) + pow( BOXSIZE4, 2)));
        break;
        
    // Four "Double Diagonals"
    case 105:
        gam = new Point(0, 0, 0);
        p = mapGame2gl(gam);
        winline = new Line(p, 90.0 - tan((sqrt(pow( BOXSIZE4, 2 ) + pow( BOXSIZE4, 2 )) ) / BOARD0_H ) * 180. / PI + 6.0,   45.0, 0.0, sqrt(pow( BOARD0_H, 2 ) + pow((sqrt(pow( BOXSIZE4, 2 ) + pow( BOXSIZE4, 2 ))), 2)));
        break;
        
    case 106:
        gam = new Point(0, 4, 0);
        p = mapGame2gl(gam);
        winline = new Line(p, 90.0 - tan((sqrt(pow( BOXSIZE4, 2 ) + pow( BOXSIZE4, 2 )) ) / BOARD0_H ) * 180. / PI + 6.0,  135.0, 0.0, sqrt(pow( BOARD0_H, 2 ) + pow((sqrt(pow( BOXSIZE4, 2 ) + pow( BOXSIZE4, 2 ))), 2)));
        break;
        
    case 107:
        gam = new Point(4, 4, 0);
        p = mapGame2gl(gam);
        winline = new Line(p, 90.0 + tan((sqrt(pow( BOXSIZE4, 2 ) + pow( BOXSIZE4, 2 )) ) / BOARD0_H ) * 180. / PI - 6.0,   45.0, 0.0, sqrt(pow( BOARD0_H, 2 ) + pow((sqrt(pow( BOXSIZE4, 2 ) + pow( BOXSIZE4, 2 ))), 2)));
        break;
        
    case 108:
        gam = new Point(4, 0, 0);
        p = mapGame2gl(gam);
        winline = new Line(p, 90.0 - tan((sqrt(pow( BOXSIZE4, 2 ) + pow( BOXSIZE4, 2 )) ) / BOARD0_H ) * 180. / PI + 6.0,  -45.0, 0.0, sqrt(pow( BOARD0_H, 2 ) + pow((sqrt(pow( BOXSIZE4, 2 ) + pow( BOXSIZE4, 2 ))), 2)));
        break;
        
    default: 
        TRACE( "[AddWinLine]: Error- Encountered unknown row number\n" );
		assert(0);
        break;
    }
    
    delete p;
    delete gam;
}

//
// Draws the win line, if there is one set
//
void GLBox::DrawWinningLine()
{
    if (winline == NULL) // Then haven't won yet
    {
        return;
    }
    
//  GLfloat params[4];
//  glGetFloatv(GL_CURRENT_COLOR, params);
//  glColor3f(1.0, 1.0, 1.0);
    
    GLfloat color[] = {1.0, 1.0, 1.0, 1.0};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, color);
    
    glPushMatrix();
    
    glTranslated( winline->orig->x, winline->orig->y, winline->orig->z );
    glRotated( winline->zangle, 0.0, 0.0, 1.0 ); // z axis
    glRotated( winline->yangle, 0.0, 1.0, 0.0 ); // y axis
    glRotated( winline->xangle, 1.0, 0.0, 0.0 ); // rotate around x axis
    
    GLUquadricObj *quadObj;
    
    quadObj = gluNewQuadric();
    gluQuadricDrawStyle(quadObj, GLU_FILL);
    gluQuadricNormals(quadObj, GLU_SMOOTH);
    gluQuadricOrientation(quadObj, GLU_OUTSIDE);
    gluCylinder(quadObj, 0.05, 0.05, winline->length, NUM_HOLES_PER_LINE, NUM_HOLES_PER_LINE);
    
    glPopMatrix();
    
//  glColor4fv(params); // restore the color
}

//
// Play a wave file using the wave resource
//
void GLBox::PlayGameSound(char *aSoundFile)
{
    static LPCTSTR  lpRes;
    static HRSRC    hResInfo;
    static HGLOBAL  hRes;
//  int             rc;
    
    if (!SOUND) // if sound is turned off, just return
    {
        return;
    }
    
    // Find the wave resource
    hResInfo = FindResource(g_hInst, (const char *) IDR_WIN_WAV, _T("WIN.WAV"));
    if (hResInfo == NULL)
    {
        return;
    }
    
    // Load the wave resource
    hRes = LoadResource(g_hInst, hResInfo);
    if (hRes == NULL)
    {
        return;
    }
    
    // Lock the wave resource and play it. 
    lpRes = (LPCTSTR)LockResource(hRes);
    if (lpRes != NULL)
    {
//      rc = sndPlaySound(lpRes, SND_RESOURCE | SND_MEMORY);
    }
    
    if (hRes)
    {
        GlobalFree( hRes );
    }
}

//
// DrawBox() Draws a rectangular box with the given x, y, and z ranges.  
// The box is axis-aligned.
//
void GLBox::DrawBox(double x0, double x1,
                    double y0, double y1,
                    double z0, double z1,
                    enum type)
{
    static double n[6][3] =
    {
        {-1.0,  0.0, 0.0}, {0.0, 1.0, 0.0}, {1.0, 0.0,  0.0},
        { 0.0, -1.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, -1.0}
    };
    
    static GLint faces[6][4] =
    {
        {0, 1, 2, 3}, {3, 2, 6, 7}, {7, 6, 5, 4},
        {4, 5, 1, 0}, {5, 6, 2, 1}, {7, 4, 0, 3}
    };
    
    double v[8][3], tmp;
    GLint i;
    
    if (x0 > x1)
    {
        tmp = x0; x0 = x1; x1 = tmp;
    }
    
    if (y0 > y1)
    {
        tmp = y0; y0 = y1; y1 = tmp; 
    }
    
    if (z0 > z1)
    {
        tmp = z0; z0 = z1; z1 = tmp; 
    }
    
    v[0][0] = v[1][0] = v[2][0] = v[3][0] = x0;
    v[4][0] = v[5][0] = v[6][0] = v[7][0] = x1;
    v[0][1] = v[1][1] = v[4][1] = v[5][1] = y0;
    v[2][1] = v[3][1] = v[6][1] = v[7][1] = y1;
    v[0][2] = v[3][2] = v[4][2] = v[7][2] = z0;
    v[1][2] = v[2][2] = v[5][2] = v[6][2] = z1;
    
    for (i = 0; i < 6; i++)
    {
        glBegin( type );
        glNormal3dv( &n[i][0] );
        glVertex3dv( &v[faces[i][0]][0] );
        glNormal3dv( &n[i][0] );
        glVertex3dv( &v[faces[i][1]][0] );
        glNormal3dv( &n[i][0] );
        glVertex3dv( &v[faces[i][2]][0] );
        glNormal3dv( &n[i][0] );
        glVertex3dv( &v[faces[i][3]][0] );
        glEnd();
    }
}

//
// Maps a square number to x,y,z coordinates
//
void GLBox::mapsqr2coord(int BoardPositionNumber, double &x, double &y, double &z)
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
        z = 4.0 * BOXSIZE + BOXSIZE / 2.0;
}

//
// Maps a square number to x,y,z indices for the game engine
// x,y,z range from 0 to 3. 
//
void GLBox::mapsqr2gamecoord(int BoardPositionNumber, int &x, int &y, int &z)
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
Point* GLBox::mapGame2gl(Point *game)
{
    double x,y,z;
    
    mapGamecoord2gl((int)game->x, (int)game->y, (int)game->z, x, y, z);
    
    Point *p = new Point(x, y, z);
    return p;
}

//
// Maps game coordinates (0 to 3) to 3d gl coordinates
//
void GLBox::mapGamecoord2gl(int game_x, int game_y, int game_z, double &x, double &y, double &z)
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
void GLBox::mapGL2gamecoord(double x, double y, double z, int &game_x, int &game_y, int &game_z)
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

/////////////////////////////////
// Points Class

Points::Points()
{
    x = 0;
    y = 0;
    z = 0;
    r = 1.0;  // set color to a default of red
    g = 0.0;
    b = 0.0;
    //prev = this;
    next = NULL;
}

Points::~Points()
{
}

Points::Points(double ix, double iy, double iz)
{
    x = ix;
    y = iy;
    z = iz;
    r = 1.0;  // set color to a default of red
    g = 0.0;
    b = 0.0;
//  prev = this;
    next = NULL;
}

Points::Points(double ix, double iy, double iz, float ir, float ig, float ib)
{
    x = ix;
    y = iy;
    z = iz;
    r = ir;  
    g = ig;
    b = ib;
//  prev = this;
    next = NULL;
}

Points * Points::add(double ix, double iy, double iz)
{
    Points *rtnPtr = new Points();
    rtnPtr->x = ix;
    rtnPtr->y = iy;
    rtnPtr->z = iz;
    rtnPtr->r = 1.0;  
    rtnPtr->g = 0.0; // default color
    rtnPtr->b = 0.0;
    
    rtnPtr->next = this;
//  rtnPtr->prev = oldPtr->next;

    return rtnPtr;
}

Points * Points::add(double ix, double iy, double iz, float ir, float ig, float ib)
{
    Points *rtnPtr = new Points();

    rtnPtr->x = ix;
    rtnPtr->y = iy;
    rtnPtr->z = iz;
    rtnPtr->r = ir;  
    rtnPtr->g = ig;
    rtnPtr->b = ib;

    rtnPtr->next = this;
//  rtnPtr->prev = oldPtr->next;
    
    return rtnPtr;
}

//
// removes the head of the list and returns the new head pointer
//
Points *Points::deleteHead()
{
    Points *newPtr = this->next;
	delete this;
	return newPtr;
}

/*
BOOL Points::del(Points *curPtr)
{
    Points * oldPtr = curPtr->GetPrev();
    Points * newPtr = curPtr->GetNext();

    delete newPtr;

    if (oldPtr != NULL && oldPtr->next == NULL)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
*/

/*
void Points::deleteall(void)
{
    if (this->next != NULL)
    {
        del( this );
    }
}*/

void Points::deleteall(void)
{
	if (this->next != NULL)
	{
		this->next->deleteall();
	}
	delete this;
}

//
// A search function. Returns true if list contains the point x,y,z
//
bool Points::contains(double xi, double yi, double zi, float r_i, float g_i, float b_i)
{
    if (x == xi  && y == yi  && z == zi &&
        r == r_i && g == g_i && b == b_i)
    {
        return true;
    }
    
    if (next != NULL)
    {
        return next->contains(xi, yi, zi, r_i, g_i, b_i);
    }
    
    return false; // otherwise, return false
}

//
// Return number of points in list
//
int Points::count()
{
    if (next != NULL)
    {
        return 1 + next->count();
    }
    else
    {
        return 1;
    }
}

/*
Points * Points::GetPrev()
{
    return (this->prev);
}
*/

Points * Points::GetNext()
{
    return (this->next);
}
