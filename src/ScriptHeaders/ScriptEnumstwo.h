/******************************************************************************
 * Copyright (c) 2018-2024 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *******************************************************************************/

#pragma once

//------------------------------------------------------------------------------------------------------------------------
enum SCRIPT_JC_SPECIAL
{
	SJC_CREATE_FIRE_BALL,
	SJC_CAMERA_FOLLOW_FIRE_BALL,
	SJC_CANCEL_CAMERA_FOLLOW_FIRE_BALL,
	SJC_DANS_LES_NUAGES,
	SJC_CREATE_HAND_WITH_KID,
	SJC_ACTIVATED_HAND,
	SJC_IL_ETAIT_UN_PETIT_NAVIRE,
	SJC_I_AM_THE_LITTLE_BOY,
	SJC_BLEND_ANGLE,
	SJC_PREPARE_ROTATE180,
	SJC_PREPARE_ROTATE_LEFT_90,
	SJC_PREPARE_ROTATE_RIGHT_90,
	SJC_FADE_IN_HAND,
	SJC_START_HAND_WITH_BOY,
	SJC_SWITCH_ON_BOOKMARK,
	SJC_SWITCH_OFF_BOOKMARK,
	SJC_PREPARE_TURN_LEFT,
	SJC_PREPARE_TURN_RIGHT,
	SJC_CLEAN_INTRO,
	SJC_ALWAYS_VISIBLE,
	SJC_CI_BOAT_ARRIVAL,
	SJC_CI_BOAT_ARRIVAL_END,
	SJC_RAISE_TOTEM,

	SJC_START_EYE,
	SJC_END_EYE,
	SJC_ZOOM_EYE,

	SJC_CI_BOAT_LAUNCH,
	SJC_CI_BOAT_LAUNCH_END,

	SJC_CI_BOAT_CRASH,
	SJC_CI_BOAT_CRASH2,
	SJC_CI_BOAT_CRASH_END,

	SJC_SWITCH_ON_INFLUENCE,
	SJC_SWITCH_OFF_INFLUENCE,
};

//------------------------------------------------------------------------------------------------------------------------
enum CREATURE_TATTOO_TYPE
{
	CREATURE_TATTOO_TYPE_NONE,
	CREATURE_TATTOO_TYPE_THE_BROTHERHOOD,
	CREATURE_TATTOO_TYPE_SUPER_GODS,
	CREATURE_TATTOO_TYPE_FIGHTER,
	CREATURE_TATTOO_TYPE_FIGHTER_ALWAYS,
	CREATURE_TATTOO_TYPE_LAST,
};

//------------------------------------------------------------------------------------------------------------------------
enum ENUM_SPOT_VISUAL_ALEX
{
	SPOT_VISUAL_ALEX_HULAHOOP,
	SPOT_VISUAL_ALEX_FENCE,
	SPOT_VISUAL_ALEX_LAST,
};
