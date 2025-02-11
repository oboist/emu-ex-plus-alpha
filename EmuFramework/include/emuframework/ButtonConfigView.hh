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
#include <imagine/gfx/GfxText.hh>
#include <imagine/gui/TableView.hh>
#include <imagine/gui/MenuItem.hh>
#include <imagine/input/config.hh>
#include <string>
#include <string_view>

namespace EmuEx
{

using namespace IG;
class InputManagerView;
class InputDeviceConfig;
struct KeyCategory;

class ButtonConfigSetView : public View, public EmuAppHelper<ButtonConfigSetView>
{
public:
	using SetDelegate = DelegateFunc<void (const Input::KeyEvent &)>;

	ButtonConfigSetView(ViewAttachParams attach, InputManagerView &rootIMView,
		Input::Device &dev, std::string_view actionName, SetDelegate onSet);
	void place() final;
	bool inputEvent(const Input::Event &) final;
	void draw(Gfx::RendererCommands &__restrict__) final;
	void onAddedToController(ViewController *, const Input::Event &) final;

private:
	#ifdef CONFIG_INPUT_POINTING_DEVICES
	IG::WindowRect unbindB{}, cancelB{};
	#endif
	Gfx::Text text{};
	#ifdef CONFIG_INPUT_POINTING_DEVICES
	Gfx::Text unbind{}, cancel{};
	#endif
	SetDelegate onSetD{};
	const Input::Device &dev;
	const Input::Device *savedDev{};
	InputManagerView &rootIMView;
	std::string actionStr{};

	void initPointerUI();
	bool pointerUIIsInit();
};

class ButtonConfigView : public TableView, public EmuAppHelper<ButtonConfigView>
{
private:
	struct BtnConfigMenuItem : public DualTextMenuItem
	{
		using DualTextMenuItem::DualTextMenuItem;
		void draw(Gfx::RendererCommands &__restrict__ , float xPos, float yPos, float xSize, float ySize,
			float xIndent, _2DOrigin align, const Gfx::ProjectionPlane &, Gfx::Color) const final;
	};

	InputManagerView &rootIMView;
	TextMenuItem reset{};
	std::unique_ptr<BtnConfigMenuItem[]> btn{};
	const KeyCategory *cat{};
	InputDeviceConfig *devConf{};
	Input::Time leftKeyPushTime{};

	void onSet(Input::Key mapKey, int keyToSet);
	static std::string makeKeyNameStr(Input::Key key, std::string_view name);

public:
	ButtonConfigView(ViewAttachParams attach, InputManagerView &rootIMView, const KeyCategory &cat, InputDeviceConfig &devConf);
	bool inputEvent(const Input::Event &) final;
};

}
