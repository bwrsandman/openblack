/******************************************************************************
 * Copyright (c) 2018-2024 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *******************************************************************************/

#ifndef __SCRIPTENUMS_H__
#define __SCRIPTENUMS_H__

//------------------------------------------------------------------------------------------------------------------------
enum SCRIPT_FIND_TYPE
{
	SCRIPT_FIND_TYPE_ANY =5000,
};

//------------------------------------------------------------------------------------------------------------------------
enum SCRIPT_SPIRIT_TYPE
{
	SCRIPT_SPIRIT_TYPE_NONE,
	SCRIPT_SPIRIT_TYPE_GOOD,
	SCRIPT_SPIRIT_TYPE_EVIL,
	SCRIPT_SPIRIT_TYPE_ALIGNMENT,
	SCRIPT_SPIRIT_TYPE_ANTI_ALIGNMENT,
	SCRIPT_SPIRIT_TYPE_RANDOM,
	SCRIPT_SPIRIT_TYPE_LAST,
};

//------------------------------------------------------------------------------------------------------------------------
enum SCRIPT_PUZZLE_GAME_TYPE
{
	SCRIPT_PUZZLE_GAME_TYPE_NONE,
	SCRIPT_PUZZLE_GAME_TYPE_TREE_1,
	SCRIPT_PUZZLE_GAME_TYPE_TREE_2,
	SCRIPT_PUZZLE_GAME_TYPE_HANOI,
	SCRIPT_PUZZLE_GAME_TYPE_MAZE_1,
	SCRIPT_PUZZLE_GAME_TYPE_MAZE_2,
	SCRIPT_PUZZLE_GAME_TYPE_TOTEM_1,
	SCRIPT_PUZZLE_GAME_TYPE_TOTEM_2,
	SCRIPT_PUZZLE_GAME_TYPE_TOTEM_3,
	SCRIPT_PUZZLE_GAME_TYPE_TOTEM_4,
	SCRIPT_PUZZLE_GAME_TYPE_THESIUS1,
	SCRIPT_PUZZLE_GAME_TYPE_THESIUS2,
	SCRIPT_PUZZLE_GAME_TYPE_TILT1,
	SCRIPT_PUZZLE_GAME_TYPE_TILT2,
	SCRIPT_PUZZLE_GAME_TYPE_FISHES1,
	SCRIPT_PUZZLE_GAME_TYPE_FISHES2,
	SCRIPT_PUZZLE_GAME_TYPE_IMMERSION,
	SCRIPT_PUZZLE_GAME_TYPE_IMMERSION2,
	SCRIPT_PUZZLE_GAME_TYPE_LIONSHEEP,
	SCRIPT_PUZZLE_GAME_TYPE_CHESS,
	SCRIPT_PUZZLE_GAME_TYPE_LAST,
};

//------------------------------------------------------------------------------------------------------------------------
enum SCRIPT_PUZZLE_GAME_STATUS
{
	SCRIPT_PUZZLE_GAME_STATUS_WON,
	SCRIPT_PUZZLE_GAME_STATUS_LOST,
	SCRIPT_PUZZLE_GAME_STATUS_WON_GOODWAY,
	SCRIPT_PUZZLE_GAME_STATUS_WON_EVILWAY,
	SCRIPT_PUZZLE_GAME_STATUS_GAME_IN_PROGRESS,
	SCRIPT_PUZZLE_GAME_STATUS_GAME_NOT_YET_BEGUN,
	SCRIPT_PUZZLE_GAME_STATUS_LAST,
};

