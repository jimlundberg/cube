// Engine.cpp - Graphic engine
//
// John Davin, Terrence Wong, Jim Lundberg
// Copyright (c) 2002-2004 JRL Innovations  All Rights Reserved
// 3D5 - The FIVE level 3D tic-tac-toe game
//

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
#include <math.h>
#include <iostream>
#include <time.h>
#include <limits.h>
#include <assert.h>
#include "engine.h"

Engine::Engine(QListView *aListBox)
{
    m_ListBox = aListBox;

    // Initialize board: 
    for (int i = 0; i < NUM_COMBOS; i++)
    {
        theboard[i] = (USHORT)0;
    }
    
    DEPTH       = ALPHABETA_DEPTH;        // Initialize alphabeta depth to default
//  moves       = 0;                      // Move counter
    plist       = NULL;
    moveIndex   = 0;

    srand( (int)time(NULL) );            // Seed the random number generator
}

Engine::~Engine()
{
    if (plist != NULL)
    {
        plist->deleteall();
    }
}

//
// Returns true if any player has won
//
bool Engine::IsWon(USHORT board[])
{
//  DumpLine(board[55], "[IsWon]: Combo 55 ");

    for (int i = 0; i < NUM_COMBOS; i++)
    {
        USHORT row = board[i];
        
        // If all bits are 1 then Player 1 wins
        if (((row & PLAYER1_MASK) == PLAYER1_MASK) ||
            ((row & PLAYER2_MASK) == PLAYER2_MASK))
        {
            if ((row & PLAYER3_MASK) == PLAYER3_MASK)
            {
                continue;
            }
            else
            {
				TRACE("[IsWon]: board is won on line %d. p1: %d p2: %d p3: %d\n", i,
					(row & PLAYER1_MASK) == PLAYER1_MASK, 
					(row & PLAYER2_MASK) == PLAYER2_MASK,
					(row & PLAYER3_MASK) == PLAYER3_MASK);

				TRACE("[IsWon]: board evaluated to %d\n", Evaluate(board));

				//debugmapping(15);
				//ASSERT(0);
				//debugmapping(54, 55);

                return true;
            }
        }
    }
    
    // If we get here, then there were no winning rows
    return false;
}

//
// Returns true if any player has four-of-five-in-a-row
//
bool Engine::IsFourInaRow(USHORT board[])
{
//  DumpLine(board[55], "[IsFourInaRow]: Combo 55 ");

    for (int i = 0; i < NUM_COMBOS; i++)
    {
        USHORT row = board[i];
        
		USHORT P1Row = row & PLAYER1_MASK;
		USHORT P2Row = row & PLAYER2_MASK;
        P2Row = P2Row >> 5;

		USHORT P1M1 = (P1Row & 0x01) >> 0;
		USHORT P1M2 = (P1Row & 0x02) >> 1;
		USHORT P1M3 = (P1Row & 0x04) >> 2;
		USHORT P1M4 = (P1Row & 0x08) >> 3;
		USHORT P1M5 = (P1Row & 0x10) >> 4;

		USHORT P2M1 = (P2Row & 0x01) >> 0;
		USHORT P2M2 = (P2Row & 0x02) >> 1;
		USHORT P2M3 = (P2Row & 0x04) >> 2;
		USHORT P2M4 = (P2Row & 0x08) >> 3;
		USHORT P2M5 = (P2Row & 0x10) >> 4;

        // If 4 bits are 1 then Player 1 has a force
        if ((P1M1 + P1M2 + P1M3 + P1M4 + P1M5 >= 4) ||
            (P2M1 + P2M2 + P2M3 + P2M4 + P2M5 >= 4))
		{
			TRACE("[IsFourInaRow]:has detected 4-in-a-row on line %d. p1: %d p2: %d p3: %d\n", i,
				(row & PLAYER1_MASK) == PLAYER1_MASK,
				(row & PLAYER2_MASK) == PLAYER2_MASK,
				(row & PLAYER3_MASK) == PLAYER3_MASK);

			//debugmapping(15);
			//ASSERT(0);
			//debugmapping(54, 55);

            return true;
		}
    }
    
    // If we get here, then there were no rows with four
    return false;
}

//
// Prints out player that won
//
void Engine::PrintWhoWon(USHORT ComboList[])
{
    for (int i = 0; i < NUM_COMBOS; i++)
    {
        if ((ComboList[i] & PLAYER1_MASK) == PLAYER1_MASK)
        {
            if ((ComboList[i] & PLAYER3_MASK) != PLAYER3_MASK)
            {
                printf("Player 1 wins!\n");
                m_ListBox->AddString("Player 1 wins!");
                return;
            }
        }

        if ((ComboList[i] & PLAYER2_MASK) == PLAYER2_MASK )
        {
            if ((ComboList[i] & PLAYER3_MASK) != PLAYER3_MASK )
            {
                printf("Player 2 wins!\n");
                m_ListBox->AddString("Player 2 wins!");
                return;
            }
        }
    }
    
    // If we get to here, then no winner was found. 
    printf("It's a tie! No one wins\n");
    m_ListBox->AddString("It's a tie! No one wins");
}

