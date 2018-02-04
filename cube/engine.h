// Engine.h : This is the Engine class interface and related definitions
//
// John Davin, Terrence Wong, Jim Lundberg
// Copyright (c) 2002-2004 JRL Innovations  All Rights Reserved
// 3D5 - The FIVE level 3D tic-tac-toe game
//

#ifndef ENGINE_H
#define ENGINE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//
// This is the class for the main game computations (ie., all the game code)
// 

#include "glbox.h"

typedef enum
{
    PLAYER1=0,
    PLAYER2,
    PLAYER3,
}
PLAYER_ID;

typedef struct MoveListRowType
{
    int x;
    int y;
    int z;
} 
MoveListRow;

// Bit masks used to access bits for each square
// BIT0 is for the 1st least significant bit, BIT1 is for the second, etc... 

#define BIT0  0x0001
#define BIT1  0x0002m_ListBox
#define BIT2  0x0004
#define BIT3  0x0008
#define BIT4  0x0010
#define BIT5  0x0020
#define BIT6  0x0040
#define BIT7  0x0080
#define BIT8  0x0100
#define BIT9  0x0200
#define BIT10 0x0400
#define BIT11 0x0800
#define BIT12 0x1000
#define BIT13 0x2000
#define BIT14 0x4000
#define BIT15 0x8000

class Engine 
{
public: 
    Engine(QListView *listView);
    ~Engine();

    void getMove(int player, int &x, int &y, int &z);
    USHORT GetMask(int x, int player);

    void setDepth(DIFFICULTY_LEVELS depth);
    unsigned int getDepth();
    void debugmapping(int start=0, int end = NUM_COMBOS);
    void DumpLine(USHORT line, char * msg);
    int  CountMarbles(int Player, USHORT line);
    int  Evaluate(USHORT runboard[]);
    bool IsWon(USHORT board[]);
    bool IsFourInaRow(USHORT board[]);

    int  whogoesfirst();
    int  hasWon(); // Returns 0 if game is not won, otherwise returns number of moves
	bool hasFourOfFive(); // Returns true if 4 of 5

    int  IsBoardValid(USHORT ComboList[]);
    void PrintWhoWon(USHORT ComboList[]);

    void NewGame();
    int  getWonRow();
    void GetMoveCoord(int &x, int &y, int &z, int ComboIndex, int bitnum);

    bool Check4CriticalMove(int &a, int &b, int &c, int player, USHORT board[]);
    int  alphabeta(int &a, int &b, int &c, int player, USHORT ComboList[], int depth, int alpha, int beta);
    int  minimax(int &a, int &b, int &c, int player, USHORT board[], int depth);

    bool LegalMove(int x, int y, int z, int player, USHORT ComboList[], bool FreeSpace);
    bool makeMove(int player, int x,  int y,  int z, bool bFreeSpace);
    int  MakeMove(int x, int y, int z, int player, USHORT ComboList[], bool bFreeSpace);
    void SaveMove(int x, int y, int z);
    bool UndoMove();
    void GetPrevMove(int& x, int& y, int& z);
    void MakeSpaceFree(int x, int y, int z, USHORT ComboList[]);
    int  GetMoveIndex() { return moveIndex; }

    QListView  *m_ListBox;
    Points     *plist;                  // List of points to draw pieces at
    Line       *winline;                // The line we draw across pieces on win

    MoveListRow MoveList[NUM_HOLES_PER_GAME];

protected:
    int         moves;                  // Current number of moves
    USHORT      theboard[NUM_COMBOS];   // The game board position matrix
    int         DEPTH;
    int         moveIndex;              // Index into the Move table
};

#endif
