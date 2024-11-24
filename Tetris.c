/*
** ===========================================================================
**
** File: 
**     Tetris.c
**
** Description: 
**     Tetris code
** 
** History: 
**
** when          who             what, where, why
** ----------    ------------    --------------------------------
** 2024-11-24    me              Resume.
** 2024-11-24    me              Pseudo-resume.
** 2024-11-24    me              Next level screen.
** 2024-11-23    me              Properly randomize pieces.
** 2024-11-22    me              Created.
**
** ===========================================================================
*/

/*******************************************************************************
 *                Tetris Usage Description                    
 *******************************************************************************
 * (1) The Tetris contains three files.                         
 *    Tetris.c, TetrisDefs.h, TetrisProts.h                                          
 *                                                                
 * (2) Create NVRAM slot in NVRAMEnum.h to store game grade and current game level.
 *
 * (3) Write generate resouce related code in Game.res
 *
 * (4) Add entry point in Game.cT
 *
 * (5) Add __MMI_GAME_TETRIS__ in "MMI_features.h" if you wish to enable this game.
 *
 * (6) Remember to re-generate resource.
 *******************************************************************************/
/* 
**----------------------------------------------------------------------------
**  Includes
**----------------------------------------------------------------------------
*/

#include "mmi_features.h"

#include "GameDefs.h"
#include "GameInc.h"
#include "TetrisDefs.h"
#include "TetrisProts.h"
#include "TetrisResDef.h"
#include "mmi_rp_app_games_def.h"

#ifdef __MMI_GAME_MULTI_LANGUAGE_SUPPORT__      //added for multi-language
#include "GameProts.h"
#endif

#include "mmi_frm_nvram_gprot.h" 

#ifdef __MMI_GAME_TETRIS__

/*
**----------------------------------------------------------------------------
**  Definitions
**----------------------------------------------------------------------------
*/

#define DUMMY_STR_ID    (0)
#define DUMMY_IMG_ID    (0)
#define DUMMY_NVRAM_ID  (0)
# define SETAEERECT(prc,l,t,w,h)   (prc)->x=(S16)(l),(prc)->y=(S16)(t),(prc)->dx=(S16)(w),(prc)->dy=(S16)(h)
#define theRowIsDirty( row) ( me->gridRowMap[row] == me->gridRowMask)

#ifdef _WIN32
#define debug printf
#else
#define debug __noop
#endif

/*
**----------------------------------------------------------------------------
**  Type Definitions
**----------------------------------------------------------------------------
*/

/* game context */
typedef struct
{
    BOOL			is_gameover;
    BOOL			is_new_game;
    U16				timer_elapse;
    
    GameStateEnum	previousGameState;
    GameStateEnum	gameState;
    CPrefData		configData;

    U8				fontHeight; 
    U16				screenWidth;         
    U16				screenHeight;

    U16				xWhereToLaunchTetris;
    U16				yWhereToLaunchTetris;
    U16				xWhereToDrawTheNextFallingTetris;
    U16				yWhereToDrawTheNextFallingTetris;
    
    U8				gridSideLength;   // grid side length, in pixel
    U16				gridColumnNumber; // the playing zone grid column number
    U16				gridRowNumber;    // the playing zone grid row number
    
    U8				gameLevel;              // Game level
    U16				selectedMenuItem[5];
    U16				theTopmostRowOfThePinnedTetrises; // start from 0
    U16				gameScore;              // Game score
 
    U16				sleepTime;              // period between two moving downward
    U16				gameSpeed;              // Game speed
    
    BOOL			continueGame;           // Can the game be continued
    BOOL			drawGridLines;          // Draw the gridding or not
    BOOL			soundOn;                // Enable or disable sound in the play
    BOOL			moveDownwardAccelerated;
    
    TetRect			playingZone; 
    TetRect			gridToClear;
 
    U16				dirtyRowIndex[MAX_REM_LINES];
    U8				flashCounterWhenDeleteDirtyRows; 
    U8				dirtyRowNumber;
                                        
    CTetris			theFallingTetris;
    CTetris			theNextFallingTetris;
    Grid			**gridMatrix;
    // every unS16 map a row
    U16				*gridRowMap;
    U16				gridRowMask;
    
    U8				keyBeepVolumeSetting;
} gx_tetris_context_struct;

/*
**----------------------------------------------------------------------------
**  Global variables
**----------------------------------------------------------------------------
*/

/*
**----------------------------------------------------------------------------
**  Internal variables
**----------------------------------------------------------------------------
*/

gx_tetris_context_struct g_gx_tetris_context = 
{
    FALSE,  /* is_gameover */
    TRUE,   /* is_new_game */
    100,     /* timer_elapse */
    
    GAME_STATE_INIT,
    GAME_STATE_INIT,
    {0, FALSE, FALSE, {0,0,0}},
	
	0,
	0,
	0,

	0,
	0,
	0,
	0,

	0,
	0,
	0,

	0,
    {0,0,0,0,0},
	0,
	0,

	0,
	0,

	FALSE,
	FALSE,
	FALSE,
	FALSE,
	
    {0,0,0,0},
    {0,0,0,0},

    {0,0,0,0},
	0,
	0,
	
	{0, 0, FALSE, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}},        
	{0, 0, FALSE, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}},   
	NULL,

	NULL,
	0,

	0
};

BOOL mmi_gx_tetris_showonce_timer_stop = FALSE; //improvisation for a show-once timer
gx_tetris_context_struct* me;
WCHAR drawee_string[128];
BOOL tetris_ingame = FALSE; //do not free if not in gameover state
BOOL tetris_nextlevel = FALSE;
U8 dummy_gamelevel_val = 0;
/*
**----------------------------------------------------------------------------
**  Function(internal use only) Declarations
**----------------------------------------------------------------------------
*/

//==============================Init================================
static boolean  initGameData( void);
static void     freeGameDataMemory( void);
static void     initParametersAccordingToScreenSize( void);
//============================Processor=============================

//static boolean  Tetris_HandleEvent( CTetris *ptetris, AEEEvent eCode, uint16 wParam, uint32 dwParam);
static boolean  processKey( uint16 keyCode);
static boolean  processCommand( uint16 commandId);

static void     setGameState(GameStateEnum gameState);
static void     gotoNextLevel( boolean bIsLastLevel);

static void     startOneNewGame( void);
static void     launchOneTetris( void);

static uint8    generateRandomTetrisModel( void);
static void     calculateAbsoluteCoordinates( CTetris* tetris, byte model, int16 xOrigin, int16 yOrigin);
static void     drawGameScoreAndGameSpeed( void);
static void     clearHerosScore( void);

static boolean  moveDownwardsIf( void);
static void     moveLeftwardsIf( void);
static void     moveRightwardsIf( void);
static void     rotateIf( void);
static boolean  pinTheFallingTetrisIf( void);
static boolean  deleteDirtyRowsIf( void);
static boolean  canDoAction( CTetris* tetris, ActionTypeEnum action);

static boolean  theGridIsDirtyOrInvalid( int16 x, int16 y);
static boolean  theGridIsDirty( int16 x, int16 y);

static void     saveGameDataAndDisplayGameScore( void);
static void     saveSettingData( void);
static void     gameOver( void);

static boolean  pauseOrResumeGame( void);

//-------------------- UI related method declaration
static void     redrawTheScreen( void);

static void     resetControlsAndClearScreen( boolean clearScreen);
//static void     addMenuItem(IMenuCtl * pMenu, uint16 ItemID, uint16 ImageID);
static void     drawPopWindowFrameBorder( uint16 titleID, TetRect * fRect);

static void     draw3DText( int16 x, int16 y, UI_string_type text);

static void     displaySplashScreen( void);
static void     displayMainMenu( void);
static void     displayLevelSettingScreen( void);
static void     displaySoundSettingScreen( void);
static void     displayGridSettingScreen( void);
static void     displayHerosScreen( void);
static void     displayHelpScreen( void);
static void     displayGameScoreScreen( uint16 strID, uint32 Score, boolean NewRecord);
static void     clearPlayingZoneAfterGameOver( void);

static void     drawPlayingZoneBorder( void);
static void     drawGridLines( void);
static void     promptTheGameIsPausedNow( void);
static void     drawTetris( CTetris *tetris, boolean bOverDraw);
static void     drawTheFallingTetris( void);
static void     clearTheFallingTetris( void);
static void     drawTheNextFallingTetris( void);
static void     clearTheNextFallingTetris( void);

static void     refreshPlayingZoneAfterDeleteDirtyRows( void);
static void     flashWhenDeleteDirtyRows( void);
static void     drawTheFinalTetrisWhichFillupThePlayingZone( int16 YPos);
static boolean  updateGameScoreAndGoToNextLevelIf( void);
static void		getSettingData(void);
static boolean  playMusic( MusicTypeEnum type);

/* Game framework related functions */
S16 mmi_gx_tetris_calc_best_grade(S16 old_grade, S16 new_grade);      /* descide which is best grade */
void mmi_gx_tetris_enter_game(void);      /* entry function of the game */
void mmi_gx_tetris_exit_game(void);       /* exit function - usually will stop timer and release buffer */
void mmi_gx_tetris_draw_gameover(void);   /* draw gameover screen */
void mmi_gx_tetris_draw_gamescore(WCHAR* str);
/* Game play functions */
void mmi_gx_tetris_framemove(void);
void mmi_gx_tetris_render(void);
void mmi_gx_tetris_gameover(void);
void mmi_gx_tetris_cyclic_timer(void);
void mmi_gx_tetris_init_game(void);   /* draw gameover screen */