//
// Returns true if the move is legal 
// A move is legal if there is not already a piece in the square. 
// So we just need to check if the other player has a piece there. 
// 
bool Engine::LegalMove(int x, int y, int z, int player, USHORT ComboList[], bool bFreeSpace)
{
    //if (IsWon( ComboList ) && !bFreeSpace)
    //{
	//	TRACE("[LegalMove]: not legal move due to is won.\n");
    //    return false; // We don't want to keep making moves if someone already has a win. 
    //}
    
    // Just get the horizontal line: 
	int ComboIdx = y + (z * NUM_HOLES_PER_LINE);
    USHORT line = ComboList[ComboIdx];

    USHORT mask1, mask2, mask3;

    //  Old 8 bit Mask layout
    //  -----------------------------------------
    //  | P0 | P0 | P0 | P0 | P1 | P1 | P1 | P1 |
    //  ---7----6----5----4----3----2----1----0--
    //  ---------------------------------------------------------------------------------------
    //  |  F | P3 | P3 | P3  |  P3 | P3 | P2 | P2  |  P2 | P2 | P2 | P1  |  P1 | P1 | P1 | P1 |
    //  ---F----E----D----C------B----A----9----8------7----6----5----4------3----2----1----0--
    //  New 16 bit Mask layout
    
    // We have a legal move if the square is free (so check player and opponent's bits)
    switch (x)
    {
    case 0:
        mask1 = BIT0;   // Player 1 bit
        mask2 = BIT5;   // Player 2 bit
        mask3 = BIT10;  // Player 3/Freespace bit
        break;

    case 1:
        mask1 = BIT1;   // Player 1 bit
        mask2 = BIT6;   // Player 2 bit 
        mask3 = BIT11;  // Player 3/Freespace bit
        break;

    case 2:
        mask1 = BIT2;   // Player 1 bit
        mask2 = BIT7;   // Player 2 bit 
        mask3 = BIT12;  // Player 3/Freespace bit
        break;

    case 3:
        mask1 = BIT3;   // Player 1 bit
        mask2 = BIT8;   // Player 2 bit 
        mask3 = BIT13;  // Player 3/Freespace bit
        break;

    case 4:
        mask1 = BIT4;   // Player 1 bit
        mask2 = BIT9;   // Player 2 bit 
        mask3 = BIT14;  // Player 3/Freespace bit
        break;
    }

	/* TRACE("LegalMove: %d %d %d, ComboIdx: %d\n", x,y,z, ComboIdx);
	TRACE("LegalMove: p1 bit: %d p2 bit: %d p3 bit: %d freespace: %d\n",
			line & mask1, line & mask2, line & mask3, bFreeSpace);
	DumpLine(line, "[LegalMove]: ");
*/
    // Position is filled if any of the masks 
    return ( !((line & mask1) || (line & mask2) || (line & mask3))   || bFreeSpace);
}

//
// Returns the bit mask for the x'th bit
// basically a hash function used for MakeMove
//
USHORT Engine::GetMask(int x, int player)
{
    return (USHORT)((0x0001 << x) << (player * 5));
}

//
//  Representation:
//  
//  Both of the groups of four bits represents the four squares in the winning position i from low coordinate to high. The left four bits represent the player0 (human) [human opponent's] pieces. 0 means player0 occupies the square and 1 means not. [0 means human occupies the square and 1 means not.] The right 4 bits represent player1 [the computer player], and it is the opposite: 1 means player1 occupies the square and 0 means not. [0 means not you, and 1 means occupied.]
//  
//  Examples of old masks: 
//  0000 0000 is a winning position for human
//  1111 1111 is a winning position for computer
//  1111 0000 is a blank line
//  0000 1111 is impossible
//  0111 1000 is impossible (player0 and player1 both have piece in 1st square)
//  
//  these bits always represent increasing x from left to right when possible, ignoring the plane they're on. Otherwise, they represent increasing y then increasing z.
//
//  Examples of new masks:
//  F333 3322 2221 1111 is the player positions with the 'F' being the free-space indicator
//  0000 0000 0000 0000 is a blank line
//  0000 0000 0001 1111 is a winning line for player 1
//  0000 0011 1110 0000 is a winning line for player 2
//  0111 1100 0000 0000 is a winning line for player 3
//  1111 1100 0000 0000 is when the 3rd player position(s) are used as a free space
//  0000 0000 1100 0110 is impossible because it indicates P1 and P2 are both occupying the position
//  1001 0000 0000 0000 shows a free space indicator for the center position of the line
//