//------------------------------------------------------------------------------------------------------------------------
enum SCRIPT_OBJECT_PROPERTY_TYPE
{
	SCRIPT_OBJECT_PROPERTY_TYPE_NONE,
	SCRIPT_OBJECT_PROPERTY_TYPE_HEALTH,
	SCRIPT_OBJECT_PROPERTY_TYPE_ANGLE,
	SCRIPT_OBJECT_PROPERTY_TYPE_XANGLE,
	SCRIPT_OBJECT_PROPERTY_TYPE_ZANGLE,
	SCRIPT_OBJECT_PROPERTY_TYPE_FLYING,
	SCRIPT_OBJECT_PROPERTY_TYPE_DROWNING,
	SCRIPT_OBJECT_PROPERTY_TYPE_MOVING,
	SCRIPT_OBJECT_PROPERTY_TYPE_SCALE,
	SCRIPT_OBJECT_PROPERTY_TYPE_IN_HAND,
	// SCRIPT_OBJECT_PROPERTY_TYPE_IN_HAND_ANY,	<-- CI
	SCRIPT_OBJECT_PROPERTY_TYPE_IN_HAND_GRAB,
	SCRIPT_OBJECT_PROPERTY_TYPE_SPEED,
	SCRIPT_OBJECT_PROPERTY_TYPE_RUNNING_SPEED,
	SCRIPT_OBJECT_PROPERTY_TYPE_DEFAULT_SPEED,
	SCRIPT_OBJECT_PROPERTY_TYPE_DEATH,
	SCRIPT_OBJECT_PROPERTY_TYPE_THING_TYPE,
	SCRIPT_OBJECT_PROPERTY_TYPE_AGE,
	SCRIPT_OBJECT_PROPERTY_TYPE_STRENGTH,
	SCRIPT_OBJECT_PROPERTY_TYPE_ALIGNMENT,
	// SCRIPT_OBJECT_PROPERTY_TYPE_ACTUAL_HEIGHT,	<-- CI
	SCRIPT_OBJECT_PROPERTY_TYPE_HEIGHT,
	SCRIPT_OBJECT_PROPERTY_TYPE_MAX_HEIGHT,
	SCRIPT_OBJECT_PROPERTY_TYPE_PLAYER,
	SCRIPT_OBJECT_PROPERTY_TYPE_BUILT_PERCENTAGE,
	SCRIPT_OBJECT_PROPERTY_TYPE_XPOS,
	SCRIPT_OBJECT_PROPERTY_TYPE_YPOS,
	SCRIPT_OBJECT_PROPERTY_TYPE_ZPOS,
	// SCRIPT_OBJECT_PROPERTY_TYPE_RADIUS,			<-- CI
	// SCRIPT_OBJECT_PROPERTY_TYPE_DENSITY,		<-- CI

	// Enums for creature properties
	SCRIPT_OBJECT_PROPERTY_TYPE_CREATURE_WARMTH,
	SCRIPT_OBJECT_PROPERTY_TYPE_CREATURE_FATNESS,
	SCRIPT_OBJECT_PROPERTY_TYPE_CREATURE_ENERGY,
	SCRIPT_OBJECT_PROPERTY_TYPE_CREATURE_ITCHINESS,
	SCRIPT_OBJECT_PROPERTY_TYPE_CREATURE_AMOUNT_OF_POO,
	SCRIPT_OBJECT_PROPERTY_TYPE_CREATURE_EXHAUSTION,
	SCRIPT_OBJECT_PROPERTY_TYPE_CREATURE_DEHYDRATION,
	SCRIPT_OBJECT_PROPERTY_TYPE_CREATURE_FIGHT_HEALTH,
	SCRIPT_OBJECT_PROPERTY_TYPE_CREATURE_MIN_SIZE,
	SCRIPT_OBJECT_PROPERTY_TYPE_CREATURE_MAX_SIZE,
	SCRIPT_OBJECT_PROPERTY_TYPE_CREATURE_CAN_GO_THROUGH_VORTEX,
	SCRIPT_OBJECT_PROPERTY_TYPE_CREATURE_FIGHT_POWER,
	
	SCRIPT_OBJECT_PROPERTY_TYPE_LAST,
};