//=============================Init================================
static boolean initGameDataMemory(void)
{
    int i           = 0;
	
	me = &g_gx_tetris_context;

    g_gx_tetris_context.screenWidth  = UI_device_width;
    g_gx_tetris_context.screenHeight =  UI_device_height;   // The width and height of the device.
    debug( ";initApp, screen, cx = %d, cy = %d", UI_device_width, UI_device_height);

    g_gx_tetris_context.fontHeight = gui_get_character_height();
    debug( ";initApp, font height = %d", me->fontHeight);

    // calculate screen parameters according to the device infomation
    initParametersAccordingToScreenSize();

    me->gridRowMap = get_ctrl_buffer( me->gridRowNumber * sizeof( uint16));
    if( me->gridRowMap == NULL)
    {
        debug( ";creating gridRowMap failed.");
        return FALSE;
    }
    me->gridRowMask = 0;
    for( i = 0; i < me->gridColumnNumber; i ++)
    {
        me->gridRowMask |= 1 << i;
    }
    for( i = 0; i < me->gridRowNumber; i ++)
    {
        me->gridRowMap[i] = 0;
    }
    debug( ";initApp, rows = %d, gridRowMask = 0x%x", me->gridRowNumber, me->gridRowMask);

    me->gridMatrix = get_ctrl_buffer( me->gridRowNumber * sizeof( Grid*));
    if( me->gridMatrix == NULL)
    {
        debug( ";creating gridMatrix row dimension failed.");
        return FALSE;
    }

    for( i = 0; i < me->gridRowNumber; i ++)
    {

        me->gridMatrix[i] = get_ctrl_buffer( me->gridColumnNumber * sizeof(Grid));
        if( me->gridMatrix[i] == NULL)
        {
            debug( "allocating memory for gridMatrix column %d dimension failed.", i);
            return FALSE;
        }
    }
	return TRUE;
}

static boolean initGameData(void)
{
    int i           = 0;
	int j 			= 0;

    // load configuration data
    getSettingData();

    me->sleepTime       = INIT_TIME;
    me->gameSpeed       = 0;
    me->gameLevel       = me->configData.gameLevel;
    me->soundOn         = me->configData.soundOn;
    me->drawGridLines   = me->configData.drawGridLines;

    me->previousGameState   = GAME_STATE_INIT;
    me->gameState           = GAME_STATE_INIT;

	
    if( me->gameLevel == 0 || me->gameLevel > MAX_LEVEL)
    {
        me->gameLevel = 1;
    }

    me->continueGame                = FALSE;
    me->flashCounterWhenDeleteDirtyRows = 0;
    me->dirtyRowNumber          = 0;
    me->gameScore                   = 0;

    me->theTopmostRowOfThePinnedTetrises = me->gridRowNumber - 1;
    me->sleepTime   = ( MAX_LEVEL - me->gameLevel + 1) * INIT_TIME / MAX_LEVEL;
    me->gameSpeed   = ( INIT_TIME - me->sleepTime) / TIME_SLICE;

    me->gridToClear.x  = me->playingZone.x;
    me->gridToClear.y  = me->playingZone.y + me->playingZone.dy;
    me->gridToClear.dx = me->gridSideLength - 1;
    me->gridToClear.dy = me->gridSideLength - 1;

    for(i = 0; i < MAX_REM_LINES; i++)
    {
        me->dirtyRowIndex[i]  = 0;
    }

    for( i = 0; i < me->gridRowNumber; i++)
    {
        for( j = 0; j < me->gridColumnNumber; j++)
        {
            me->gridMatrix[i][j].dirty      = FALSE;
            me->gridMatrix[i][j].tetrisModel = MAX_BLOCK_KIND;
        }
        me->gridRowMap[i] = 0;
    }

	return TRUE;
}

static void freeGameDataMemory(  void)
{
    int i;

    for( i = 0; i < me->gridRowNumber; i ++)
    {
        free_ctrl_buffer( me->gridMatrix[i]);
    }
    free_ctrl_buffer( me->gridMatrix);
    free_ctrl_buffer( me->gridRowMap);
	debug("data free");
	me = NULL;
}

static void initParametersAccordingToScreenSize( void)
{

    uint16 marginHorizontal = 0;
    uint16 marginVertical   = 0;

    // the playing zone has a fixed column number
    me->gridColumnNumber    = 12;

    // determine grid size according to screen size,
    // there are two borders, totally occupy 4 pixels
    // and the next tetris occupies 4 grids
    me->gridSideLength = ( me->screenWidth - 4) / ( me->gridColumnNumber + 4);
    debug( ";init, grid, w / 16, %d", me->gridSideLength);
    if( me->gridSideLength > 4)
    {

        if( ( (me->screenHeight - 4) / me->gridSideLength) < 16)
        {

            me->gridSideLength = ( me->screenHeight - 4) / 16;
            me->gridSideLength = me->gridSideLength < 4 ? 4: me->gridSideLength;
            debug( ";init, grid, rows < 16, %d", me->gridSideLength);
        }
    }

    // grid side is determined, now we determine grid row number
    // it shall not more than 21
    me->gridRowNumber = (me->screenHeight - 4) / me->gridSideLength;
    me->gridRowNumber = me->gridRowNumber > 21 ? 21 : me->gridRowNumber;
    debug( ";grid, rowNumber = %d, columnNumber = %d", me->gridRowNumber, me->gridColumnNumber);

    // calculate the margin
    marginHorizontal    = ( me->screenWidth - 4 - (me->gridSideLength * 17)) / 3;
    marginVertical      = ( me->screenHeight - 4 -
            me->gridRowNumber * me->gridSideLength) >> 1;

    SETAEERECT( &me->playingZone,
            marginHorizontal + 2,
            marginVertical + 2,
            me->gridSideLength * 12,
            me->gridSideLength * me->gridRowNumber
            );

    me->xWhereToDrawTheNextFallingTetris = me->playingZone.x + me->playingZone.dx +
                            marginHorizontal + ( me->gridSideLength << 1);
    me->yWhereToDrawTheNextFallingTetris = me->gridSideLength;

    me->yWhereToLaunchTetris = me->playingZone.y;
    me->xWhereToLaunchTetris = me->playingZone.x +
        (  (me->playingZone.dx - me->gridSideLength * (me->gridColumnNumber % 2)) >> 1);
} // initParametersAccordingToScreenSize

static void killTimer()
{
	gui_cancel_timer(moveDownwardsIf);
	gui_cancel_timer(drawGameScoreAndGameSpeed);
	gui_cancel_timer(flashWhenDeleteDirtyRows);
	gui_cancel_timer(clearPlayingZoneAfterGameOver);
	gui_cancel_timer(gameOver);
}

static void setGameState(GameStateEnum gameState)
{
	debug("state=====%d", gameState);
    g_gx_tetris_context.previousGameState   = g_gx_tetris_context.gameState;
    g_gx_tetris_context.gameState           = gameState;
}

static void startOneNewGame(  void)
{

    resetControlsAndClearScreen(  TRUE);
    initGameData();
    drawPlayingZoneBorder();
    drawGameScoreAndGameSpeed();

    setGameState(GAME_STATE_RUNNING);

    me->theFallingTetris.pinned = FALSE;
    calculateAbsoluteCoordinates( &me->theNextFallingTetris,
            generateRandomTetrisModel(),
            me->xWhereToDrawTheNextFallingTetris,
            me->yWhereToDrawTheNextFallingTetris);
    launchOneTetris();
	tetris_ingame = TRUE; //bypass value so as not to free data when next level screen shows up or resume game...
} // startOneNewGame

static void launchOneTetris( void)
{

    uint8  i;

    if( pinTheFallingTetrisIf())
    {
        if( deleteDirtyRowsIf() && !updateGameScoreAndGoToNextLevelIf())
        {
            return;
        }
    }

    calculateAbsoluteCoordinates( &me->theFallingTetris,
            me->theNextFallingTetris.model,
            me->xWhereToLaunchTetris,
            me->yWhereToLaunchTetris
        );
    for( i = 0; i < 4; i ++)
    {
        if(theGridIsDirtyOrInvalid(me->theFallingTetris.block[i].x, me->theFallingTetris.block[i].y))
        {
            drawTheFinalTetrisWhichFillupThePlayingZone(me->theFallingTetris.block[i].y);

            playMusic(MUSIC_GAMEOVER);
            gui_start_timer(OVER_TIME, gameOver);
            return;
        }
    }
    drawTheFallingTetris();
    me->theFallingTetris.pinned = FALSE;

    clearTheNextFallingTetris();
    calculateAbsoluteCoordinates( &me->theNextFallingTetris,
            generateRandomTetrisModel(),
            me->xWhereToDrawTheNextFallingTetris,
            me->yWhereToDrawTheNextFallingTetris);
    drawTheNextFallingTetris();

    me->moveDownwardAccelerated = FALSE;
	gui_start_timer(me->sleepTime, moveDownwardsIf);
} // launchOneTetris

static uint8 generateRandomTetrisModel( void)
{
    return rand() % MAX_BLOCK_KIND;
}

static void calculateAbsoluteCoordinates(  CTetris* tetris,
        byte model,
        int16 xOrigin,
        int16 yOrigin
)
{

    int i = 0;

    tetris->model = model;
    for( i = 0; i < 4; i ++)
    {
        tetris->block[i].x = xOrigin + TETRIS_MODEL[model].block[i].x * me->gridSideLength;
        tetris->block[i].y = yOrigin + TETRIS_MODEL[model].block[i].y * me->gridSideLength;
    }
}