//
// If the move is legal, return 1 else return 0
// player is 0 if human, 1 if computer
//
int Engine::MakeMove(int x, int y, int z, int player, USHORT ComboList[], bool bFreeSpace)
{
	//TRACE("[MakeMove]: %d %d %d player %d\n", x,y,z, player);
	//DumpLine(ComboList[55], "[MakeMove1]: Combo 55 ");

    if (x < 0 || x > 4 || y < 0 || y > 4 || z < 0 || z > 4)
    {
		// this should probably only happen if user clicks outside the boundaries of the board
        TRACE( "Illegal Move! %d %d %d\n", x, y ,z );
		//ASSERT(0);
      //exit(0);
        return 0;
    }
    
    if (!LegalMove(x, y, z, player, ComboList, bFreeSpace))
    {
		TRACE("[MakeMove]: not legal move. returning.\n");
        return 0;
    }

    //SaveMove( x, y, z );
    // SaveMove can not go here!

    // We have the coordinate they input and the shiftx which is # positions to shift to access that bit
    
    // If the x position of the ComboList is unoccupied, make it occupied by "player"
    
    int xmask0 = GetMask(x, player); // Mask for player0 x bit
    int ymask0 = GetMask(y, player); // Mask for player0 y bit
    int zmask0 = GetMask(z, player); // Mask for player0 z bit
    
    // You know that any move must go into exactly one of each straight line winning position
    ComboList[y  +      (z*5) ] = ComboList[y  +      (z*5) ] | xmask0; // 0-24 left to right winning positions
    ComboList[25 + (x + (z*5))] = ComboList[25 + (x + (z*5))] | ymask0; //25-49 down the y axis
    ComboList[50 + (x + (y*5))] = ComboList[50 + (x + (y*5))] | zmask0; //50-74 down the z axis
    
    // Check to see if the move goes into some of the diagonal winning positions
    if (x == y)
    {
        // Then it is on one of the 5 diagonals on an xy plane going from top left to bottom right
        ComboList[75+z] = ComboList[75 + z] | xmask0;
    }

    if (x == (4-y))
    {
        // Then it is on one of the 5 diagonals on an xy plane going top right to bottom left
        ComboList[80+z] = ComboList[80 + z] | xmask0;
    }

    if (x == z)
    {
        // Diagonals along the xz plane (ex: 1,1,1 to 5,5,1 and 1,1,2 to 5,5,2 etc)
        ComboList[85+y] = ComboList[85 + y] | xmask0;
    }

    if (x == (4-z))
    {
        // Same as above but from 1,1,5 up to 1,5,1 etc. Bits left to right with increasing x
        ComboList[90+y] = ComboList[90 + y] | xmask0;
    }

    if (y == z)
    {
        // The diagonals from 1,1,1 down to 1,5,5 and 2,1,1 to 2,5,5 etc
        ComboList[95+x] = ComboList[95 + x] | ymask0;
    }

    if (y == (4-z))
    {
        // Same as above but from 1,1,5 up to 1,5,1
        ComboList[100+x] = ComboList[100 + x] | ymask0;
    }

    // 4 Double diagonals
    if (x == y && y == z)
    {
        // Diagonal from 1,1,1 to 5,5,5
        ComboList[105]   = ComboList[105] | xmask0;
    }

    if (x == (4-y) && y == (4-z))
    {
        // Diagonal from 1,5,5 to 5,1,1
        ComboList[106]   = ComboList[106] | xmask0;
    }

    if (x == y && x == (4-z))
    {
        // Diagonal from 1,1,5 to 5,5,1
        ComboList[107]   = ComboList[107] | xmask0;
    }

    if (x == (4-y) && y == z)
    {
        // Diagonal from 1,5,1 to 5,1,5
        ComboList[108]   = ComboList[108] | xmask0;
    }

	//DumpLine(ComboList[55], "[MakeMove2]: Combo 55 ");
	//TRACE("[MakeMove]: made move for p %d: %d %d %d\n", player, x, y, z);
    return 1;
}

