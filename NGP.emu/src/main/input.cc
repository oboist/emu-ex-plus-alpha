/*  This file is part of NGP.emu.

	NGP.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	NGP.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with NGP.emu.  If not, see <http://www.gnu.org/licenses/> */

#include <emuframework/EmuApp.hh>
#include <emuframework/EmuInput.hh>
#include "MainSystem.hh"

namespace EmuEx
{

enum
{
	ngpKeyIdxUp = Controls::systemKeyMapStart,
	ngpKeyIdxRight,
	ngpKeyIdxDown,
	ngpKeyIdxLeft,
	ngpKeyIdxLeftUp,
	ngpKeyIdxRightUp,
	ngpKeyIdxRightDown,
	ngpKeyIdxLeftDown,
	ngpKeyIdxOption,
	ngpKeyIdxA,
	ngpKeyIdxB,
	ngpKeyIdxATurbo,
	ngpKeyIdxBTurbo
};

const char *EmuSystem::inputFaceBtnName = "A/B";
const char *EmuSystem::inputCenterBtnName = "Option";
const int EmuSystem::inputFaceBtns = 2;
const int EmuSystem::inputCenterBtns = 1;
const int EmuSystem::maxPlayers = 1;

constexpr unsigned ctrlUpBit = 0x01, ctrlDownBit = 0x02, ctrlLeftBit = 0x04, ctrlRightBit = 0x08,
		ctrlABit = 0x10, ctrlBBit = 0x20, ctrlOptionBit = 0x40;

VController::Map NgpSystem::vControllerMap(int player)
{
	VController::Map map{};
	map[VController::F_ELEM] = ctrlABit;
	map[VController::F_ELEM+1] = ctrlBBit;

	map[VController::C_ELEM] = ctrlOptionBit;

	map[VController::D_ELEM] = ctrlUpBit | ctrlLeftBit;
	map[VController::D_ELEM+1] = ctrlUpBit;
	map[VController::D_ELEM+2] = ctrlUpBit | ctrlRightBit;
	map[VController::D_ELEM+3] = ctrlLeftBit;
	map[VController::D_ELEM+5] = ctrlRightBit;
	map[VController::D_ELEM+6] = ctrlDownBit | ctrlLeftBit;
	map[VController::D_ELEM+7] = ctrlDownBit;
	map[VController::D_ELEM+8] = ctrlDownBit | ctrlRightBit;
	return map;
}

unsigned NgpSystem::translateInputAction(unsigned input, bool &turbo)
{
	turbo = 0;
	switch(input)
	{
		case ngpKeyIdxUp: return ctrlUpBit;
		case ngpKeyIdxRight: return ctrlRightBit;
		case ngpKeyIdxDown: return ctrlDownBit;
		case ngpKeyIdxLeft: return ctrlLeftBit;
		case ngpKeyIdxLeftUp: return ctrlLeftBit | ctrlUpBit;
		case ngpKeyIdxRightUp: return ctrlRightBit | ctrlUpBit;
		case ngpKeyIdxRightDown: return ctrlRightBit | ctrlDownBit;
		case ngpKeyIdxLeftDown: return ctrlLeftBit | ctrlDownBit;
		case ngpKeyIdxOption: return ctrlOptionBit;
		case ngpKeyIdxATurbo: turbo = 1; [[fallthrough]];
		case ngpKeyIdxA: return ctrlABit;
		case ngpKeyIdxBTurbo: turbo = 1; [[fallthrough]];
		case ngpKeyIdxB: return ctrlBBit;
		default: bug_unreachable("input == %d", input);
	}
	return 0;
}

void NgpSystem::handleInputAction(EmuApp *, InputAction a)
{
	inputBuff = IG::setOrClearBits(inputBuff, (uint8_t)a.key, a.state == Input::Action::PUSHED);
}

void NgpSystem::clearInputBuffers(EmuInputView &)
{
	inputBuff = {};
}

}