static boolean canDoAction( CTetris* tetris, ActionTypeEnum action)
{

    byte    i = 0;
    int16   x = 0;
    int16   y = 0;
    boolean returnValue = FALSE;

    switch( action)
    {

        case ACTION_DOWN:
        {

            for( i = 0; i < 4; i ++)
            {
                x = tetris->block[i].x;
                y = tetris->block[i].y + me->gridSideLength;

                if( theGridIsDirtyOrInvalid( x, y) ||
                    ( y + me->gridSideLength > me->playingZone.y + me->playingZone.dy)
                )
                {
                    returnValue = FALSE;
                    goto _canDoAction_return_;
                }

                tetris->block[i].y = y;
            }
            returnValue = TRUE;
            goto _canDoAction_return_;
        }

        case ACTION_LEFT:
        {

            for( i = 0; i < 4; i ++)
            {
                x = tetris->block[i].x - me->gridSideLength;
                y = tetris->block[i].y;

                if( theGridIsDirtyOrInvalid( x, y) || ( x < me->playingZone.x))
                {
                    returnValue = FALSE;
                    goto _canDoAction_return_;
                }

                tetris->block[i].x = x;
            }
            returnValue = TRUE;
            goto _canDoAction_return_;
        }

        case ACTION_RIGHT:
        {

            for( i = 0; i < 4; i ++)
            {
                x = tetris->block[i].x + me->gridSideLength;
                y = tetris->block[i].y;

                if( theGridIsDirtyOrInvalid( x, y) ||
                    ( x + me->gridSideLength > me->playingZone.x + me->playingZone.dx)
                )
                {
                    returnValue = FALSE;
                    goto _canDoAction_return_;
                }

                tetris->block[i].x = x;
            }
            returnValue = TRUE;
            goto _canDoAction_return_;
        }

        case ACTION_ROTATE:
        {

            int16 xAxis = tetris->block[tetris->axis].x;
            int16 yAxis = tetris->block[tetris->axis].y;
            int   xMin  =  3000;
            int   xMax  = -3000;
            int   yMin  =  3000;
            int   yMax  = -3000;

            for( i = 0; i < 4; i ++)
            {
                x = xAxis + yAxis - tetris->block[i].y;
                y = yAxis + tetris->block[i].x - xAxis;

                if( theGridIsDirty( x, y) ||
                    y + me->gridSideLength > me->playingZone.y + me->playingZone.dy
                )
                {

                    debug( ";rotate, grid[%d][%d] is dirty",
                            (x - me->playingZone.x) / me->gridSideLength,
                            (y - me->playingZone.y) / me->gridSideLength
                         );
                    returnValue = FALSE;
                    goto _canDoAction_return_;
                }

                tetris->block[i].x = x;
                tetris->block[i].y = y;

                if( xMax < x)
                {
                    xMax = x;
                }
                if( yMax < y)
                {
                    yMax = y;
                }
                if( xMin > x)
                {
                    xMin = x;
                }
                if( yMin > y)
                {
                    yMin = y;
                }
            }
            debug( ";xMin: %d, xMax: %d, yMin: %d, yMax: %d", xMin, xMax, yMin, yMax);

            // left blocked
            if( xMin < me->playingZone.x)
            {
                int distance = me->playingZone.x - xMin;

                for( i = 0; i < 4; i ++)
                {
                    tetris->block[i].x += distance;

                    if( theGridIsDirty( tetris->block[i].x, tetris->block[i].y) ||
                        tetris->block[i].y + me->gridSideLength > me->playingZone.y + me->playingZone.dy
                    )
                    {
                        debug( ";rotate, moveRight %d, grid[%d][%d] is dirty",
                                distance,
                                (tetris->block[i].x - me->playingZone.x) / me->gridSideLength,
                                (tetris->block[i].y - me->playingZone.y) / me->gridSideLength
                             );

                        returnValue = FALSE;
                        goto _canDoAction_return_;
                    }
                }
            }
            // right blocked
            if( xMax + me->gridSideLength > me->playingZone.x + me->playingZone.dx)
            {
                int distance = ( xMax + me->gridSideLength) - ( me->playingZone.x + me->playingZone.dx);

                for( i = 0; i < 4; i ++)
                {
                    tetris->block[i].x -= distance;

                    if( theGridIsDirty( tetris->block[i].x, tetris->block[i].y) ||
                        tetris->block[i].y + me->gridSideLength > me->playingZone.y + me->playingZone.dy
                    )
                    {
                        debug( ";rotate, moveLeft %d, grid[%d][%d] is dirty",
                                distance,
                                (tetris->block[i].x - me->playingZone.x) / me->gridSideLength,
                                (tetris->block[i].y - me->playingZone.y) / me->gridSideLength
                             );

                        returnValue = FALSE;
                        goto _canDoAction_return_;
                    }
                }
            }
            // top blocked
            if( yMin < me->playingZone.y)
            {
                int distance = me->playingZone.y - yMin;

                for( i = 0; i < 4; i ++)
                {
                    tetris->block[i].y += distance;

                    if( theGridIsDirty( tetris->block[i].x, tetris->block[i].y) ||
                        tetris->block[i].y + me->gridSideLength > me->playingZone.y + me->playingZone.dy
                    )
                    {
                        debug( ";rotate, moveDown %d, grid[%d][%d] is dirty",
                                distance,
                                (tetris->block[i].x - me->playingZone.x) / me->gridSideLength,
                                (tetris->block[i].y - me->playingZone.y) / me->gridSideLength
                             );

                        returnValue = FALSE;
                        goto _canDoAction_return_;
                    }
                }
            }

            returnValue = TRUE;
            goto _canDoAction_return_;
        }

    } // switch( action)

_canDoAction_return_:

    return returnValue;
} // canDoAction

static boolean moveDownwardsIf( void)
{

    boolean returnValue = TRUE;
    CTetris tetris;

    memcpy( &tetris, &me->theFallingTetris, sizeof( tetris));
    clearTheFallingTetris();

    if( me->moveDownwardAccelerated)
    {

        while( canDoAction( &tetris, ACTION_DOWN))
        {
            memcpy( &me->theFallingTetris, &tetris, sizeof( tetris));
        }
        me->theFallingTetris.pinned = TRUE;
    }
    else
    {

        if( canDoAction( &tetris, ACTION_DOWN))
        {
            memcpy( &me->theFallingTetris, &tetris, sizeof( tetris));
        }
        else
        {
            me->theFallingTetris.pinned = TRUE;
        }
    }
    drawTheFallingTetris();

    if(me->theFallingTetris.pinned  || !canDoAction(&tetris, ACTION_DOWN) )
    {
        me->theFallingTetris.pinned = TRUE;

        launchOneTetris();
        returnValue = FALSE;
        goto _moveDownwardIf_return_;
    }

    gui_start_timer(me->sleepTime, moveDownwardsIf);

_moveDownwardIf_return_:
    return returnValue;
} // moveDownwardsIf

static void doLeftRightRotateAction( ActionTypeEnum action)
{

    if( action == ACTION_LEFT || action == ACTION_RIGHT || action == ACTION_ROTATE)
    {
        CTetris tetris;
        memcpy( &tetris, &me->theFallingTetris, sizeof( tetris));
        if( canDoAction( &tetris, action))
        {
            clearTheFallingTetris();
            memcpy( &me->theFallingTetris, &tetris, sizeof( tetris));
            drawTheFallingTetris();
        }
    }
}

static void moveLeftwardsIf( void)
{
    doLeftRightRotateAction( ACTION_LEFT);
}

static void moveRightwardsIf( void)
{
    doLeftRightRotateAction( ACTION_RIGHT);
}

static void rotateIf( void)
{
    doLeftRightRotateAction( ACTION_ROTATE);
}

static boolean pinTheFallingTetrisIf( void)
{

    if( me->theFallingTetris.pinned)
    {
        int i   = 0;
        int row = 0;
        int col = 0;

        for( i = 0; i < 4; i ++)
        {
            row = ( me->theFallingTetris.block[i].y - me->playingZone.y) / me->gridSideLength;
            col = ( me->theFallingTetris.block[i].x - me->playingZone.x) / me->gridSideLength;

            me->gridMatrix[row][col].dirty       = TRUE;
            me->gridMatrix[row][col].tetrisModel = me->theFallingTetris.model;
            me->gridRowMap[row] |= 1 << ( me->gridColumnNumber - 1 - col);

            if( row < me->theTopmostRowOfThePinnedTetrises)
            {
                me->theTopmostRowOfThePinnedTetrises = row;
            }
        }
    } // if( pinned)

    return me->theFallingTetris.pinned;
} //pinTheFallingTetrisIf

static boolean theGridIsDirtyOrInvalid( int16 x, int16 y)
{
    int16 row       = (y - me->playingZone.y) / me->gridSideLength;
    int16 column    = (x - me->playingZone.x) / me->gridSideLength;

    if( row < 0     || row >= me->gridRowNumber         ||
        column < 0  || column >= me->gridColumnNumber
    )
    {
        return TRUE;
    }
    else
    {
        return me->gridMatrix[row][column].dirty;
    }
}

static boolean theGridIsDirty( int16 x, int16 y)
{
    int16 row       = (y - me->playingZone.y) / me->gridSideLength;
    int16 column    = (x - me->playingZone.x) / me->gridSideLength;

    if( row < 0     || row >= me->gridRowNumber         ||
        column < 0  || column >= me->gridColumnNumber
    )
    {
        return FALSE;
    }
    else
    {
        return me->gridMatrix[row][column].dirty;
    }
}

static int getTheBottomMostYCoordinateOfTheFallingTetris( void)
{

    int i = 0;
    int y = 0;

    for( i = 0; i < 4; i ++)
    {
        if( y < me->theFallingTetris.block[i].y)
        {
            y = me->theFallingTetris.block[i].y;
        }
    }

    return y;
}