//
// Make (x,y,z) a free space (remove any piece that is there according to the board ComboList[]
//
void Engine::MakeSpaceFree(int x, int y, int z, USHORT ComboList[])
{
	// we can assume x,y,z are valid because they come from the stored move list

	// get masks that affect all three players: 
    int xmask0 = GetMask( x, PLAYER1 )+GetMask( x, PLAYER2 )+GetMask( x, PLAYER3 );
    int ymask0 = GetMask( y, PLAYER1 )+GetMask( y, PLAYER2 )+GetMask( y, PLAYER3 );
    int zmask0 = GetMask( z, PLAYER1 )+GetMask( z, PLAYER2 )+GetMask( z, PLAYER3 );;
    
    // You know that any move must go into exactly one of each straight line winning position
    ComboList[y  +      (z*5) ] = ComboList[y  +      (z*5) ] & ~xmask0; // 0-24 left to right winning positions
    ComboList[25 + (x + (z*5))] = ComboList[25 + (x + (z*5))] & ~ymask0; //25-49 down the y axis
    ComboList[50 + (x + (y*5))] = ComboList[50 + (x + (y*5))] & ~zmask0; //50-74 down the z axis
    
    // Check to see if the move goes into some of the diagonal winning positions
    if (x == y)
    {
        // Then it is on one of the 5 diagonals on an xy plane going from top left to bottom right
        ComboList[75+z] = ComboList[75 + z] & ~xmask0;
    }

    if (x == (4-y))
    {
        // Then it is on one of the 5 diagonals on an xy plane going top right to bottom left
        ComboList[80+z] = ComboList[80 + z] & ~xmask0;
    }

    if (x == z)
    {
        // Diagonals along the xz plane (ex: 1,1,1 to 5,5,1 and 1,1,2 to 5,5,2 etc)
        ComboList[85+y] = ComboList[85 + y] & ~xmask0;
    }

    if (x == (4-z))
    {
        // Same as above but from 1,1,5 up to 1,5,1 etc. Bits left to right with increasing x
        ComboList[90+y] = ComboList[90 + y] & ~xmask0;
    }

    if (y == z)
    {
        // The diagonals from 1,1,1 down to 1,5,5 and 2,1,1 to 2,5,5 etc
        ComboList[95+x] = ComboList[95 + x] & ~ymask0;
    }

    if (y == (4-z))
    {
        // Same as above but from 1,1,5 up to 1,5,1
        ComboList[100+x] = ComboList[100 + x] & ~ymask0;
    }

    // 4 Double diagonals
    if (x == y && y == z)
    {
        // Diagonal from 1,1,1 to 5,5,5
        ComboList[105]   = ComboList[105] & ~xmask0;
    }

    if (x == (4-y) && y == (4-z))
    {
        // Diagonal from 1,5,5 to 5,1,1
        ComboList[106]   = ComboList[106] & ~xmask0;
    }

    if (x == y && x == (4-z))
    {
        // Diagonal from 1,1,5 to 5,5,1
        ComboList[107]   = ComboList[107] & ~xmask0;
    }

    if (x == (4-y) && y == z)
    {
        // Diagonal from 1,5,1 to 5,1,5
        ComboList[108]   = ComboList[108] & ~xmask0;
    }
}


void Engine::GetPrevMove(int& x, int& y, int& z)
{
    x = MoveList[moveIndex].x;
    y = MoveList[moveIndex].y;
    z = MoveList[moveIndex].z;
}

void Engine::SaveMove(int x, int y, int z)
{
	ASSERT(moveIndex >= 0 && moveIndex < NUM_HOLES_PER_GAME);
    MoveList[moveIndex].x = x;
    MoveList[moveIndex].y = y;
    MoveList[moveIndex].z = z;
    moveIndex++;
}

/*
 *
 * return true if succeeded, false o/w
 */
bool Engine::UndoMove()
{
	//moves--;
    if (moveIndex <= 0 )
    {   // no moves to undo
		return false;
	}
    moveIndex--;
	ASSERT(moveIndex >= 0 && moveIndex < NUM_HOLES_PER_GAME);
	MakeSpaceFree(MoveList[moveIndex].x, MoveList[moveIndex].y,MoveList[moveIndex].z, theboard);
    MoveList[moveIndex].x = 0;
    MoveList[moveIndex].y = 0;
    MoveList[moveIndex].z = 0;
    
	return true;
}

// Prints out a line from the ComboList array 
// (debugging function) 
//
void Engine::DumpLine(USHORT line, char *msg)
{
    char    str[80] = {0};
    int     k;

    bool freeSpace = ((line & 0x8000) == 0x8000);

    strcat(str, msg);
    
    for (k = 15; k >= 0; k--)
    {
        if ((line >> k) & 0x1)
        {
            strcat(str, "1");
        }
        else
        {
            strcat(str, "0");
        }
        
        if ((k == 5) || (k == 10) || (k == 15))
        {
            strcat(str, " ");
        }
    }
 
    strcat(str, "\n");

//  TRACE(str);
    m_ListBox->AddString(str);
}

