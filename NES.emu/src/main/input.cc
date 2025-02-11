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

#include <emuframework/EmuApp.hh>
#include <emuframework/EmuInput.hh>
#include <imagine/util/math/math.hh>
#include "MainSystem.hh"
#include <fceu/fceu.h>

namespace EmuEx
{

enum
{
	nesKeyIdxUp = Controls::systemKeyMapStart,
	nesKeyIdxRight,
	nesKeyIdxDown,
	nesKeyIdxLeft,
	nesKeyIdxLeftUp,
	nesKeyIdxRightUp,
	nesKeyIdxRightDown,
	nesKeyIdxLeftDown,
	nesKeyIdxSelect,
	nesKeyIdxStart,
	nesKeyIdxA,
	nesKeyIdxB,
	nesKeyIdxATurbo,
	nesKeyIdxBTurbo,
	nesKeyIdxAB,
};

const char *EmuSystem::inputFaceBtnName = "A/B";
const char *EmuSystem::inputCenterBtnName = "Select/Start";
const int EmuSystem::inputFaceBtns = 2;
const int EmuSystem::inputCenterBtns = 2;
const int EmuSystem::maxPlayers = 4;
std::array<int, EmuSystem::MAX_FACE_BTNS> EmuSystem::vControllerImageMap{1, 0};

void NesSystem::connectNESInput(int port, ESI type)
{
	assert(GameInfo);
	if(type == SI_GAMEPAD)
	{
		//logMsg("gamepad to port %d", port);
		FCEUI_SetInput(port, SI_GAMEPAD, &padData, 0);
	}
	else if(type == SI_ZAPPER)
	{
		//logMsg("zapper to port %d", port);
		FCEUI_SetInput(port, SI_ZAPPER, &zapperData, 1);
	}
	else
	{
		FCEUI_SetInput(port, SI_NONE, 0, 0);
	}
}

VController::Map NesSystem::vControllerMap(int player)
{
	using namespace IG;
	unsigned playerMask = player << 8;
	VController::Map map{};
	map[VController::F_ELEM] = bit(1) | playerMask;
	map[VController::F_ELEM+1] = bit(0) | playerMask;

	map[VController::C_ELEM] = bit(2) | playerMask;
	map[VController::C_ELEM+1] = bit(3) | playerMask;

	map[VController::D_ELEM] = bit(4) | bit(6) | playerMask;
	map[VController::D_ELEM+1] = bit(4) | playerMask;
	map[VController::D_ELEM+2] = bit(4) | bit(7) | playerMask;
	map[VController::D_ELEM+3] = bit(6) | playerMask;
	map[VController::D_ELEM+5] = bit(7) | playerMask;
	map[VController::D_ELEM+6] = bit(5) | bit(6) | playerMask;
	map[VController::D_ELEM+7] = bit(5) | playerMask;
	map[VController::D_ELEM+8] = bit(5) | bit(7) | playerMask;
	return map;
}

static unsigned playerInputShift(int player)
{
	switch(player)
	{
		case 1: return 8;
		case 2: return 16;
		case 3: return 24;
	}
	return 0;
}

unsigned NesSystem::translateInputAction(unsigned input, bool &turbo)
{
	turbo = 0;
	assert(input >= nesKeyIdxUp);
	int player = (input - nesKeyIdxUp) / Controls::gamepadKeys;
	unsigned playerMask = player << 8;
	input -= Controls::gamepadKeys * player;
	switch(input)
	{
		case nesKeyIdxUp: return bit(4) | playerMask;
		case nesKeyIdxRight: return bit(7) | playerMask;
		case nesKeyIdxDown: return bit(5) | playerMask;
		case nesKeyIdxLeft: return bit(6) | playerMask;
		case nesKeyIdxLeftUp: return bit(6) | bit(4) | playerMask;
		case nesKeyIdxRightUp: return bit(7) | bit(4) | playerMask;
		case nesKeyIdxRightDown: return bit(7) | bit(5) | playerMask;
		case nesKeyIdxLeftDown: return bit(6) | bit(5) | playerMask;
		case nesKeyIdxSelect: return bit(2) | playerMask;
		case nesKeyIdxStart: return bit(3) | playerMask;
		case nesKeyIdxATurbo: turbo = 1; [[fallthrough]];
		case nesKeyIdxA: return bit(0) | playerMask;
		case nesKeyIdxBTurbo: turbo = 1; [[fallthrough]];
		case nesKeyIdxB: return bit(1) | playerMask;
		case nesKeyIdxAB: return bit(0) | bit(1) | playerMask;
		default: bug_unreachable("input == %d", input);
	}
	return 0;
}

void NesSystem::handleInputAction(EmuApp *, InputAction a)
{
	int player = a.key >> 8;
	auto key = a.key & 0xFF;
	bool isPushed = a.state == Input::Action::PUSHED;
	if(GameInfo->type == GIT_VSUNI) // TODO: make coin insert separate key
	{
		if(isPushed && key == IG::bit(3))
			FCEUI_VSUniCoin();
	}
	else if(GameInfo->inputfc == SIFC_HYPERSHOT)
	{
		if(auto hsKey = key & 0x3;
			hsKey)
		{
			hsKey = hsKey == 0x3 ? 0x3 : hsKey ^ 0x3; // swap the 2 bits
			auto hsPlayerInputShift = player == 1 ? 3 : 1;
			fcExtData = IG::setOrClearBits(fcExtData, hsKey << hsPlayerInputShift, isPushed);
		}
	}
	padData = IG::setOrClearBits(padData, key << playerInputShift(player), isPushed);
}

bool NesSystem::onPointerInputStart(const Input::MotionEvent &e, Input::DragTrackerState, IG::WindowRect gameRect)
{
	if(!usingZapper)
		return false;
	zapperData[2] = 0;
	if(gameRect.overlaps(e.pos()))
	{
		int xRel = e.pos().x - gameRect.x, yRel = e.pos().y - gameRect.y;
		int xNes = IG::remap(xRel, 0, gameRect.xSize(), 0, 256);
		int yNes = IG::remap(yRel, 0, gameRect.ySize(), 0, 224) + 8;
		logMsg("zapper pushed @ %d,%d, on NES %d,%d", e.pos().x, e.pos().y, xNes, yNes);
		zapperData[0] = xNes;
		zapperData[1] = yNes;
		zapperData[2] |= 0x1;
	}
	else // off-screen shot
	{
		zapperData[0] = 0;
		zapperData[1] = 0;
		zapperData[2] |= 0x2;
	}
	return true;
}

bool NesSystem::onPointerInputEnd(const Input::MotionEvent &, Input::DragTrackerState, IG::WindowRect)
{
	if(!usingZapper)
		return false;
	zapperData[2] = 0;
	return true;
}

void NesSystem::clearInputBuffers(EmuInputView &)
{
	IG::fill(zapperData);
	padData = {};
	fcExtData = {};
}

}

void GetMouseData(uint32 (&d)[3])
{
	// TODO
}