static boolean deleteDirtyRowsIf(  void)
{

    int i = 0;
    int j = 0;
    int k = 0;

    int row                             = 0;
    int yOfTheFallingTetrisBottomMost   = 0;

    TetRect rect;
    rect.dx = me->gridSideLength - 1;
    rect.dy = rect.dx;

    yOfTheFallingTetrisBottomMost = getTheBottomMostYCoordinateOfTheFallingTetris();
    row = ( yOfTheFallingTetrisBottomMost - me->playingZone.y) / me->gridSideLength;
    for( i = 0; row >= 0 && i < 4; i ++)
    {

        if( !theRowIsDirty( row))
        {
            row --;
            continue;
        }

        me->dirtyRowIndex[me->dirtyRowNumber] = row - me->dirtyRowNumber;
        me->dirtyRowNumber += 1;

        for(j = row; j >= (me->theTopmostRowOfThePinnedTetrises - 1); j--)
        {
            for( k = 0; k < me->gridColumnNumber; k ++)
            {
                me->gridMatrix[j][k].dirty       = me->gridMatrix[j-1][k].dirty;
                me->gridMatrix[j][k].tetrisModel = me->gridMatrix[j-1][k].tetrisModel;
                me->gridRowMap[j]                = me->gridRowMap[j-1];
            }
        }

    } // for( i = 0; row >= 0 && i < 4; i ++)

    if( me->dirtyRowNumber != 0)
    {

        if( me->dirtyRowNumber == 1)
        {
            playMusic( MUSIC_SINLINE);
        }
        else
        {
            playMusic( MUSIC_MULLINES);
        }
        gui_cancel_timer(moveDownwardsIf);
        drawTheFallingTetris();
        flashWhenDeleteDirtyRows();// Produce flash effect where there are full lines.
    }
    else
    {
        playMusic( MUSIC_PLACE);
    }

    return me->dirtyRowNumber > 0;
} //deleteDirtyRowsIf

static boolean pauseOrResumeGame( void)
{

    // to pause game
    if(me->gameState == GAME_STATE_RUNNING)
    {

        if( me->flashCounterWhenDeleteDirtyRows != 0)
        {

            refreshPlayingZoneAfterDeleteDirtyRows();

            drawTheFallingTetris();
            me->flashCounterWhenDeleteDirtyRows = 0;
            me->dirtyRowNumber = 0;
        }

        killTimer();
        setGameState(GAME_STATE_PAUSED);

        promptTheGameIsPausedNow();
        return TRUE;
    }
    else if(me->gameState == GAME_STATE_PAUSED)
    {

        setGameState(GAME_STATE_RUNNING);

        redrawTheScreen();
        return TRUE;
    }

    return FALSE;
} // pauseOrResume

//---------------------------- UI related method definition

void Tetris_2Key(void)
{
 	if(me->gameState == GAME_STATE_RUNNING && me->theFallingTetris.model != 6 &&
       !me->theFallingTetris.pinned && me->flashCounterWhenDeleteDirtyRows == 0)
    {
		rotateIf();
    }
}

void Tetris_4Key(void)
{
 	if(me->gameState == GAME_STATE_RUNNING&&
       !me->theFallingTetris.pinned && me->flashCounterWhenDeleteDirtyRows == 0)
    {
		moveLeftwardsIf();
    }
}

void Tetris_5Key(void)
{
	 pauseOrResumeGame();
}

void Tetris_6Key(void)
{
 	if(me->gameState == GAME_STATE_RUNNING &&
       !me->theFallingTetris.pinned && me->flashCounterWhenDeleteDirtyRows == 0)
    {
		moveRightwardsIf();
    }
}

void Tetris_8Key(void)
{
 	if(me->gameState == GAME_STATE_RUNNING &&
       !me->theFallingTetris.pinned && me->flashCounterWhenDeleteDirtyRows == 0)
    {
		gui_cancel_timer(moveDownwardsIf);
        me->moveDownwardAccelerated = TRUE;
        moveDownwardsIf();
    }
}

void Tetris_KeyboardKey(S32 vkey_code, S32 key_state)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
#if(MMI_BUILD_TYPE == BUILD_TYPE_X86WIN32)
    if (key_state)
    {
        switch (vkey_code)  /* key down */
        {
            case 32:
                Tetris_5Key();  /* left */
                break;
            case 37:
                Tetris_4Key();  /* left */
                break;
            case 38:
                Tetris_2Key();    /* up */
                break;
            case 39:
                Tetris_6Key(); /* right */
                break;
            case 40:
                Tetris_8Key();  /* down */
                break;
			case
                //     case 1:         tetris_handle_key_exit();
                //                             break;
        }
    }
    else    /* key up */
    {
    }
#else /* (MMI_BUILD_TYPE == BUILD_TYPE_X86WIN32) */ 
    UI_UNUSED_PARAMETER(vkey_code);
    UI_UNUSED_PARAMETER(key_state);
#endif /* (MMI_BUILD_TYPE == BUILD_TYPE_X86WIN32) */ 

}

static void displaySplashScreen(  void)
{
	PU8				pImage = NULL;
	gx_tetris_context_struct *me = &g_gx_tetris_context;
	
	pImage = (PU8)get_image(IMG_ID_GX_TETRIS_SPLASH);

	if (pImage != NULL)
	{
		U32	iImgWdt,iImgHgt;
		U32 iStrWdt,iStrHgt;
		PU8 pTitleString;
		TetRect rect;
		gui_measure_image(pImage,&iImgWdt,&iImgHgt);
		gui_set_clip(0, 0, UI_device_width - 1, UI_device_height - 1);
		gui_fill_rectangle(0, 0, UI_device_width - 1, UI_device_height - 1, gui_color(0x0, 0xcf, 0xff));
		gui_show_transparent_image((UI_device_width - iImgWdt)  >> 1,
                       				UI_device_width - iImgHgt, pImage,0);
		pTitleString = GetString(STR_GX_TETRIS_GAME_NAME);
		gui_measure_string((UI_string_type)pTitleString, &iStrWdt, &iStrHgt);
		rect.dx  = iStrWdt;
        rect.x   = ( UI_device_width - rect.dx) >> 1;
        rect.y   = ( UI_device_width - iImgHgt - iStrHgt) >> 1;
        draw3DText( rect.x, rect.y, pTitleString);
		gui_BLT_double_buffer(0, 0, UI_device_width - 1, UI_device_height - 1);
		
		GFX_PLAY_AUDIO_MIDI_TITLE(tetris_sfx_title, tetris_sfx_title_size, DEVICE_AUDIO_PLAY_ONCE);
	}
}

static void drawText3( S16 x, S16 y, UI_string_type text, RGBVAL color2draw, U32 flags)
{
	color saved_color;
	S32	str_wdt;
	S32 str_hgt;

	saved_color = gui_get_text_color();
	gui_set_text_color(gui_color(color2draw.r, color2draw.g, color2draw.b));
	gui_measure_string(text, &str_wdt, &str_hgt);

    if (r2lMMIFlag)
    {
        gui_move_text_cursor(x + str_wdt, y);
    }
    else
    {
        gui_move_text_cursor(x, y);
    }
	
	gui_print_text(text);
	gui_set_text_color(saved_color);
}

static void draw3DText( S16 x, S16 y, UI_string_type text)
{
	RGBVAL color1;
	RGBVAL color2;
	RGBVAL color3;

	color1.r = color1.g = color1.b = 0xff;
	color2.r = color2.g = color2.b = 0x00;
	color3.r = 0xff;
	color3.g = 0xcc;
	color3.b = 0x00;
	//drawText3( x, y - 1, text, color1, 0);
	//drawText3( x - 1, y + 1, text, color2, 0);
    //drawText3( x + 1, y + 1, text, color2, 0);
    drawText3( x, y, text, color3, 0);
}


static void drawPlayingZoneBorder( void)
{

    TetRect rect;

    // to clear the screen with background color
    rect.x  = 0;
    rect.y  = 0;
    rect.dx = me->screenWidth;
    rect.dy = me->screenHeight;
	gui_fill_rectangle(rect.x,rect.y,rect.x+rect.dx-1,rect.y+rect.dy-1,COLOR_BACKGROUND);

    // to draw the border of the playing zone
    // 1. draw inner border
    rect.x  =  me->playingZone.x - 1;
    rect.y  =  me->playingZone.y - 1;
    rect.dx = me->playingZone.dx + 2;
    rect.dy = me->playingZone.dy + 2;
	gui_fill_rectangle(rect.x,rect.y,rect.x+rect.dx-1,rect.y+rect.dy-1,gui_color(20,90,141));
    // 2. draw outer border
    rect.x  -=  1;
    rect.y  -=  1;
    rect.dx += 2;
    rect.dy += 2;
	gui_fill_rectangle(rect.x,rect.y,rect.x+rect.dx,rect.y+rect.dy-1,COLOR_BACKGROUND);

    // to draw grid lines
    if( me->drawGridLines)
    {
        drawGridLines();
    }

}

static void drawGridLines( void)
{

    int     i = 0;
    int     x,y,ey;

    x = me->playingZone.x - 1;
    y = me->playingZone.y - 1;
    ey = me->playingZone.y + me->playingZone.dy - 1;
    for(i = 1; i < me->gridColumnNumber; i ++)
    {
        x += me->gridSideLength;
		gui_fill_rectangle(x-1,y,x,ey,gui_color(0,0,0x8b));
    }
} // drawGridLines