//
// Returns false if ComboList is not valid
// just a debugging function (not used anywhere in code anymore)
//
int Engine::IsBoardValid(USHORT ComboList[])
{
    int sum = 0;
    
    for (int i = 0; i < NUM_COMBOS; i++)
    {
        if (!(ComboList[i] & BIT0) )
        {
            sum += ComboList[i] & BIT0;
        }
        
        if (!(ComboList[i] & BIT1) )
        {
            sum += ComboList[i] & BIT1;
        }
        
        if (!(ComboList[i] & BIT2) )
        {
            sum += ComboList[i] & BIT2;
        }
        
        if( !(ComboList[i] & BIT3) )
        {
            sum += ComboList[i] & BIT3;
        }

        if( !(ComboList[i] & BIT4) )
        {
            sum += ComboList[i] & BIT4;
        }
    }
    
    // If sum is greater than 0 then we had an error
    return !sum;
}

//
// Returns the number of marbles in a row of a selected player, 
// Return *positive* for *player2* and negative for player1
//
int Engine::CountMarbles(int Player, USHORT line)
{
    int sum = 0;
    
    // Check if 1st player has any pieces (a 1 in the first 5 bits) in their row
    if (Player == PLAYER1)
    {
        sum -= !!(line & BIT0) + !!(line & BIT1) + !!(line & BIT2) + !!(line & BIT3) + !!(line & BIT4);
    }
    // Check if 2nd player has any in their row
    else if (Player == PLAYER2) 
    {
        sum += !!(line & BIT5) + !!(line & BIT6) + !!(line & BIT7) + !!(line & BIT8) + !!(line & BIT9);
    }
	else
		ASSERT(!"INVALID PLAYER");
    // Check if 3rd player has any in their row
    // These bits are free spaces if bit 15 is set
    //else if (Player == PLAYER3)
    //{
    //    sum += !!(line & BIT10) + !!(line & BIT11) + !!(line & BIT12) + !!(line & BIT13) + !!(line & BIT14);
    //}
    // else we had mixed pieces, meaning the row is blocked

    return sum;
}

//
// Higher = good for player2, lower = bad for player2 (good for player1)
// Adds up the number of pieces in each possible winning row, 
// but only if there is no opponent piece blocking
//
int Engine::Evaluate(USHORT runboard[])
{
    USHORT  line;
    int     num = 0;
    int     sum = 0;

    for (int player = PLAYER1; player <= PLAYER2; player++)
    { // do nothing with player3 free squares? 
        for (int i = 0; i < NUM_COMBOS; i++)
        {
            line = runboard[i];        
            num = CountMarbles( player, line );
        
            if (num == 2 || num == -2)
            {
                sum += num;  // sum += 2 or -2
            }
            else if (num == 3 || num == -3)
            {
                sum += num * 2; // sum += 6 or -6
            }
            else if (num == 4 || num == -4)
            {
                sum += num * 3; // sum += 12 or -12
            }
			else if (num == 5 || num == -5)
            {
                // Then one player has won
                sum = num * 100000; // Return a big number
            }
        }
    }
    return sum;
}


// 
// Randomly determine player will go first
//
int Engine::whogoesfirst()
{
    return (rand() % 2);
}

//
// Given the board number (the array index in theboard[]) and the bit number of
// the position we're concerned with, this function will calculate the 
// x,y,z coordinates of the square. 
//
void Engine::GetMoveCoord(int &x, int &y, int &z, int ComboIndex, int bitnum)
{
//  printf( "[GetMoveCoord]: received ComboIndex=%d, bitnum=%d\n", ComboIndex, bitnum );
    
    int bitmask = 4 - (bitnum % NUM_HOLES_PER_LINE);

    x = y = z = 0;

    if (ComboIndex < 25)
    { 
        // Across the x axis (left to right)
        y = ComboIndex % NUM_HOLES_PER_LINE;
        z = ComboIndex / NUM_HOLES_PER_LINE;
        x = bitmask;
    }
    else if (ComboIndex < 50)
    { 
        // Down the y axis
        z = (ComboIndex - 25) / NUM_HOLES_PER_LINE;
        x = ComboIndex % NUM_HOLES_PER_LINE;
        y = bitmask;
    }
    else if (ComboIndex < 75)
    {
        // Down the z axis
        y = (ComboIndex - 50) / NUM_HOLES_PER_LINE;
        x = ComboIndex % NUM_HOLES_PER_LINE;
        z = bitmask;
    }
    else if (ComboIndex < 80 )
    {
        // 5 Diagonals from top left to bottom right on xy plane
        x = y = bitmask;
        z = ComboIndex - 75;
    }
    else if (ComboIndex < 85)
    {
        // 5 Diagonals from top right to bottom left on xy plane
        x = bitmask;
        y = 4 - bitmask;
        z = ComboIndex - 80;
    }
    else if (ComboIndex < 90)
    {
        // 5 Diagonals on xz plane
        x = z = bitmask;
        y = ComboIndex - 85;
    }
    else if (ComboIndex < 95)
    {
        // 5 Diagonals on xz plane in other direction
        x = bitmask;
        z = 4 - bitmask;
        y = ComboIndex - 90;
    }
    else if (ComboIndex < 100)
    {
        // The diagonals from 0,0,0 down to 0,4,4 and 1,0,0 to 1,4,4 etc
        x = ComboIndex - 95;
        y = z = bitmask;
    }
    else if (ComboIndex < 105)
    {
        // Same as above but from 0,0,4 up to 0,4,0
        x = ComboIndex - 100;
        y = bitmask;
        z = 4 - bitmask;
    }
    else if (ComboIndex == 105)
    {
        // Diagonal from 0,0,0 to 4,4,4
        x = y = z = bitmask;
    }
    else if (ComboIndex == 106)
    {
        // Diagonal from 0,4,4 to 4,0,0
        x = bitmask;
        y = 4 - bitmask;
        z = bitmask;
    }
    else if (ComboIndex == 107)
    {
        // Diagonal from 0,0,4 to 4,4,0
        x = y = bitmask;
        z = 4 - bitmask;
    }
    else if (ComboIndex == 108)
    { 
        // Diagonal from 0,4,0 to 4,0,4
        x = bitmask;
        y = z = 4 - bitmask;
    }
}

