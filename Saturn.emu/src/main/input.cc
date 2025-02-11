/*  This file is part of Saturn.emu.

	Saturn.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Saturn.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Saturn.emu.  If not, see <http://www.gnu.org/licenses/> */

#include <emuframework/EmuApp.hh>
#include <emuframework/EmuInput.hh>
#include "MainSystem.hh"

namespace EmuEx
{

enum
{
	ssKeyIdxUp = Controls::systemKeyMapStart,
	ssKeyIdxRight,
	ssKeyIdxDown,
	ssKeyIdxLeft,
	ssKeyIdxLeftUp,
	ssKeyIdxRightUp,
	ssKeyIdxRightDown,
	ssKeyIdxLeftDown,
	ssKeyIdxStart,
	ssKeyIdxA,
	ssKeyIdxB,
	ssKeyIdxC,
	ssKeyIdxX,
	ssKeyIdxY,
	ssKeyIdxZ,
	ssKeyIdxL,
	ssKeyIdxR,
	ssKeyIdxATurbo,
	ssKeyIdxBTurbo,
	ssKeyIdxCTurbo,
	ssKeyIdxXTurbo,
	ssKeyIdxYTurbo,
	ssKeyIdxZTurbo,
	ssKeyIdxLastGamepad = ssKeyIdxZTurbo
};

const char *EmuSystem::inputFaceBtnName = "A-Z";
const char *EmuSystem::inputCenterBtnName = "Start";
const int EmuSystem::inputFaceBtns = 8;
const int EmuSystem::inputCenterBtns = 1;
int EmuSystem::inputLTriggerIndex = 6;
int EmuSystem::inputRTriggerIndex = 7;
const int EmuSystem::maxPlayers = 2;

VController::Map SaturnSystem::vControllerMap(int player)
{
	unsigned playerOffset = player ? Controls::gamepadKeys : 0;
	VController::Map map{};
	map[VController::F_ELEM] = ssKeyIdxA + playerOffset;
	map[VController::F_ELEM+1] = ssKeyIdxB + playerOffset;
	map[VController::F_ELEM+2] = ssKeyIdxC + playerOffset;
	map[VController::F_ELEM+3] = ssKeyIdxX + playerOffset;
	map[VController::F_ELEM+4] = ssKeyIdxY + playerOffset;
	map[VController::F_ELEM+5] = ssKeyIdxZ + playerOffset;
	map[VController::F_ELEM+6] = ssKeyIdxL + playerOffset;
	map[VController::F_ELEM+7] = ssKeyIdxR + playerOffset;

	map[VController::C_ELEM] = ssKeyIdxStart + playerOffset;

	map[VController::D_ELEM] = ssKeyIdxLeftUp + playerOffset;
	map[VController::D_ELEM+1] = ssKeyIdxUp + playerOffset;
	map[VController::D_ELEM+2] = ssKeyIdxRightUp + playerOffset;
	map[VController::D_ELEM+3] = ssKeyIdxLeft + playerOffset;
	map[VController::D_ELEM+5] = ssKeyIdxRight + playerOffset;
	map[VController::D_ELEM+6] = ssKeyIdxLeftDown + playerOffset;
	map[VController::D_ELEM+7] = ssKeyIdxDown + playerOffset;
	map[VController::D_ELEM+8] = ssKeyIdxRightDown + playerOffset;
	return map;
}

unsigned SaturnSystem::translateInputAction(unsigned input, bool &turbo)
{
	turbo = 0;
	switch(input)
	{
		case ssKeyIdxXTurbo:
		case ssKeyIdxYTurbo:
		case ssKeyIdxZTurbo:
		case ssKeyIdxATurbo:
		case ssKeyIdxBTurbo:
		case ssKeyIdxCTurbo:
		case ssKeyIdxXTurbo + Controls::gamepadKeys:
		case ssKeyIdxYTurbo + Controls::gamepadKeys:
		case ssKeyIdxZTurbo + Controls::gamepadKeys:
		case ssKeyIdxATurbo + Controls::gamepadKeys:
		case ssKeyIdxBTurbo + Controls::gamepadKeys:
		case ssKeyIdxCTurbo + Controls::gamepadKeys:
			turbo = 1; [[fallthrough]];
		default: return input;
	}
}

void SaturnSystem::handleInputAction(EmuApp *, InputAction a)
{
	unsigned player = 0;
	if(a.key > ssKeyIdxLastGamepad)
	{
		player = 1;
		a.key -= Controls::gamepadKeys;
	}
	PerPad_struct *p = (player == 1) ? pad[1] : pad[0];
	bool pushed = a.state == Input::Action::PUSHED;
	switch(a.key)
	{
		bcase ssKeyIdxUp: if(pushed) PerPadUpPressed(p); else PerPadUpReleased(p);
		bcase ssKeyIdxRight: if(pushed) PerPadRightPressed(p); else PerPadRightReleased(p);
		bcase ssKeyIdxDown: if(pushed) PerPadDownPressed(p); else PerPadDownReleased(p);
		bcase ssKeyIdxLeft: if(pushed) PerPadLeftPressed(p); else PerPadLeftReleased(p);
		bcase ssKeyIdxLeftUp: if(pushed) { PerPadLeftPressed(p); PerPadUpPressed(p); }
			else { PerPadLeftReleased(p); PerPadUpReleased(p); }
		bcase ssKeyIdxRightUp: if(pushed) { PerPadRightPressed(p); PerPadUpPressed(p); }
			else { PerPadRightReleased(p); PerPadUpReleased(p); }
		bcase ssKeyIdxRightDown: if(pushed) { PerPadRightPressed(p); PerPadDownPressed(p); }
			else { PerPadRightReleased(p); PerPadDownReleased(p); }
		bcase ssKeyIdxLeftDown: if(pushed) { PerPadLeftPressed(p); PerPadDownPressed(p); }
			else { PerPadLeftReleased(p); PerPadDownReleased(p); }
		bcase ssKeyIdxStart: if(pushed) PerPadStartPressed(p); else PerPadStartReleased(p);
		bcase ssKeyIdxXTurbo:
		case ssKeyIdxX: if(pushed) PerPadXPressed(p); else PerPadXReleased(p);
		bcase ssKeyIdxYTurbo:
		case ssKeyIdxY: if(pushed) PerPadYPressed(p); else PerPadYReleased(p);
		bcase ssKeyIdxZTurbo:
		case ssKeyIdxZ: if(pushed) PerPadZPressed(p); else PerPadZReleased(p);
		bcase ssKeyIdxATurbo:
		case ssKeyIdxA: if(pushed) PerPadAPressed(p); else PerPadAReleased(p);
		bcase ssKeyIdxBTurbo:
		case ssKeyIdxB: if(pushed) PerPadBPressed(p); else PerPadBReleased(p);
		bcase ssKeyIdxCTurbo:
		case ssKeyIdxC: if(pushed) PerPadCPressed(p); else PerPadCReleased(p);
		bcase ssKeyIdxL: if(pushed) PerPadLTriggerPressed(p); else PerPadLTriggerReleased(p);
		bcase ssKeyIdxR: if(pushed) PerPadRTriggerPressed(p); else PerPadRTriggerReleased(p);
		bdefault: bug_unreachable("input == %d", a.key);
	}
}

void SaturnSystem::clearInputBuffers(EmuInputView &)
{
	PerPortReset();
	pad[0] = PerPadAdd(&PORTDATA1);
	pad[1] = PerPadAdd(&PORTDATA2);
}

}