static void promptTheGameIsPausedNow( void)
{

    int16   x       = 0;
    int16   y       = 0;
    PU8		imagePaused = NULL;
	U32	iImgWdt,iImgHgt;
	
	imagePaused = (PU8)get_image(IMG_ID_GX_TETRIS_GAMEPAUSE_EN);
	if( imagePaused == NULL)
    {
        return;
    }
	gui_measure_image(imagePaused,&iImgWdt,&iImgHgt);
	
    x = ( ( me->playingZone.dx - iImgWdt) >> 1) + me->playingZone.x;
    y = ( ( me->playingZone.dy - iImgHgt) >> 1) + me->playingZone.y;
	gui_show_transparent_image(x,y,imagePaused,0);
	
    gui_BLT_double_buffer(0, 0, UI_device_width - 1, UI_device_height - 1);
} // promptTheGameIsPausedNow

static void drawGameScoreAndGameSpeed( void)
{

    uint16  x           = 0;
    uint16  y           = 0;
    uint16  digitWidth  = 0;
    int8    i           = 0;
    uint16  power       = 0;
	PU8		imageScore = NULL;
	PU8		imageSpeed = NULL;
	PU8		imageDigit = NULL;
	U32	iImgWdt,iImgHgt;
	
	imageScore = (PU8)get_image(IMG_ID_GX_TETRIS_GAMESCORE_EN);
	if( imageScore == NULL)
    {
        return;
    }
	imageSpeed = (PU8)get_image(IMG_ID_GX_TETRIS_GAMESPEED_EN);
	if( imageSpeed == NULL)
    {
        return;
    }
	
    gui_measure_image(imageScore,&iImgWdt,&iImgHgt);
    x = me->screenWidth - iImgWdt;
    y = me->yWhereToDrawTheNextFallingTetris + 4 * me->gridSideLength;
	gui_show_transparent_image(x,y,imageScore,0);
	y += iImgHgt + 2;
	
    x = me->screenWidth;
    for( i = 0, power = 1; i < 4; i ++, power *= 10)
    {
		imageDigit = (PU8)get_image(IMG_ID_GX_TETRIS_DIGIT_0+( me->gameScore / power % 10));
		gui_measure_image(imageDigit,&iImgWdt,&iImgHgt);
        x -= iImgWdt + 2;
        gui_show_transparent_image(x,y,imageDigit,0);
    }

	gui_measure_image(imageSpeed,&iImgWdt,&iImgHgt);
    x = me->screenWidth - iImgWdt;
    y += iImgHgt + 6;
	gui_show_transparent_image(x,y,imageSpeed,0);

    y += iImgHgt + 2;
    x = me->screenWidth;
    for( i = 0, power = 1; i < 4; i ++, power *= 10)
    {
		imageDigit = (PU8)get_image(IMG_ID_GX_TETRIS_DIGIT_0+( me->gameSpeed / power % 10));
		gui_measure_image(imageDigit,&iImgWdt,&iImgHgt);
        x -= iImgWdt + 2;
        gui_show_transparent_image(x,y,imageDigit,0);
    }


}

static void redrawTheScreen( void)
{
    int  i,j;
    char tetrisModel;
    TetRect rect;

    resetControlsAndClearScreen(  TRUE);
    drawPlayingZoneBorder();
    drawGameScoreAndGameSpeed();

    me->flashCounterWhenDeleteDirtyRows = 0;
    me->dirtyRowNumber                  = 0;
    rect.dx                             = me->gridSideLength - 1;
    rect.dy                             = rect.dx;

    for( i = me->gridRowNumber - 1; i >= me->theTopmostRowOfThePinnedTetrises; i --)
    {

        for( j = 0; j < me->gridColumnNumber; j ++)
        {

            if( me->gridMatrix[i][j].dirty)
            {
                rect.x      = j * me->gridSideLength + me->playingZone.x;
                rect.y      = i * me->gridSideLength + me->playingZone.y;
                tetrisModel     = me->gridMatrix[i][j].tetrisModel;

				gui_fill_rectangle(rect.x,rect.y,rect.x+rect.dx-1,rect.y+rect.dy-1,
								  gui_color(COLOR_PALETTE[tetrisModel].r,COLOR_PALETTE[tetrisModel].g,
								  		    COLOR_PALETTE[tetrisModel].b));
            }
        }
    }

    drawTheFallingTetris();
    drawTheNextFallingTetris();

    if(me->gameState == GAME_STATE_PAUSED)
    {
        promptTheGameIsPausedNow();
    }
    else if(me->gameState == GAME_STATE_NEXTLEVEL)
    {
        gotoNextLevel(FALSE);
    }
    else if(me->gameState == GAME_STATE_RUNNING)
    {
        moveDownwardsIf();
    }
    else
    {
        gui_BLT_double_buffer(0, 0, UI_device_width - 1, UI_device_height - 1);
    }
}

static void drawTetris( CTetris *tetris, boolean bOverDraw)
{

    int     i = 0;
    RGBVAL  color;
    TetRect rect;

    if (bOverDraw)
    {
		color.r = 0;
		color.g = 0;
		color.b = 0;
	}
	else
	{
		color.r = COLOR_PALETTE[tetris->model].r;
		color.g = COLOR_PALETTE[tetris->model].g;
		color.b = COLOR_PALETTE[tetris->model].b;
	}
    for( i = 0; i < 4; i ++)
    {

        rect.x  = tetris->block[i].x;
        rect.y  = tetris->block[i].y;
        rect.dx = me->gridSideLength - 1;
        rect.dy = me->gridSideLength - 1;

        gui_fill_rectangle(rect.x,rect.y,rect.x+rect.dx-1,rect.y+rect.dy-1,
								  gui_color(color.r,color.g,color.b));
    }

    gui_BLT_double_buffer(0, 0, UI_device_width - 1, UI_device_height - 1);
}

static void drawTheFallingTetris( void)
{

    drawTetris( &me->theFallingTetris, FALSE);
}

static void clearTheFallingTetris( void)
{

    drawTetris( &me->theFallingTetris, TRUE);
}

static void drawTheNextFallingTetris( void)
{

    drawTetris( &me->theNextFallingTetris, FALSE);
}

static void clearTheNextFallingTetris( void)
{

    drawTetris( &me->theNextFallingTetris, TRUE);
}

static boolean playMusic( MusicTypeEnum type)
{
	void* buffer;
	U32 size;
	debug("sound=====%d",me->soundOn);
    if( me->soundOn == FALSE)
    {
        return FALSE;
    }

    switch(type)
    {
        case MUSIC_PLACE:
			buffer = tetris_sfx_blink;
			size = tetris_sfx_blink_size;
            break;

        case MUSIC_SINLINE:
            buffer = tetris_sfx_singleline;
			size = tetris_sfx_singleline_size;
            break;

        case MUSIC_MULLINES:
            buffer = tetris_sfx_mullines;
			size = tetris_sfx_mullines_size;
            break;

        case MUSIC_NEXTSPEED:
            buffer = tetris_sfx_hero;
			size = tetris_sfx_hero_size;
            break;

        case MUSIC_NEXTLEVEL:
            buffer = tetris_sfx_hero;
			size = tetris_sfx_hero_size;
            break;

        case MUSIC_GAMEOVER:
            buffer = tetris_sfx_fail;
			size = tetris_sfx_fail_size;
            break;

        default:
            return FALSE;
    }
	debug("play sound");
    GFX_PLAY_AUDIO_MIDI_TITLE(buffer, size, DEVICE_AUDIO_PLAY_ONCE);
    return TRUE;
} // playMusic

static void refreshPlayingZoneAfterDeleteDirtyRows( void)
{

    int i = 0;
    int j = 0;
    int tetrisModel;
    TetRect rect;

    for( i = me->theTopmostRowOfThePinnedTetrises; i <= me->dirtyRowIndex[0]; i++)
    {
        for(j = 0; j < me->gridColumnNumber; j++)
        {
            rect.x = j * me->gridSideLength + me->playingZone.x;
            rect.y = i * me->gridSideLength + me->playingZone.y;
            rect.dx = rect.dy = me->gridSideLength - 1;
            if( !me->gridMatrix[i][j].dirty)    // Use background color.
            {
				gui_fill_rectangle(rect.x,rect.y,rect.x+rect.dx-1,rect.y+rect.dy-1,COLOR_BACKGROUND);
            }
            else                                    // Use color of the block.
            {
                tetrisModel = me->gridMatrix[i][j].tetrisModel;
				gui_fill_rectangle(rect.x,rect.y,rect.x+rect.dx-1,rect.y+rect.dy-1,
								  gui_color(COLOR_PALETTE[tetrisModel].r,COLOR_PALETTE[tetrisModel].g,
								  		    COLOR_PALETTE[tetrisModel].b));
            }
        }
    }
    me->theTopmostRowOfThePinnedTetrises += me->dirtyRowNumber;
    gui_BLT_double_buffer(0, 0, UI_device_width - 1, UI_device_height - 1);
} // refreshPlayingZoneAfterDeleteDirtyRows

static boolean updateGameScoreAndGoToNextLevelIf( void)
{

    switch( me->dirtyRowNumber)
    {
        case 1:
            me->gameScore += 10;
            break;

        case 2:
            me->gameScore += 30;
            break;

        case 3:
            me->gameScore += 50;
            break;

        case 4:
            me->gameScore += 100;
            break;
    }

    if( me->gameScore >= LEVEL_SCORE)
    {

        playMusic( MUSIC_NEXTSPEED);

        me->gameSpeed += 1;
        me->gameScore -= LEVEL_SCORE;
        me->sleepTime -= TIME_SLICE;
        drawGameScoreAndGameSpeed();
		
        if(me->sleepTime <= FLASH_TIME)
        {

            playMusic( MUSIC_NEXTLEVEL);
            killTimer();
            gotoNextLevel( TRUE);
            return FALSE;
        }
        else if( me->gameSpeed ==
                 ( INIT_TIME - (MAX_LEVEL - me->gameLevel + 1) * INIT_TIME
                         / MAX_LEVEL
                 ) / TIME_SLICE
        )
        {

            playMusic( MUSIC_NEXTLEVEL);

            killTimer();
            gotoNextLevel( FALSE);
            return FALSE;
        }
    }

    drawGameScoreAndGameSpeed();
    return TRUE;
} // updateGameScoreAndGoToNextLevelIf