void Engine::debugmapping(int start, int end)
{
    int     ComboIndex;
    USHORT  bitnum;
    int     x, y, z;
    char    msg[255];

    m_ListBox->ResetContent();

    for (ComboIndex = start; ComboIndex < end; ComboIndex++)
    {
        USHORT line = theboard[ComboIndex];

        bitnum = line;
        
        GetMoveCoord(x, y, z, ComboIndex, bitnum);

        sprintf(msg, "Combo %03d @ %d,%d,%d %04X=", ComboIndex + 1, x+1, y+1, z+1, bitnum);
        DumpLine(line, msg);
    }
}

//
// Critical moves are moves that we absolutely must make (it would be stupid to 
// do otherwise).  If we have 3 pieces in a row (and the 4th square is empty), 
// then obviously we should move to the 4th square to win. 
// Otherwise, if we don't have a win then if the opponent has any unobstructed 
// rows with 3 pieces, then we should move to the 4th square to block. 
//
bool Engine::Check4CriticalMove(int &a, int &b, int &c, int player, USHORT ComboList[])
{
    int  Combinations;
    int  bitnum;
    bool ret = false;

    TRACE("[Check4CriticalMove]: called with player\n");

    for (Combinations = 0; Combinations < NUM_COMBOS; Combinations++)
    {
        USHORT line = theboard[Combinations];
        
		// if the current player is the computer, and *either* player has 
		//   a 4-in-a-row, make a move to block (or complete our 4-in-a-row)
        if ( player == COMPUTER && 
				( abs( CountMarbles( PLAYER1, line )) == 4 ||
                  abs( CountMarbles( PLAYER2, line )) == 4 ))
        {
            // Then we have a win so find the empty square
            if ((line & BIT0) == 0 && (line & BIT5) == 0)
                bitnum = 4;
            else if ((line & BIT1) == 0 && (line & BIT6) == 0)
                bitnum = 3;
            else if ((line & BIT2) == 0 && (line & BIT7) == 0)
                bitnum = 2;
            else if ((line & BIT3) == 0 && (line & BIT8) == 0)
                bitnum = 1;
            else if ((line & BIT4) == 0 && (line & BIT9) == 0)
                bitnum = 0;
			else{
				TRACE("%d, %d, %d\n", line, player, COMPUTER);
			//	assert(!"Error: could not find blank square");
				return false;
			}

            // Fill the a,b,c variables with the correct coordinates. 
            GetMoveCoord(a, b, c, Combinations, bitnum);
            TRACE("[Check4CriticalMove]: found move for Computer: %d,%d,%d\n", a, b, c);
            ret = true;
            break;
        }
    }

    return ret;
}


