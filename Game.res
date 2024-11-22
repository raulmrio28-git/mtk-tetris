#ifdef __MMI_GAME_TETRIS__
    #define TETRIS_IMG_PATH CUST_IMG_BASE_PATH"\\\\GameImages\\\\Tetris\\\\"

    <STRING id="STR_GX_TETRIS_GAME_NAME"/>
    <STRING id="STR_GX_TETRIS_HELP_DESCRIPTION"/>
    <STRING id="STR_GX_TETRIS_GRID"/>
    <STRING id="STR_GX_TETRIS_EASY"/>
    <STRING id="STR_GX_TETRIS_NORMAL"/>
    <STRING id="STR_GX_TETRIS_HARD"/>
    <STRING id="STR_GX_TETRIS_REPORT_SCORE"/>
    <STRING id="STR_GX_TETRIS_REPORT_RECORD"/>
    <STRING id="STR_GX_TETRIS_PASS"/>
    <STRING id="STR_GX_TETRIS_PASS_LAST"/>
    <STRING id="STR_GX_TETRIS_HERO_LEVEL"/>
    <STRING id="STR_GX_TETRIS_HERO_SCORE"/>
	
	<IMAGE id="IMG_ID_GX_TETRIS_SPLASH">TETRIS_IMG_PATH"TetrisSplash1"GX_IMG_TYPE</IMAGE>
    <IMAGE id="IMG_ID_GX_TETRIS_DIGIT_0">TETRIS_IMG_PATH"digit_0"GX_IMG_TYPE</IMAGE>
    <IMAGE id="IMG_ID_GX_TETRIS_DIGIT_1">TETRIS_IMG_PATH"digit_1"GX_IMG_TYPE</IMAGE>
    <IMAGE id="IMG_ID_GX_TETRIS_DIGIT_2">TETRIS_IMG_PATH"digit_2"GX_IMG_TYPE</IMAGE>
    <IMAGE id="IMG_ID_GX_TETRIS_DIGIT_3">TETRIS_IMG_PATH"digit_3"GX_IMG_TYPE</IMAGE>
    <IMAGE id="IMG_ID_GX_TETRIS_DIGIT_4">TETRIS_IMG_PATH"digit_4"GX_IMG_TYPE</IMAGE>
    <IMAGE id="IMG_ID_GX_TETRIS_DIGIT_5">TETRIS_IMG_PATH"digit_5"GX_IMG_TYPE</IMAGE>
    <IMAGE id="IMG_ID_GX_TETRIS_DIGIT_6">TETRIS_IMG_PATH"digit_6"GX_IMG_TYPE</IMAGE>
    <IMAGE id="IMG_ID_GX_TETRIS_DIGIT_7">TETRIS_IMG_PATH"digit_7"GX_IMG_TYPE</IMAGE>
    <IMAGE id="IMG_ID_GX_TETRIS_DIGIT_8">TETRIS_IMG_PATH"digit_8"GX_IMG_TYPE</IMAGE>
    <IMAGE id="IMG_ID_GX_TETRIS_DIGIT_9">TETRIS_IMG_PATH"digit_9"GX_IMG_TYPE</IMAGE>
    <IMAGE id="IMG_ID_GX_TETRIS_SCORE">TETRIS_IMG_PATH"TetrisScore"GX_IMG_TYPE</IMAGE>
	<IMAGE id="IMG_ID_GX_TETRIS_START">TETRIS_IMG_PATH"TetrisStart"GX_IMG_TYPE</IMAGE>
	<IMAGE id="IMG_ID_GX_TETRIS_LEVEL">TETRIS_IMG_PATH"TetrisLevel"GX_IMG_TYPE</IMAGE>
	<IMAGE id="IMG_ID_GX_TETRIS_SCORE">TETRIS_IMG_PATH"TetrisScore"GX_IMG_TYPE</IMAGE>
	<IMAGE id="IMG_ID_GX_TETRIS_HELP">TETRIS_IMG_PATH"TetrisHelp"GX_IMG_TYPE</IMAGE>
	<IMAGE id="IMG_ID_GX_TETRIS_CONTINUE">TETRIS_IMG_PATH"TetrisContinue"GX_IMG_TYPE</IMAGE>
	<IMAGE id="IMG_ID_GX_TETRIS_SOUND">TETRIS_IMG_PATH"TetrisSound"GX_IMG_TYPE</IMAGE>
	<IMAGE id="IMG_ID_GX_TETRIS_GRID">TETRIS_IMG_PATH"TetrisGrid"GX_IMG_TYPE</IMAGE>
    <IMAGE id="IMG_ID_GX_TETRIS_GAMESPEED_EN">TOPGUN_IMG_PATH"speed.en"GX_IMG_TYPE</IMAGE>
    <IMAGE id="IMG_ID_GX_TETRIS_GAMESCORE_EN">TOPGUN_IMG_PATH"score.en"GX_IMG_TYPE</IMAGE>
	<IMAGE id="IMG_ID_GX_TETRIS_GAMEPAUSE_EN">TOPGUN_IMG_PATH"paused.en"GX_IMG_TYPE</IMAGE>
#ifdef    __MMI_GAME_MULTI_LANGUAGE_SUPPORT__
    <IMAGE id="IMG_ID_GX_TETRIS_GAMESPEED_CN">TOPGUN_IMG_PATH"speed.cn"GX_IMG_TYPE</IMAGE>
    <IMAGE id="IMG_ID_GX_TETRIS_GAMESCORE_CN">TOPGUN_IMG_PATH"score.cn"GX_IMG_TYPE</IMAGE>
	<IMAGE id="IMG_ID_GX_TETRIS_GAMEPAUSE_CN">TOPGUN_IMG_PATH"paused.cn"GX_IMG_TYPE</IMAGE>
#endif /*__MMI_GAME_MULTI_LANGUAGE_SUPPORT__*/

    /* Tetris NVRAM */
    <CACHEDATA type="short" id="NVRAM_GX_TETRIS_SCORE_EASY" restore_flag="TRUE">
        <DEFAULT_VALUE> [0xFF, 0xFF] </DEFAULT_VALUE>
        <DESCRIPTION> Short Cache </DESCRIPTION>
    </CACHEDATA>
    <CACHEDATA type="short" id="NVRAM_GX_TETRIS_SCORE_MEDIUM" restore_flag="TRUE">
        <DEFAULT_VALUE> [0xFF, 0xFF] </DEFAULT_VALUE>
        <DESCRIPTION> Short Cache </DESCRIPTION>
    </CACHEDATA>
    <CACHEDATA type="short" id="NVRAM_GX_TETRIS_SCORE_HARD" restore_flag="TRUE">
        <DEFAULT_VALUE> [0xFF, 0xFF] </DEFAULT_VALUE>
        <DESCRIPTION> Short Cache </DESCRIPTION>
    </CACHEDATA>
#endif  //__MMI_GAME_TETRIS__