#pragma once

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
#include "MainSystem.hh"

namespace EmuEx
{

class WsApp final: public EmuApp
{
public:
	WsSystem wsSystem;

	WsApp(ApplicationInitParams initParams, ApplicationContext &ctx):
		EmuApp{initParams, ctx}, wsSystem{ctx}
	{
		setDefaultVControlsButtonSpacing(100);
		setDefaultVControlsButtonStagger(5);
	}

	auto &system() { return wsSystem;  }
	const auto &system() const { return wsSystem;  }
};

using MainApp = WsApp;

}