//------------------------------------------------------------------------------------------------------------------------
enum SCRIPT_OBJECT_TYPE
{
	SCRIPT_OBJECT_TYPE_NONE,
	SCRIPT_OBJECT_TYPE_MARKER,
	SCRIPT_OBJECT_TYPE_ABODE,
	SCRIPT_OBJECT_TYPE_FEATURE,
	SCRIPT_OBJECT_TYPE_VILLAGER,
	SCRIPT_OBJECT_TYPE_VILLAGER_CHILD,
	SCRIPT_OBJECT_TYPE_ANIMAL,
	SCRIPT_OBJECT_TYPE_REWARD,
	SCRIPT_OBJECT_TYPE_MOBILE_STATIC,
	SCRIPT_OBJECT_TYPE_TOWN,
	SCRIPT_OBJECT_TYPE_DANCE,
	SCRIPT_OBJECT_TYPE_FLOCK,
	SCRIPT_OBJECT_TYPE_CREATURE,
	SCRIPT_OBJECT_TYPE_DEAD_TREE,
	SCRIPT_OBJECT_TYPE_INFLUENCE_RING,
	SCRIPT_OBJECT_TYPE_WEATHER_THING,
	SCRIPT_OBJECT_TYPE_STORE,
	SCRIPT_OBJECT_TYPE_TIMER,
	SCRIPT_OBJECT_TYPE_CITADEL,
	SCRIPT_OBJECT_TYPE_WORSHIP_SITE,
	SCRIPT_OBJECT_TYPE_MOBILE_OBJECT,
	SCRIPT_OBJECT_TYPE_BIRD,
	SCRIPT_OBJECT_TYPE_TREE,
	SCRIPT_OBJECT_TYPE_VORTEX,
	SCRIPT_OBJECT_TYPE_SPELL_SEED,
	SCRIPT_OBJECT_TYPE_POO,
	SCRIPT_OBJECT_TYPE_WHALE,
	SCRIPT_OBJECT_TYPE_ARK,
	SCRIPT_OBJECT_TYPE_BALL,
	SCRIPT_OBJECT_TYPE_MIST,
	SCRIPT_OBJECT_TYPE_ONE_SHOT_SPELL,
	SCRIPT_OBJECT_TYPE_ONE_SHOT_SPELL_IN_HAND,
	SCRIPT_OBJECT_TYPE_PUZZLE_GAME,
	SCRIPT_OBJECT_TYPE_ROCK,
	SCRIPT_OBJECT_TYPE_TOTEM,
	SCRIPT_OBJECT_TYPE_FIELD,
	SCRIPT_OBJECT_TYPE_SPELL_DISPENSER,
	SCRIPT_OBJECT_TYPE_HIGHLIGHT,
	SCRIPT_OBJECT_TYPE_COMPUTER_PLAYER,
	SCRIPT_OBJECT_TYPE_SCAFFOLD,
	SCRIPT_OBJECT_TYPE_TOTEM_STATUE,
	SCRIPT_OBJECT_TYPE_ANIMATED_STATIC,
	SCRIPT_OBJECT_TYPE_SPECIAL_FIELD,
	SCRIPT_OBJECT_TYPE_DIE,
	SCRIPT_OBJECT_TYPE_CREATURE_ISLE_BUILDING,
	SCRIPT_OBJECT_TYPE_DOLPHIN,
	SCRIPT_OBJECT_TYPE_CANNON,
	SCRIPT_OBJECT_TYPE_DUMB_CREATURE,
	SCRIPT_OBJECT_TYPE_FLOWERS,
	SCRIPT_OBJECT_TYPE_FEMALE_CREATURE,
	SCRIPT_OBJECT_TYPE_LAST,
};

//------------------------------------------------------------------------------------------------------------------------
enum SCRIPT_BOOL
{
	SCRIPT_TRUE =  1,
	SCRIPT_FALSE =  0,
};

//------------------------------------------------------------------------------------------------------------------------
enum SCRIPT_INTERFACE_SET
{
	SCRIPT_INTERFACE_SET_NONE,
	SCRIPT_INTERFACE_SET_NO_INTERFACE,
	SCRIPT_INTERFACE_SET_INTERACTION,
	SCRIPT_INTERFACE_SET_PICK_UP,
	SCRIPT_INTERFACE_SET_NORMAL,
	SCRIPT_INTERFACE_ACTIVATE_GESTURES,
};

//------------------------------------------------------------------------------------------------------------------------
enum SCRIPT_AVI_SEQUENCE
{
	SCRIPT_AVI_SEQUENCE_NONE,
	SCRIPT_AVI_SEQUENCE_INTRO,
	SCRIPT_AVI_SEQUENCE_FALLING,
	SCRIPT_AVI_SEQUENCE_LAST,
};