static void drawTheFinalTetrisWhichFillupThePlayingZone( int16 yFrowWhereToDrawTheTetris)
{

    if( yFrowWhereToDrawTheTetris <= me->playingZone.y)
    {
        return;
    }
    else
    {

        int i       = 0;
        int height  = 0;
        int row     = 0;
        int col     = 0;
        TetRect rect;

        rect.dx = me->gridSideLength - 1;
        rect.dy = rect.dx;

        height  = yFrowWhereToDrawTheTetris - me->playingZone.y;
        row     = 0;
        col     = 0;
        for( i = 0; i < 4; i++)
        {

            rect.x = me->theFallingTetris.block[i].x;
            rect.y = me->theFallingTetris.block[i].y - height;

            if( rect.y < me->playingZone.y)
            {
                continue;
            }

			gui_fill_rectangle(rect.x,rect.y,rect.x+rect.dx-1,rect.y+rect.dy-1,
								  gui_color(COLOR_PALETTE[me->theFallingTetris.model].r,
								  			COLOR_PALETTE[me->theFallingTetris.model].g,
								  		    COLOR_PALETTE[me->theFallingTetris.model].b));

            row = (rect.y - me->playingZone.y) / me->gridSideLength;
            col = (rect.x - me->playingZone.x) / me->gridSideLength;

            me->gridMatrix[row][col].dirty      = TRUE;
            me->gridMatrix[row][col].tetrisModel = me->theFallingTetris.model;
        }

        gui_BLT_double_buffer(0, 0, UI_device_width - 1, UI_device_height - 1);
    }
} // drawTheFinalTetrisWhichFillupThePlayingZone

static void flashWhenDeleteDirtyRows( void)
{
    int     i       = 0;
    int     j       = 0;
    color   dcolor;
    TetRect rect;

    if( me->flashCounterWhenDeleteDirtyRows % 2 == 0)
    {
        dcolor = gui_color(205,179,139);
    }
    else
    {
        dcolor = COLOR_BACKGROUND;
    }

    rect.dx = me->gridSideLength - 1;
    rect.dy = me->gridSideLength - 1;
    for( i = 0; i < me->dirtyRowNumber; i++)
    {

        rect.x = me->playingZone.x;
        rect.y = me->dirtyRowIndex[i] * me->gridSideLength + me->playingZone.y;
        for( j = 0; j < me->gridColumnNumber; j++)
        {
            gui_fill_rectangle(rect.x,rect.y,rect.x+rect.dx-1,rect.y+rect.dy-1,dcolor);
            rect.x += me->gridSideLength;
        }
    }
    gui_BLT_double_buffer(0, 0, UI_device_width - 1, UI_device_height - 1);

    if(me->flashCounterWhenDeleteDirtyRows < MAX_FLASH_TIMES)
    {

        me->flashCounterWhenDeleteDirtyRows += 1;
        gui_start_timer(FLASH_TIME, flashWhenDeleteDirtyRows);
    }
    else
    {

        refreshPlayingZoneAfterDeleteDirtyRows();

        me->flashCounterWhenDeleteDirtyRows = 0;
        me->dirtyRowNumber = 0;
        drawTheFallingTetris();

		gui_start_timer(me->sleepTime, moveDownwardsIf);
    }
} // flashWhenDeleteDirtyRows

static void gameOver( void)
{

    setGameState(GAME_STATE_OVER);

	killTimer();
    clearPlayingZoneAfterGameOver();
}

static void clearPlayingZoneAfterGameOver( void)
{

    int column = 0;

    if( me->gridToClear.y > me->playingZone.y)
    {
        me->gridToClear.y -= me->gridSideLength;
    }
    else
    {
        saveGameDataAndDisplayGameScore();
        return;
    }

    me->gridToClear.x = me->playingZone.x;
    for( column = 0; column < me->gridColumnNumber; column ++)
    {
		gui_fill_rectangle(me->gridToClear.x,me->gridToClear.y,me->gridToClear.x+me->gridToClear.dx-1,
						   me->gridToClear.y+me->gridToClear.dy-1, gui_color(0, 0xcf, 0xff));
        me->gridToClear.x += me->gridSideLength;
    }
    gui_BLT_double_buffer(0, 0, UI_device_width - 1, UI_device_height - 1);

    gui_start_timer(OVER_TIME, clearPlayingZoneAfterGameOver);
}

static void saveGameDataAndDisplayGameScore( void)
{

    int     gameLevelStartFromZero;
    uint16  baseSpeed;
    uint16  totalScore;

    gameLevelStartFromZero      = g_gx_tetris_context.gameLevel - 1;
    baseSpeed   = (INIT_TIME - (( MAX_LEVEL - gameLevelStartFromZero) * INIT_TIME / MAX_LEVEL)) / TIME_SLICE;
    totalScore  = (g_gx_tetris_context.gameSpeed - baseSpeed) * LEVEL_SCORE + g_gx_tetris_context.gameScore;
    if( totalScore > g_gx_tetris_context.configData.topScore[gameLevelStartFromZero])
    {
        g_gx_tetris_context.configData.topScore[gameLevelStartFromZero] = totalScore;
        saveSettingData();
        displayGameScoreScreen( STR_GX_TETRIS_REPORT_RECORD, totalScore, TRUE);
    }
    else
    {
        displayGameScoreScreen( STR_GX_TETRIS_REPORT_SCORE, totalScore, FALSE);
    }
} // saveGameDataAndDisplayGameScore

static void resetControlsAndClearScreen( boolean clearScreen)
{
    if( clearScreen)
    {
		clear_screen();
    }
} // resetControlsAndClearScreen

static void drawStringBox()
{
	S32 maxht, fh; 

	create_multiline_inputbox_set_buffer(drawee_string, gui_strlen(drawee_string), gui_strlen(drawee_string), 0);
	MMI_multiline_inputbox.flags |= UI_MULTI_LINE_INPUT_BOX_DISABLE_CURSOR_DRAW
        | UI_MULTI_LINE_INPUT_BOX_CENTER_JUSTIFY
        | UI_MULTI_LINE_INPUT_BOX_DISABLE_SCROLLBAR | UI_MULTI_LINE_INPUT_BOX_VIEW_MODE;
	resize_multiline_inputbox(UI_device_width - 2, UI_device_height);
    show_multiline_inputbox_no_draw();
    fh = get_multiline_inputbox_line_height();

    /* Move the inputbox to appropriate position */
#ifdef __MMI_MAINLCD_128X128__
    maxht = (MMI_multiline_inputbox.n_lines * fh) + MULTILINE_INPUTBOX_HEIGHT_PAD + MMI_multiline_inputbox.text_y + 10;
#else 
    maxht = (MMI_multiline_inputbox.n_lines * fh) + MULTILINE_INPUTBOX_HEIGHT_PAD + MMI_multiline_inputbox.text_y;
#endif 

    if (maxht > UI_device_height)
    {
        maxht = UI_device_height;
    }
	resize_multiline_inputbox(UI_device_width, maxht);
	show_multiline_inputbox();
}

void displayScreenStop(void)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    /* stop all playing audio and vibration */
    VibratorOff();
    mdi_audio_stop_string();

    /* resume if there is background playing */
    mdi_audio_resume_background_play();

}

extern void mmi_gfx_enter_game(void);
extern void mmi_gfx_exit_game(void);

static void gotoNextLevelYesButton(void)
{
	S16 error;
	MMI_BOOL entry_ret = MMI_FALSE;
	me->gameLevel += 1;
	//copy of mmi_gfx_enter_game
	mmi_frm_scrn_close_active_id();
	entry_ret = mmi_frm_scrn_enter (GFX.cur_gid, GFX_GAME_SCREEN, mmi_gfx_exit_game, mmi_gfx_enter_game, MMI_FRM_FG_ONLY_SCRN);
	if (!entry_ret)
	{
		kal_prompt_trace(MOD_MMI, "Misstion is imposible");
		return;
	}

    /* reset clip */
    gdi_layer_reset_clip();
    gdi_layer_reset_text_clip();

    /* entry full screen app */
    entry_full_screen();

    ClearInputEventHandler(MMI_DEVICE_ALL);
    clear_category_screen_key_handlers();

    SetKeyHandler(mmi_frm_scrn_close_active_id, KEY_RSK, KEY_EVENT_UP);
    SetKeyHandler(mmi_gfx_volume_up, KEY_VOL_UP, KEY_EVENT_DOWN);
    SetKeyHandler(mmi_gfx_volume_down, KEY_VOL_DOWN, KEY_EVENT_DOWN);

    /* reset clip */
    gui_reset_text_clip();
    gui_reset_clip();

    /* 
     * This is used to solve a very rare situation. When playing a IMELODY 
     * with backlight on/off, and the screen previous to this screen is a 
     * horizontal screen. Before enter this screen, the IMELODY turn off the
     * backlight. While exit previous screen, the layer rotate back to normal
     * size and the content is not correct. So when calling TurnOnBacklight, 
     * LCD is in sleepin state and draw wrong content to LCD.
     * So we need to clear the buffer first to avoid this situation.
     */
    gdi_layer_clear(GDI_COLOR_BLACK);

    /* stop MMI sleep */
    //TurnOnBacklight(0);

#ifdef __MMI_SUBLCD__
    /* draw game icon on sublcd */
    ForceSubLCDScreen(mmi_gfx_entry_sublcd_screen);
#endif /* __MMI_SUBLCD__ */ 
    /*
     * fill background with white color - 
     * for display smaller game on bigger LCM that will have clear background 
     */
    gui_fill_rectangle(0, 0, UI_device_width - 1, UI_device_height - 1, gui_color(255, 255, 255));

    /* lock and blt, this may force full screen region will be blt when enter game */
    gdi_layer_lock_frame_buffer();
    gdi_layer_blt_previous(0, 0, UI_device_width - 1, UI_device_height - 1);
    gdi_layer_unlock_frame_buffer();

    /* disalbe align timer and enter game */
    UI_disable_alignment_timers();
#ifdef __MTK_TARGET__
    mmi_frm_set_key_handle_in_high_frequency(MMI_TRUE);
#endif 

    MMI_PRINT(MOD_MMI_MEDIA_APP, MMI_MEDIA_TRC_G2_APP, "*---[Tetris.c] Enter game after level screen---*\n",
                         GetString(GFX.game_data.game_str_id));

    GFX.game_data.enter_game_func_ptr();
	tetris_nextlevel = FALSE; //back to the original value
}