// 
// MiniMax search with Alpha Beta pruning
// 
int Engine::alphabeta(int &a, int &b, int &c, int player, USHORT ComboList[], int depth, int alpha, int beta)
{
    USHORT  tempboard[NUM_COMBOS];
    int     value;     // Return value of alphabeta calls

    ASSERT(player == PLAYER1 || player == PLAYER2);

//  TRACE("[alphabeta]: called with %d %d %d, p=%d, depth=%d, alpha=%d, beta=%d\n",
//        a, b, c, player, depth, alpha, beta);

    if (depth == DEPTH)
    {
        a = b = c = -1;
        
        if (Check4CriticalMove(a, b, c, player, ComboList))
        {
			TRACE("[alphabeta]: got critical move\n");
            return 100000;
        }
    }
    
    if (depth <= 0)
    {
        int result = Evaluate(ComboList);
//      TRACE("[alphabeta]: at depth %d, eval'd to %d\n", depth, result);
		return result;
    }
    

    // Run through the ComboList looking for legal moves
    for (int i = 0; i<NUM_HOLES_PER_LINE; i++) // Iterate across boards (z)
    { 
        for (int k = 0; k<NUM_HOLES_PER_LINE; k++)  // Iterate down rows (y)
        {
            for (int j = 0; j<NUM_HOLES_PER_LINE; j++) // Run down each row
            {
                if (LegalMove(i, k, j, player, ComboList, false))
                {
                    // Copy ComboList so we don't write over original
                    memcpy(tempboard, ComboList, NUM_COMBOS * sizeof(USHORT));

                    MakeMove(i, k, j, player, tempboard, false);

					//TRACE( "[alphabeta]: recursing on depth %d with (%d, %d, %d)\n", depth, a, b, c );
                  
                    // Call alphabeta with one less depth and the new board
                    value = alphabeta(a, b, c, !player, tempboard, (depth-1), alpha, beta);
					//TRACE( "[alphabeta]: recursion on depth %d returned %d\n", depth-1, value );
                    //TRACE( "[alphabeta]: alpha=%d beta=%d player=%d\n", alpha, beta, player);


					//if(depth==DEPTH) TRACE("[alphabetaT]: got %d for move %d %d %d\n", value,i,k,j);
					//if(depth==DEPTH) TRACE("[alphabetaT]: alpha=%d beta=%d p=%d\n", alpha,beta,player);

                    //if (player == COMPUTER)
                    if (player == PLAYER2)
                    { 
                        // We want to maximize for player2 (computer)
                        // Put the max of alpha and value into alpha
                        if (value > alpha)
                        {
                            // Found new bestMove:
                            alpha = value;  
                            
                            // Only set at the top level:
                            if (depth == DEPTH)
                            {
                                a = i; // Set best move variables to current position
                                b = k;
                                c = j;
								//TRACE("\n[alphabeta]: set new p2 move %d %d %d with eval %d\n",
								//		a,b,c,value);
                            }
                        }  
                        if (alpha >= beta)
                        {
                            return beta;
                        }
                    }
                    else
                    {
                        // Then we're player0 (human): 
                        // put the min of beta and value into beta
                        if (value < beta)
                        {
                            beta = value; // Found new bestMove:

                            // Only set if at the top level
                            if (depth == DEPTH)
                            {
                                a = i; // Set best move variables to current position
                                b = k;
                                c = j;
								//TRACE("\n[alphabeta]: set new p1 move %d %d %d with eval %d\n",
								//		a,b,c,value);
                            }
                        }
                        
                        if (alpha >= beta)
                        {
                            return alpha;
                        }
                    } // end of player ifs
                } // end of LegalMove if
				//else{
					//if(depth==DEPTH) TRACE("[alphabeta]: %d %d %d is not legal move (depth %d)\n\n\n", i, k ,j,depth);
				//}
            } // end of this row
        }
    }
  
    if (player == PLAYER2)
		return alpha;
	else // player == PLAYER1
		return beta;
}


//
// Makes the move x,y,z
// Returns false if move failed
//
bool Engine::makeMove(int player, int x, int y, int z, bool bFreeSpace)
{
	// This Should be called only for human player moves!! 
	// ie, if the computer is moving, getMove will automatically call MakeMove

    if (MakeMove(x, y, z, player, theboard, bFreeSpace) == 0)
    {
        printf("[MakeMove]: invalid move\n");
        return false;
    }
    else 
    {
        //moves++;
        if (!bFreeSpace)
            SaveMove(x, y, z);
        return true;
    }

    debugmapping();
}

//
// Have the computer choose a move and store it in the x,y,z parameters
//
void Engine::getMove(int player, int &x, int &y, int &z)
{
    int     a, b, c;
    USHORT  runboard[NUM_COMBOS];

    memcpy(runboard, theboard, NUM_COMBOS * sizeof(USHORT));

	TRACE("calling alphabeta with depth=%d, random depth=%d, player=%d\n", 
			getDepth(), RANDOM_DIFF,player);
    if (getDepth() > RANDOM_DIFF)
    { 
        a = -1;
        b = -1;
        c = -1;

        // This could take a while
        AfxGetApp()->BeginWaitCursor();

        alphabeta(a, b, c, player, runboard, DEPTH, INT_MIN, INT_MAX);
//      minimax(a, b, c, player, runboard, DEPTH);

        // Alpha starts at negative infinity (INT.MIN) and beta starts at positive inf.
        printf("[getMove]: alphabeta selected %d %d %d\n", a, b, c);

        AfxGetApp()->EndWaitCursor();
    }
    else 
    {
        // Otherwise pick a random move
        a = rand() % NUM_HOLES_PER_LINE;
        b = rand() % NUM_HOLES_PER_LINE;
        c = rand() % NUM_HOLES_PER_LINE;

        while (!LegalMove(a, b, c, player, runboard, false))
        {
            a = rand() % NUM_HOLES_PER_LINE;
            b = rand() % NUM_HOLES_PER_LINE;
            c = rand() % NUM_HOLES_PER_LINE;
        }
		TRACE("getMove: picked random move\n");
    }
    
    MakeMove(a, b, c, player, theboard, false);

    x = a;
    y = b;
    z = c;
//  moves++;
    SaveMove(x, y, z);

    TRACE("[getMove]: done\n");
    TRACE("Computer chooses move: %d %d %d\n\n\n\n ", a, b, c);

	char str[64];
	switch (player)
	{
	case PLAYER1:
        sprintf(str, "                         %C%d%c", 'V' + c, b + 1, 'a' + a);
		break;

	case PLAYER2:
        sprintf(str, "                                                  %C%d%c", 'V' + c, b + 1, 'a' + a);
		break;
	}

    m_ListBox->AddString(str);
}

