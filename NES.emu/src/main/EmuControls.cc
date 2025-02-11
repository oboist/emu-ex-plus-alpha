/*  This file is part of NES.emu.

	NES.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	NES.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with NES.emu.  If not, see <http://www.gnu.org/licenses/> */

#include <imagine/util/preprocessor/repeat.h>
#include <emuframework/EmuInput.hh>

namespace EmuEx::Controls
{

extern const int gamepadKeys = 15;
const int systemTotalKeys = gameActionKeys + gamepadKeys*4;

void transposeKeysForPlayer(KeyConfig::KeyArray &key, int player)
{
	genericMultiplayerTranspose(key, player, 1);
}

constexpr std::array<const std::string_view, gamepadKeys> gamepadName
{
	"Up",
	"Right",
	"Down",
	"Left",
	"Left+Up",
	"Right+Up",
	"Right+Down",
	"Left+Down",
	"Select",
	"Start",
	"A",
	"B",
	"Turbo A",
	"Turbo B",
	"A+B"
};

constexpr int gamepadKeyOffset = gameActionKeys;
constexpr int gamepad2KeyOffset = gamepadKeyOffset + gamepadKeys;
constexpr int gamepad3KeyOffset = gamepad2KeyOffset + gamepadKeys;
constexpr int gamepad4KeyOffset = gamepad3KeyOffset + gamepadKeys;

constexpr KeyCategory category[]
{
	EMU_CONTROLS_IN_GAME_ACTIONS_CATEGORY_INIT,
	{"Set Gamepad Keys", gamepadName, gamepadKeyOffset},
	{"Set Gamepad 2 Keys", gamepadName, gamepad2KeyOffset, 1},
	{"Set Gamepad 3 Keys", gamepadName, gamepad3KeyOffset, 1},
	{"Set Gamepad 4 Keys", gamepadName, gamepad4KeyOffset, 1}
};

std::span<const KeyCategory> categories() { return category; }

const KeyConfig defaultKeyProfile[] =
{
	{
		Map::SYSTEM,
		{"PC Keyboard"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_GENERIC_KB_PROFILE_INIT,

			Keycode::UP,
			Keycode::RIGHT,
			Keycode::DOWN,
			Keycode::LEFT,
			0, 0, 0, 0,
			Keycode::SPACE,
			Keycode::ENTER,
			Keycode::X,
			Keycode::Z,
			Keycode::S,
			Keycode::A,
		}
	},
	#ifdef CONFIG_INPUT_GAMEPAD_DEVICES
	{
		Map::SYSTEM,
		DeviceSubtype::GENERIC_GAMEPAD,
		{"Generic Gamepad"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_ANDROID_GENERIC_GAMEPAD_PROFILE_INIT,

			Keycode::UP,
			Keycode::RIGHT,
			Keycode::DOWN,
			Keycode::LEFT,
			0, 0, 0, 0,
			Keycode::GAME_SELECT,
			Keycode::GAME_START,
			Keycode::GAME_A,
			Keycode::GAME_X,
			Keycode::GAME_B,
			Keycode::GAME_Y,
		}
	},
	{
		Map::SYSTEM,
		DeviceSubtype::PS3_CONTROLLER,
		{"PS3 Controller"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_ANDROID_PS3_GAMEPAD_PROFILE_INIT,

			Keycode::PS3::UP,
			Keycode::PS3::RIGHT,
			Keycode::PS3::DOWN,
			Keycode::PS3::LEFT,
			0, 0, 0, 0,
			Keycode::PS3::SELECT,
			Keycode::PS3::START,
			Keycode::PS3::CROSS,
			Keycode::PS3::SQUARE,
			Keycode::PS3::CIRCLE,
			Keycode::PS3::TRIANGLE,
		}
	},
	{
		Map::SYSTEM,
		DeviceSubtype::OUYA_CONTROLLER,
		{"OUYA Controller"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_OUYA_PROFILE_INIT,

			Keycode::Ouya::UP,
			Keycode::Ouya::RIGHT,
			Keycode::Ouya::DOWN,
			Keycode::Ouya::LEFT,
			0, 0, 0, 0,
			Keycode::Ouya::L3,
			Keycode::Ouya::R3,
			Keycode::Ouya::O,
			Keycode::Ouya::U,
			Keycode::Ouya::A,
			Keycode::Ouya::Y,
		}
	},
	{
		Map::SYSTEM,
		DeviceSubtype::NVIDIA_SHIELD,
		{"NVidia Shield"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_NVIDIA_SHIELD_PROFILE_INIT,

			Keycode::UP,
			Keycode::RIGHT,
			Keycode::DOWN,
			Keycode::LEFT,
			0, 0, 0, 0,
			Keycode::GAME_LEFT_THUMB,
			Keycode::GAME_START,
			Keycode::GAME_A,
			Keycode::GAME_X,
			Keycode::GAME_B,
			Keycode::GAME_Y,
		}
	},
	{
		Map::SYSTEM,
		DeviceSubtype::_8BITDO_SF30_PRO,
		{"8Bitdo SF30 Pro"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_8BITDO_SF30_PRO_PROFILE_INIT,

			Keycode::UP,
			Keycode::RIGHT,
			Keycode::DOWN,
			Keycode::LEFT,
			0, 0, 0, 0,
			Keycode::GAME_SELECT,
			Keycode::GAME_START,
			Keycode::GAME_B,
			Keycode::GAME_Y,
		}
	},
	{
		Map::SYSTEM,
		DeviceSubtype::_8BITDO_SN30_PRO_PLUS,
		{"8BitDo SN30 Pro+"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_8BITDO_SN30_PRO_PLUS_PROFILE_INIT,

			Keycode::UP,
			Keycode::RIGHT,
			Keycode::DOWN,
			Keycode::LEFT,
			0, 0, 0, 0,
			Keycode::GAME_SELECT,
			Keycode::GAME_START,
			Keycode::GAME_B,
			Keycode::GAME_Y,
		}
	},
	#endif
	#if defined(__ANDROID__) && __ARM_ARCH == 7
	{
		Map::SYSTEM,
		DeviceSubtype::XPERIA_PLAY,
		{"Xperia Play"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_ANDROID_NAV_PROFILE_INIT,

			Keycode::XperiaPlay::UP,
			Keycode::XperiaPlay::RIGHT,
			Keycode::XperiaPlay::DOWN,
			Keycode::XperiaPlay::LEFT,
			0, 0, 0, 0,
			Keycode::XperiaPlay::SELECT,
			Keycode::XperiaPlay::START,
			Keycode::XperiaPlay::CROSS,
			Keycode::XperiaPlay::SQUARE,
			Keycode::XperiaPlay::CIRCLE,
			Keycode::XperiaPlay::TRIANGLE,
		}
	},
	{
		Map::SYSTEM,
		DeviceSubtype::MOTO_DROID_KEYBOARD,
		{"Droid/Milestone Keyboard"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_ANDROID_NAV_PROFILE_INIT,

			Keycode::UP,
			Keycode::RIGHT,
			Keycode::DOWN,
			Keycode::LEFT,
			0, 0, 0, 0,
			Keycode::SPACE,
			Keycode::ENTER,
			Keycode::C,
			Keycode::X,
			Keycode::F,
			Keycode::D,
		}
	},
	#endif
	#ifdef CONFIG_MACHINE_PANDORA
	{
		Map::SYSTEM,
		DeviceSubtype::PANDORA_HANDHELD,
		{"Default Pandora"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_PANDORA_PROFILE_INIT,

			Keycode::Pandora::UP,
			Keycode::Pandora::RIGHT,
			Keycode::Pandora::DOWN,
			Keycode::Pandora::LEFT,
			0, 0, 0, 0,
			Keycode::Pandora::SELECT,
			Keycode::Pandora::START,
			Keycode::Pandora::X,
			Keycode::Pandora::A,
			Keycode::Pandora::B,
			Keycode::Pandora::Y,
		}
	},
	#endif
};

const unsigned defaultKeyProfiles = std::size(defaultKeyProfile);

#ifdef CONFIG_INPUT_APPLE_GAME_CONTROLLER

const KeyConfig defaultAppleGCProfile[] =
{
	{
		Map::APPLE_GAME_CONTROLLER,
		{"Default"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_APPLEGC_PROFILE_INIT,
			AppleGC::UP,
			AppleGC::RIGHT,
			AppleGC::DOWN,
			AppleGC::LEFT,
			0, 0, 0, 0,
			AppleGC::RSTICK_LEFT,
			AppleGC::RSTICK_RIGHT,
			AppleGC::A,
			AppleGC::X,
			AppleGC::B,
			AppleGC::Y,
		}
	},
};

const unsigned defaultAppleGCProfiles = std::size(defaultAppleGCProfile);

#endif

// Wiimote

const KeyConfig defaultWiimoteProfile[] =
{
	{
		Map::WIIMOTE,
		{"Default"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_WIIMOTE_PROFILE_INIT,

			Wiimote::UP,
			Wiimote::RIGHT,
			Wiimote::DOWN,
			Wiimote::LEFT,
			0, 0, 0, 0,
			Wiimote::MINUS,
			Wiimote::PLUS,
			Wiimote::_2,
			Wiimote::_1,
		}
	},
};

const unsigned defaultWiimoteProfiles = std::size(defaultWiimoteProfile);

const KeyConfig defaultWiiCCProfile[] =
{
	{
		Map::WII_CC,
		{"Default"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_WII_CC_PROFILE_INIT,

			WiiCC::UP,
			WiiCC::RIGHT,
			WiiCC::DOWN,
			WiiCC::LEFT,
			0, 0, 0, 0,
			WiiCC::MINUS,
			WiiCC::PLUS,
			WiiCC::B,
			WiiCC::Y,
			WiiCC::A,
			WiiCC::X,
		}
	},
};

const unsigned defaultWiiCCProfiles = std::size(defaultWiiCCProfile);

// iControlPad

const KeyConfig defaultIControlPadProfile[] =
{
	{
		Map::ICONTROLPAD,
		{"Default"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_ICP_NUBS_PROFILE_INIT,

			iControlPad::UP,
			iControlPad::RIGHT,
			iControlPad::DOWN,
			iControlPad::LEFT,
			0, 0, 0, 0,
			iControlPad::SELECT,
			iControlPad::START,
			iControlPad::X,
			iControlPad::A,
			iControlPad::B,
			iControlPad::Y,
		}
	},
};

const unsigned defaultIControlPadProfiles = std::size(defaultIControlPadProfile);

// iCade

const KeyConfig defaultICadeProfile[] =
{
	{
		Map::ICADE,
		{"Default"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_UNBINDED_PROFILE_INIT,

			ICade::UP,
			ICade::RIGHT,
			ICade::DOWN,
			ICade::LEFT,
			0, 0, 0, 0,
			ICade::SELECT,
			ICade::START,
			ICade::A,
			ICade::X,
			ICade::B,
			ICade::Y,
		}
	},
};

const unsigned defaultICadeProfiles = std::size(defaultICadeProfile);

// Zeemote

const KeyConfig defaultZeemoteProfile[] =
{
	{
		Map::ZEEMOTE,
		{"Default"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_UNBINDED_PROFILE_INIT,

			Zeemote::UP,
			Zeemote::RIGHT,
			Zeemote::DOWN,
			Zeemote::LEFT,
			0, 0, 0, 0,
			Zeemote::C,
			Zeemote::POWER,
			Zeemote::B,
			Zeemote::A,
		}
	},
};

const unsigned defaultZeemoteProfiles = std::size(defaultZeemoteProfile);

// PS3

const KeyConfig defaultPS3Profile[] =
{
	{
		Map::PS3PAD,
		{"Default"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_GENERIC_PS3PAD_PROFILE_INIT,

			PS3::UP,
			PS3::RIGHT,
			PS3::DOWN,
			PS3::LEFT,
			0, 0, 0, 0,
			PS3::SELECT,
			PS3::START,
			PS3::CROSS,
			PS3::SQUARE,
			PS3::CIRCLE,
			PS3::TRIANGLE,
		}
	},
};

const unsigned defaultPS3Profiles = std::size(defaultPS3Profile);

};