static void gotoNextLevel( boolean isLastLevel)
{
	uint16  gameLevelStartFromZero;
    uint16  levelInitSpeed;
    uint16  titleId;
    uint16  totalScore;
    uint32  staticProperties;
    WCHAR  formatString[128];
    WCHAR  displayString[128];
	U8 *guiBuffer;  /* Buffer holding history data */
	MMI_BOOL entry_ret;

    killTimer();
    resetControlsAndClearScreen( FALSE);

	gameLevelStartFromZero  = me->gameLevel - 1;
    levelInitSpeed = (INIT_TIME - ( ( MAX_LEVEL - gameLevelStartFromZero) * INIT_TIME / MAX_LEVEL)) / TIME_SLICE;
    totalScore = (me->gameSpeed - levelInitSpeed) * LEVEL_SCORE;
    totalScore += isLastLevel ?g_gx_tetris_context.gameScore : 0;
    if(g_gx_tetris_context.configData.topScore[gameLevelStartFromZero] < totalScore)
    {
       g_gx_tetris_context.configData.topScore[gameLevelStartFromZero] = totalScore;
        saveSettingData();
    }
    mmi_wcscpy(formatString, (WCHAR *) GetString(isLastLevel ? STR_GX_TETRIS_PASS_LAST : STR_GX_TETRIS_PASS));
    mmi_wsprintf_ex( displayString, sizeof( displayString), formatString, totalScore);
	mmi_wcscpy(drawee_string, displayString);

	tetris_nextlevel = TRUE;

	entry_full_screen();

	entry_ret = mmi_frm_scrn_enter (GFX.cur_gid, GFX_GAMEOVER_SCREEN, displayScreenStop, displayGameScoreScreen, MMI_FRM_UNKNOW_SCRN);
	if (!entry_ret)
	{
		kal_prompt_trace(MOD_MMI, "Misstion is imposible");
		return;
	}
    guiBuffer = mmi_frm_scrn_get_gui_buf(GFX.cur_gid, GFX_GAMEOVER_SCREEN);

    /* not first time enter */
    if (guiBuffer != NULL)
    {
        GFX.is_first_time_enter_gameover = FALSE;
    }

    if( isLastLevel)
    {
		ShowCategory221Screen(
			0,
			0,                                      /* caption */
			0,
			0,                          /* LSK */
			STR_GLOBAL_BACK,
			IMG_GLOBAL_BACK,                                      /* RSK */
			GDI_COLOR_WHITE,
			drawStringBox);
    }
    else
    {
		ShowCategory221Screen(
			0,
			0,                                      /* caption */
			STR_GLOBAL_YES,
			IMG_GLOBAL_YES,                          /* LSK */
			STR_GLOBAL_NO,
			IMG_GLOBAL_NO,                                      /* RSK */
			GDI_COLOR_WHITE,
			drawStringBox);
    }
	SetLeftSoftkeyFunction(gotoNextLevelYesButton, KEY_EVENT_UP);
	SetRightSoftkeyFunction(mmi_frm_scrn_close_active_id, KEY_EVENT_UP);
    setGameState(isLastLevel ? GAME_STATE_LASTLEVEL : GAME_STATE_NEXTLEVEL);
} // gotoNextLevel

static void displayGameScoreScreen( uint16 resourceId, uint32 theScore, boolean recordBroken)
{
    WCHAR   scoreFormatString[128];
    WCHAR   scoreString[128];
    U8 *guiBuffer;  /* Buffer holding history data */
	MMI_BOOL entry_ret;

    resetControlsAndClearScreen( FALSE);

	mmi_wcscpy(scoreFormatString, (WCHAR *) GetString(resourceId));
    if( recordBroken)
    {

		mmi_wsprintf_ex(scoreString, sizeof(scoreString), scoreFormatString, theScore);
    }
    else
    {
		mmi_wsprintf_ex(scoreString, sizeof(scoreString), scoreFormatString, theScore,
						g_gx_tetris_context.configData.topScore[g_gx_tetris_context.gameLevel - 1]);
    }
	mmi_wcscpy(drawee_string, scoreString);
    entry_full_screen();
	entry_ret = mmi_frm_scrn_enter (GFX.cur_gid, GFX_GAMEOVER_SCREEN, displayScreenStop, displayGameScoreScreen, MMI_FRM_UNKNOW_SCRN);
	if (!entry_ret)
	{
		kal_prompt_trace(MOD_MMI, "Misstion is imposible");
		return;
	}
    guiBuffer = mmi_frm_scrn_get_gui_buf(GFX.cur_gid, GFX_GAMEOVER_SCREEN);

    /* not first time enter */
    if (guiBuffer != NULL)
    {
        GFX.is_first_time_enter_gameover = FALSE;
    }

    /* suspend background play */
    mdi_audio_suspend_background_play();
	if (recordBroken)
		GFX_PLAY_AUDIO_COMPLETE();
	else
		GFX_PLAY_AUDIO_GAMEOVER();

    ShowCategory221Screen(
        0,
        0,                                      /* caption */
        0,
        0,                          /* LSK */
        STR_GLOBAL_BACK,
        IMG_GLOBAL_BACK,                                      /* RSK */
        GDI_COLOR_WHITE,
        drawStringBox);  /* redraw callback */

    /* go back to game menu */
    /*SetKeyHandler(mmi_frm_scrn_close_active_id, KEY_LEFT_ARROW, KEY_EVENT_DOWN);*/

    /* gameover will go back to first menuitem */
    g_gx_tetris_context.is_gameover = TRUE;
    g_gx_tetris_context.is_new_game = TRUE;

    SetRightSoftkeyFunction(mmi_frm_scrn_close_active_id, KEY_EVENT_UP);
	tetris_ingame = FALSE; //back to the original value
	tetris_nextlevel = FALSE;
    setGameState(GAME_STATE_REPORT);
} // displayGameScoreScreen

/* Decided to add this improvised method for showing the splash screen with a delay... */
void mmi_gfx_entry_menu_screen_tetris(void)
{
	if (mmi_gx_tetris_showonce_timer_stop == TRUE)
	{
		gui_cancel_timer(mmi_gfx_entry_menu_screen_tetris);
		mmi_gx_tetris_showonce_timer_stop = FALSE;
	}
	mmi_gfx_entry_menu_screen();
}

static void getSettingData(void)
{
	S16 error;
	int i;
#ifdef __MMI_GAME_MULTICHANNEL_SOUND__
	ReadValue(NVRAM_GFX_SOUND_EFFECT_SETTING, &me->configData.soundOn, DS_BYTE, &error);
#else /* __MMI_GAME_MULTICHANNEL_SOUND__ */ 
    ReadValue(NVRAM_GFX_AUDIO_SETTING, &me->configData.soundOn, DS_BYTE, &error);
#endif
	if (error != NVRAM_READ_SUCCESS)
		goto PrefReset;

	ReadValue(NVRAM_GAME_TETRIS_LEVEL, &me->configData.gameLevel, DS_BYTE, &error);
	if (error != NVRAM_READ_SUCCESS)
		goto PrefReset;

	me->configData.gameLevel++;
	me->configData.drawGridLines = TRUE;
	
	ReadValue(NVRAM_GX_TETRIS_SCORE_EASY, &me->configData.topScore[0], DS_SHORT, &error);
	if (error != NVRAM_READ_SUCCESS)
		goto PrefReset;
	ReadValue(NVRAM_GX_TETRIS_SCORE_MEDIUM, &me->configData.topScore[1], DS_SHORT, &error);
	if (error != NVRAM_READ_SUCCESS)
		goto PrefReset;
	ReadValue(NVRAM_GX_TETRIS_SCORE_HARD, &me->configData.topScore[2], DS_SHORT, &error);
	if (error != NVRAM_READ_SUCCESS)
		goto PrefReset;

	goto End;

PrefReset:
    me->configData.gameLevel        = 1;
    me->configData.drawGridLines    = FALSE;
    me->configData.soundOn      	= FALSE;

    for( i = 0; i < MAX_LEVEL; i ++)
    {
        me->configData.topScore[i] = 0;
    }

End:
	return;
}