//------------------------------------------------------------------------------------------------------------------------
enum SCRIPT_INTERFACE_LEVEL
{
	SCRIPT_INTERFACE_LEVEL_NORMAL,
	SCRIPT_INTERFACE_LEVEL_JUST_GRAB,
	SCRIPT_INTERFACE_LEVEL_JUST_GRAB_FAR_TO_CITADEL,
	SCRIPT_INTERFACE_LEVEL_JUST_ROTATE,
	SCRIPT_INTERFACE_LEVEL_JUST_DOUBLE_CLICK_AND_DRAG,
	SCRIPT_INTERFACE_LEVEL_JUST_ZOOM,
	SCRIPT_INTERFACE_LEVEL_JUST_ROTATE_INTERACT,
	SCRIPT_INTERFACE_LEVEL_JUST_ROTATE_INTERACT_AND_ZOOM,
	SCRIPT_INTERFACE_LEVEL_JUST_HAND_MOVE,
	SCRIPT_INTERFACE_LEVEL_JUST_9, // <- missing in CI
	SCRIPT_INTERFACE_LEVEL_JUST_ROTATE_AND_DRAG,
	SCRIPT_INTERFACE_LEVEL_JUST_PITCH,
	SCRIPT_INTERFACE_LEVEL_JUST_HAND_INTERACTION,
	SCRIPT_INTERFACE_LEVEL_JUST_GRAB_DOUBLE_CLICK_AND_ROTATE,
	SCRIPT_INTERFACE_LEVEL_JUST_GRAB_DOUBLE_CLICK_AND_ROTATE_AND_PITCH,
	SCRIPT_INTERFACE_LEVEL_JUST_GRAB_DOUBLE_CLICK_AND_ROTATE_PITCH_AND_ZOOM,
};

//------------------------------------------------------------------------------------------------------------------------
enum SCRIPT_COUNTDOWN_DRAW
{
	SCRIPT_COUNTDOWN_DRAW_NONE,
	SCRIPT_COUNTDOWN_DRAW_TOP_LEFT,
	SCRIPT_COUNTDOWN_DRAW_BOTTOM_LEFT,
	SCRIPT_COUNTDOWN_DRAW_TOP_RIGHT,
	SCRIPT_COUNTDOWN_DRAW_BOTTOM_RIGHT,
	SCRIPT_COUNTDOWN_DRAW_CENTRE,
	SCRIPT_COUNTDOWN_DRAW_TOP_CENTRE,
	SCRIPT_COUNTDOWN_DRAW_BOTTOM_CENTRE,
	SCRIPT_COUNTDOWN_DRAW_LAST,
};

//------------------------------------------------------------------------------------------------------------------------
enum SCRIPT_DEV_FUNCTION
{
	SCRIPT_DEV_FUNCTION_NONE,
	SCRIPT_DEV_FUNCTION_START_DEVELOPMENT_SCRIPTS,
	SCRIPT_DEV_FUNCTION_ROPE_LEASH_ENABLED,
	SCRIPT_DEV_FUNCTION_OTHER_LEASHES_ENABLED,
	SCRIPT_DEV_FUNCTION_LOAD_MY_CREATURE,
	SCRIPT_DEV_FUNCTION_RESET_ESCAPE_STATE,
	SCRIPT_DEV_FUNCTION_MY_CREATURE_POINT_OUT_HIGHLIGHT,
	SCRIPT_DEV_FUNCTION_CLEAR_INTERACTION_MAGNITUDE,
	SCRIPT_DEV_FUNCTION_MY_CREATURE_CAN_DIE,
	SCRIPT_DEV_FUNCTION_MY_CREATURE_CANNOT_DIE,
	SCRIPT_DEV_FUNCTION_CREATURE_HELP_ON,
	SCRIPT_DEV_FUNCTION_CREATURE_HELP_OFF,
	SCRIPT_DEV_FUNCTION_ENTER_SAVEGAMEROOM,

	SCRIPT_DEV_FUNCTION_LAST,
};

//------------------------------------------------------------------------------------------------------------------------

#endif //__SCRIPTENUMS_H__