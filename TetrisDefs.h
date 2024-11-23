/*
** ===========================================================================
**
** File: 
**     TetrisDefs.h
**
** Description: 
**     Tetris Defines
** 
** History: 
**
** when          who             what, where, why
** ----------    ------------    --------------------------------
** 2024-11-22    me              Created.
**
** ===========================================================================
*/

#ifndef _GAME_TETRIS_PROTS_H_
#define _GAME_TETRIS_PROTS_H_
/* 
**----------------------------------------------------------------------------
**  Includes
**---------------------------------------------------------------------------- 
*/

#ifdef __cplusplus
extern "C" {
#endif

/*
**----------------------------------------------------------------------------
**  Definitions
**----------------------------------------------------------------------------
*/

#define MAX_LEVEL        3  // Max number of levels supported
#define MAX_REM_LINES    4  // Max number of lines deleted once
#define MAX_FLASH_TIMES  3  // Max flash times when delete a full line
#define MAX_BLOCK_KIND   7  // Max number of types supported

#define INIT_TIME        900    // The initial time in easy level a block stops
#define TIME_SLICE       50     // The time decrement when level adds(ms)
#define FLASH_TIME       200    // Used to set timer
#define OVER_TIME        100    // Used to set timer when game is over
#define LEVEL_SCORE      500    // Max score in a level  

#define COLOR_BACKGROUND    gui_color(0,0,0)

#define TETRIS_VERSION 2  

/*
**----------------------------------------------------------------------------
**  Type Definitions
**----------------------------------------------------------------------------
*/

typedef S8 int8;
typedef S16 int16;
typedef S32 int32;
typedef U8 uint8;
typedef U16 uint16;
typedef U32 uint32;
typedef BOOL boolean;
typedef U8 byte;

typedef enum
{
    GAME_STATE_INIT = -3,
    GAME_STATE_SUSPENDED,
    GAME_STATE_SPLASH,
    
    GAME_STATE_MAIN_MENU = 0,
    GAME_STATE_LEVEL_SETTING,
    GAME_STATE_SOUND_SETTING,
    GAME_STATE_GRID_SETTING,
    GAME_STATE_HERO,
    
    GAME_STATE_HELP,
    GAME_STATE_RUNNING,
    GAME_STATE_PAUSED,
    GAME_STATE_NEXTLEVEL,
    GAME_STATE_LASTLEVEL,
    GAME_STATE_OVER,
    GAME_STATE_REPORT
} GameStateEnum;                   

typedef enum
{
    ACTION_NONE,
    ACTION_DOWN,
    ACTION_LEFT,
    ACTION_RIGHT,
    ACTION_ROTATE  
} ActionTypeEnum;

typedef enum
{
	MUSIC_TITLE,
    MUSIC_PLACE,
    MUSIC_SINLINE,
    MUSIC_MULLINES,
    MUSIC_NEXTSPEED,
    MUSIC_NEXTLEVEL,
    MUSIC_GAMEOVER
} MusicTypeEnum;

typedef struct _CPrefData
{
    uint8	gameLevel;
    BOOL	soundOn;
    BOOL	drawGridLines;
    U16		topScore[MAX_LEVEL];
} CPrefData;                             

typedef struct _CBlock
{
    S16 x;
    S16 y;
} CBlock;

typedef struct _CTetris
{
    U8		axis;       // whick block is the rotate axis
    U8		model;      // which model
    BOOL	pinned;  // if can not move downward, it is called pinned
    CBlock	block[4]; // the 4 blocks consisting a tetris
} CTetris;

typedef struct Grid
{
    BOOL dirty;
    uint8   tetrisModel;
} Grid;

typedef struct RGBVAL
{
	U8	r;
	U8	g;
	U8	b;
} RGBVAL;

typedef struct TetRect
{
   S16	x,y;
   S16	dx, dy;
} TetRect;

/*
**----------------------------------------------------------------------------
**  Variable Declarations
**----------------------------------------------------------------------------
*/

static CTetris TETRIS_MODEL[] =
{     
    //   * *
    // * *
    // the 1st block is the axis
    { 0, 0, FALSE, { {0, 0}, {1, 0}, {-1, 1}, {0, 1} } },
    
    // * * * * 
    // the 2nd block is the axis
    { 1, 1, FALSE, { {-1, 0}, {0, 0}, {1, 0}, {2, 0}} },
    
    // * *
    //   * *
    // the 2nd block is the axis
    { 1, 1, FALSE, { {0, 0}, {1, 0}, {1, 1}, {2, 1} } },
    
    // *
    // * * *
    // the 2nd block is the axis
    { 1, 1, FALSE, { {0, 0}, {0 ,1}, {1, 1}, {2, 1} } },

    //   *
    // * * *
    // the 3rd block is the axis
    { 2, 2, FALSE, { {0, 0}, {-1, 1}, {0, 1}, {1, 1} } },

    //     *
    // * * *
    // the 4th block is the axis
    { 3, 3, FALSE, { {1, 0}, {-1, 1}, {0, 1}, {1, 1} } },

    // * *
    // * *
    // no rotation operation
    { 1, 0, FALSE, { {0, 0}, {1, 0}, {0, 1}, {1, 1} } }
};          
    
static RGBVAL COLOR_PALETTE[] =
{
    {0xff, 0x66, 0x40},
    {0xff, 0x1b, 0xf1},
    {0xff, 0x01, 0x58},
    {0xa8, 0xff, 0x02},
    {0x69, 0x00, 0xda},
    {0xf8, 0x7a, 0x00},
    {0x00, 0x16, 0xd9}
};

/*
**----------------------------------------------------------------------------
**  Function(external use only) Declarations
**----------------------------------------------------------------------------
*/

extern void mmi_gx_tetris_enter_GFX(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif
#endif /* _GAME_TETRIS_PROTS_H_ */ 