/*
 * This function (minimax algorithm) NOT used, just was used for debugging/testing.
 *  (it picks the same choices as alphabeta, but is slower). 
 */
int Engine::minimax(int &a, int &b, int &c, int player, USHORT board[], int depth)
{
	int bestval;
	USHORT tempboard[NUM_COMBOS];

    if (player == PLAYER2)
		bestval = INT_MIN;
	else
		bestval = INT_MAX;

    if ( depth == 0)
    {
		return Evaluate(board);
	}

    // Run through the board looking for legal moves
    for (int i = 0; i < NUM_HOLES_PER_LINE; i++) // Iterate across boards (z)
    { 
        for (int k = 0; k < NUM_HOLES_PER_LINE; k++)  // Iterate down rows (y)
        {
            for (int j = 0; j < NUM_HOLES_PER_LINE; j++) // Run down each row
            {
                if (!LegalMove(i, k, j, player, board, false))
                {
					continue;
				}

                // Copy ComboList so we don't write over original
                memcpy(tempboard, board, NUM_COMBOS * sizeof(USHORT));

                MakeMove(i, k, j, player, tempboard, false);

                int value = minimax(a, b, c, !player, tempboard, depth-1); // recurse

                if (depth == DEPTH)
                    TRACE("[minimax2]: got val %d for move %d %d %d with bestval=%d\n",
						value, i,k,j, bestval);
                if (depth == DEPTH - 1)
                    TRACE("[minimax%d]: got val %d for move %d %d %d with bestval=%d\n",
						depth, value, i,k,j, bestval);

				// player2 maximizes, player1 minimizes.
                if (player == PLAYER2)
                {
                    if (  value > bestval)
                    {
						bestval  = value;
						a = i;  b = k;  c = j; 
					}
				}
                else
                { // player1, minimize.
                    if (value < bestval)
                    {
						bestval  = value;
						a = i;  b = k;  c = j; 
					}	
				}



			} // end for loops
		}
	}

	return bestval;
}

//
// Sets the depth for minimax search 
// (basically equivalent to setting a difficulty level)
//
void Engine::setDepth(DIFFICULTY_LEVELS d)
{
    TRACE("Depth set to %d\n", d);
    DEPTH = d;
}

//
// Returns the current depth
//
unsigned int Engine::getDepth()
{
    return DEPTH;
}

//
// A wrapper function for our won function (way above)
//
int Engine::hasWon()
{
    if (IsWon( theboard ))
    {
        //return moves;
		return moveIndex+1;
    }
    else if (moveIndex == NUM_HOLES_PER_GAME - 1) // Then it's a tie!
    {
        return -1;
    }
    else
    {
        return  0;
    }

	return 0;
}

//
// A wrapper function for our IsFourOfFive() function (way above)
//
bool Engine::hasFourOfFive()
{
    if (IsFourInaRow( theboard ))
	{
		return true;
	}

	return false;
}

void Engine::NewGame()
{
    // Simply re-initialize board: 
    for (int i = 0; i < NUM_COMBOS; i++)
    {
        theboard[i] = (USHORT)0; // This clears all positions
    }

//    moves = 0;
	moveIndex = 0;
}

//
// Returns the array index of the winning row
//
int Engine::getWonRow()
{
    for (int i = 0; i < NUM_COMBOS; i++)
    {
        USHORT row = theboard[i];

        if ((row & PLAYER1_MASK) == PLAYER1_MASK)
        {
            if ((row & PLAYER3_MASK) != PLAYER3_MASK)
            {
                return i;
            }
        }
        else if ((row & PLAYER2_MASK) == PLAYER2_MASK)
        {
            if ((row & PLAYER3_MASK) != PLAYER3_MASK)
            {
                return i;
            }
        }
    }

    // If we get here, then there were no winning rows
    return -1;
}
