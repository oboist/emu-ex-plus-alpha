#pragma once

/*  This file is part of EmuFramework.

	Imagine is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Imagine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with EmuFramework.  If not, see <http://www.gnu.org/licenses/> */

#include <emuframework/EmuAppHelper.hh>
#include <imagine/gui/View.hh>

namespace EmuEx
{

using namespace IG;
class VController;
class EmuApp;
class EmuVideoLayer;
class EmuViewController;

class EmuInputView : public View, public EmuAppHelper<EmuInputView>
{
public:
	EmuInputView();
	EmuInputView(ViewAttachParams attach, VController &vCtrl, EmuVideoLayer &videoLayer);
	void place() final;
	void draw(Gfx::RendererCommands &__restrict__) final;
	bool inputEvent(const Input::Event &) final;
	void resetInput();
	VController *activeVController() const { return vController; }

private:
	VController *vController{};
	EmuVideoLayer *videoLayer{};
	bool ffToggleActive{};

	void updateRunSpeed();
};

}