static void saveSettingData( void)
{
	S16 error;
	U8 level_store;

    g_gx_tetris_context.configData.gameLevel        = g_gx_tetris_context.gameLevel;
    g_gx_tetris_context.configData.drawGridLines    = g_gx_tetris_context.drawGridLines;
    g_gx_tetris_context.configData.soundOn          = g_gx_tetris_context.soundOn;
	
	level_store = g_gx_tetris_context.configData.gameLevel-1;
	WriteValue(NVRAM_GAME_TETRIS_LEVEL, &level_store, DS_BYTE, &error);
	if (error != NVRAM_WRITE_SUCCESS)
		return;
	
	WriteValue(NVRAM_GX_TETRIS_SCORE_EASY, &g_gx_tetris_context.configData.topScore[0], DS_SHORT, &error);
	if (error != NVRAM_WRITE_SUCCESS)
		return;
	WriteValue(NVRAM_GX_TETRIS_SCORE_MEDIUM, &g_gx_tetris_context.configData.topScore[1], DS_SHORT, &error);
	if (error != NVRAM_WRITE_SUCCESS)
		return;
	WriteValue(NVRAM_GX_TETRIS_SCORE_HARD, &g_gx_tetris_context.configData.topScore[2], DS_SHORT, &error);
	if (error != NVRAM_WRITE_SUCCESS)
		return;
}

void mmi_gx_tetris_draw_gamescore(WCHAR* str)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    U8 buffer[64] = {0};
    U8 buffer_UCS2[64] = {0};
    S32 text_image_width = 0;
    S32 text_image_height = 0;
    S32 box_image_width = 0;
    S32 box_image_height = 0;
    S32 pic_image_width = 0;
    S32 pic_image_height = 0;
    S32 text_image_offset_y = 0;
    S32 box_image_offset_y = 0;
    S32 pic_image_offset_y = 0;
    S32 score_str_offset_y = 0;
    S32 score_str_offset_x = 0;
    S32 str_width = 0;
    S32 str_height = 0;
    S32 spacing = 0;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    clear_screen();
    gui_reset_clip();
    gui_reset_text_clip();
    gui_set_font(&MMI_medium_font);

    spacing = (UI_device_height - MMI_button_bar_height - (text_image_height + box_image_height + pic_image_height)) >> 2;

    text_image_offset_y = spacing;
    box_image_offset_y = text_image_offset_y + text_image_height + spacing;
    pic_image_offset_y = box_image_offset_y + box_image_height + spacing;

    gui_measure_string((UI_string_type) str, &str_width, &str_height);

    score_str_offset_x = (UI_device_width - str_width) >> 1;
    score_str_offset_y = box_image_offset_y + ((box_image_height - str_height) >> 1);

    gui_set_text_color(gui_color(0, 0, 0));

    if (r2lMMIFlag)
    {
        gui_move_text_cursor(score_str_offset_x + str_width, score_str_offset_y);
    }
    else
    {
        gui_move_text_cursor(score_str_offset_x, score_str_offset_y);
    }

    gui_print_text((UI_string_type) str);
    gui_BLT_double_buffer(0, 0, UI_device_width - 1, UI_device_height - 1);

}

/*
** ===========================================================================
**
** Function:        
**     mmi_gx_tetris_enter_gfx
**
** Description: 
**     Prepare game framework
** 
** Input: 
**     none
** 
** Output: 
**     init'ed GFX
** 
** Return value: 
**     none
** 
** Side effects:
**     none
**
** ===========================================================================
*/

void mmi_gx_tetris_enter_gfx(void)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    /* Game menu */
    GFX.game_data.game_img_id = 0;   /* game icon img ID */
    GFX.game_data.game_str_id = STR_GX_TETRIS_GAME_NAME;   /* game name string ID */
    GFX.game_data.menu_resume_str_id = STR_GAME_RESUME;       /* "Resume" string ID */
    GFX.game_data.menu_new_str_id = STR_GAME_NEW; /* "New Game" string ID */
    GFX.game_data.menu_level_str_id = STR_GAME_LEVEL; /* "Game Level" string ID */
    GFX.game_data.menu_grade_str_id = STR_GAME_GRADE; /* "Best Grade" string ID */
    GFX.game_data.menu_help_str_id = STR_GLOBAL_HELP;   /* "Game Help" string ID */

    /* level / grade */
    GFX.game_data.level_count = 3;  /* how many levels */
    GFX.game_data.level_str_id_list[0] = STR_GX_TETRIS_EASY; /* level string ID */
    GFX.game_data.level_str_id_list[1] = STR_GX_TETRIS_NORMAL;       /* level string ID */
    GFX.game_data.level_str_id_list[2] = STR_GX_TETRIS_HARD; /* level string ID */

    /* add slot in NVRAMEnum.h */
    GFX.game_data.grade_nvram_id_list[0] = NVRAM_GX_TETRIS_SCORE_EASY;  /* grade slot in NVRAM (short) */
    GFX.game_data.grade_nvram_id_list[1] = NVRAM_GX_TETRIS_SCORE_MEDIUM;  /* grade slot in NVRAM */
    GFX.game_data.grade_nvram_id_list[2] = NVRAM_GX_TETRIS_SCORE_HARD;  /* grade slot in NVRAM */
    GFX.game_data.level_nvram_id = NVRAM_GAME_TETRIS_LEVEL;          /* current lvl idnex  in NVRAM (byte) */

    /* help */
    GFX.game_data.help_str_id = STR_GX_TETRIS_HELP_DESCRIPTION;    /* help desciption string id */

    /* misc */
    GFX.game_data.grade_value_ptr = (S16*) (&g_gx_tetris_context.gameScore);        /* current level's grade (S16*) */
    GFX.game_data.level_index_ptr = (U8*) (&dummy_gamelevel_val); /* ptr to current level index (U8*) */
    GFX.game_data.is_new_game = (BOOL*) (&g_gx_tetris_context.is_new_game);  /* ptr to new game flag (BOOL*) */

    /* function ptr */
    GFX.game_data.enter_game_func_ptr = mmi_gx_tetris_enter_game;     /* function to enter new game */
    GFX.game_data.exit_game_func_ptr = mmi_gx_tetris_exit_game;       /* function to exit game */
    
    /* some flags */
    GFX.game_data.is_keypad_audio_enable = FALSE;   /* play keypad tone or not */
	mmi_gx_tetris_showonce_timer_stop = TRUE;

	displaySplashScreen();
    gui_start_timer(INIT_TIME, mmi_gfx_entry_menu_screen_tetris);
}

/*
** ===========================================================================
**
** Function:        
**     mmi_gx_tetris_enter_game
**
** Description: 
**     Enters game
** 
** Input: 
**     none
** 
** Output: 
**     none
** 
** Return value: 
**     none
** 
** Side effects:
**     none
**
** ===========================================================================
*/

void mmi_gx_tetris_enter_game(void)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (g_gx_tetris_context.is_new_game == TRUE)
    {
        mmi_gx_tetris_init_game();    /* is new game, otherwise resume game */
    }

    g_gx_tetris_context.is_gameover = FALSE;

	SetKeyHandler(Tetris_2Key, KEY_2, KEY_EVENT_DOWN);
    SetKeyHandler(Tetris_4Key, KEY_4, KEY_EVENT_DOWN);
	SetKeyHandler(Tetris_5Key, KEY_5, KEY_EVENT_DOWN);
    SetKeyHandler(Tetris_6Key, KEY_6, KEY_EVENT_DOWN);
    SetKeyHandler(Tetris_8Key, KEY_8, KEY_EVENT_DOWN);
	SetKeyHandler(Tetris_2Key, KEY_UP_ARROW, KEY_EVENT_DOWN);
    SetKeyHandler(Tetris_4Key, KEY_LEFT_ARROW, KEY_EVENT_DOWN);
	SetKeyHandler(Tetris_5Key, KEY_IP, KEY_EVENT_DOWN);
    SetKeyHandler(Tetris_6Key, KEY_RIGHT_ARROW, KEY_EVENT_DOWN);
    SetKeyHandler(Tetris_8Key, KEY_DOWN_ARROW, KEY_EVENT_DOWN);
	register_keyboard_key_handler(Tetris_KeyboardKey);

    /* start game loop */
    if (g_gx_tetris_context.is_gameover == FALSE)
    {
		if (g_gx_tetris_context.is_new_game == TRUE)
		{
			g_gx_tetris_context.is_new_game = FALSE;
			startOneNewGame();
		}
		else
		{
			if (tetris_nextlevel == TRUE)
				me->gameLevel += 1;
            setGameState(GAME_STATE_RUNNING);
            redrawTheScreen();
		}
    }
}

/*
** ===========================================================================
**
** Function:        
**     mmi_gx_tetris_exit_game
**
** Description: 
**     Exit game function
** 
** Input: 
**     none
** 
** Output: 
**     none
** 
** Return value: 
**     none
** 
** Side effects:
**     none
**
** ===========================================================================
*/

void mmi_gx_tetris_exit_game(void)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
	if (tetris_ingame == FALSE)
		freeGameDataMemory();
	killTimer();
}

/*
** ===========================================================================
**
** Function:        
**     mmi_gx_tetris_init_game
**
** Description: 
**     Initialize game
** 
** Input: 
**     none
** 
** Output: 
**     none
** 
** Return value: 
**     none
** 
** Side effects:
**     none
**
** ===========================================================================
*/

void mmi_gx_tetris_init_game(void)
{
	if (tetris_ingame == FALSE)
	{
		if (initGameDataMemory() == TRUE)
			debug( ";Tetris_InitAppData success");
		else
			freeGameDataMemory();
	}
}

#endif /* __MMI_GAME_TETRIS__ */ 
