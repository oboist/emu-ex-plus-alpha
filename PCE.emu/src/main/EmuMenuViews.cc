/*  This file is part of PCE.emu.

	PCE.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	PCE.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with PCE.emu.  If not, see <http://www.gnu.org/licenses/> */

#include <emuframework/OptionView.hh>
#include <emuframework/EmuSystemActionsView.hh>
#include <emuframework/EmuInput.hh>
#include "MainApp.hh"
#include <imagine/fs/FS.hh>
#include <imagine/util/format.hh>
#include <mednafen/pce_fast/vdc.h>

namespace EmuEx
{

template <class T>
using MainAppHelper = EmuAppHelper<T, MainApp>;

class ConsoleOptionView : public TableView, public MainAppHelper<ConsoleOptionView>
{
	BoolMenuItem sixButtonPad
	{
		"6-button Gamepad", &defaultFace(),
		(bool)system().option6BtnPad,
		[this](BoolMenuItem &item, View &, Input::Event e)
		{
			system().sessionOptionSet();
			system().option6BtnPad = item.flipBoolValue(*this);
			set6ButtonPadEnabled(app(), system().option6BtnPad);
		}
	};

	BoolMenuItem arcadeCard
	{
		"Arcade Card", &defaultFace(),
		(bool)system().optionArcadeCard,
		[this](BoolMenuItem &item, View &, Input::Event e)
		{
			system().sessionOptionSet();
			system().optionArcadeCard = item.flipBoolValue(*this);
			app().promptSystemReloadDueToSetOption(attachParams(), e);
		}
	};

	TextHeadingMenuItem videoHeading{"Video", &defaultBoldFace()};

	TextMenuItem visibleVideoLinesItem[5]
	{
		{"11+224", &defaultFace(), setVisibleVideoLinesDel(11, 234)},
		{"18+224", &defaultFace(), setVisibleVideoLinesDel(18, 241)},
		{"4+232",  &defaultFace(), setVisibleVideoLinesDel(4, 235)},
		{"3+239",  &defaultFace(), setVisibleVideoLinesDel(3, 241)},
		{"0+242",  &defaultFace(), setVisibleVideoLinesDel(0, 241)},
	};

	MultiChoiceMenuItem visibleVideoLines
	{
		"Visible Lines", &defaultFace(),
		[this]()
		{
			switch(system().visibleLines.first)
			{
				default: return 0;
				case 18: return 1;
				case 4: return 2;
				case 3: return 3;
				case 0: return 4;
			}
		}(),
		visibleVideoLinesItem
	};

	TextMenuItem::SelectDelegate setVisibleVideoLinesDel(uint8_t startLine, uint8_t endLine)
	{
		return [=, this]() { system().setVisibleLines({startLine, endLine}); };
	}

	std::array<MenuItem*, 4> menuItem
	{
		&sixButtonPad,
		&arcadeCard,
		&videoHeading,
		&visibleVideoLines,
	};

public:
	ConsoleOptionView(ViewAttachParams attach):
		TableView
		{
			"Console Options",
			attach,
			menuItem
		}
	{}
};

class CustomSystemActionsView : public EmuSystemActionsView
{
private:
	TextMenuItem options
	{
		"Console Options", &defaultFace(),
		[this](Input::Event e) { pushAndShow(makeView<ConsoleOptionView>(), e); }
	};

public:
	CustomSystemActionsView(ViewAttachParams attach): EmuSystemActionsView{attach, true}
	{
		item.emplace_back(&options);
		loadStandardItems();
	}
};

class CustomFilePathOptionView : public FilePathOptionView, public MainAppHelper<CustomFilePathOptionView>
{
	using MainAppHelper<CustomFilePathOptionView>::system;

	TextMenuItem sysCardPath
	{
		biosMenuEntryStr(appContext().fileUriDisplayName(system().sysCardPath)), &defaultFace(),
		[this](Input::Event e)
		{
			auto biosSelectMenu = makeViewWithName<BiosSelectMenu>("System Card", &system().sysCardPath,
				[this](std::string_view displayName)
				{
					logMsg("set bios %s", system().sysCardPath.data());
					sysCardPath.compile(biosMenuEntryStr(displayName), renderer(), projP);
				},
				hasHuCardExtension);
			pushAndShow(std::move(biosSelectMenu), e);
		}
	};

	std::string biosMenuEntryStr(std::string_view displayName) const
	{
		return fmt::format("System Card: {}", displayName);
	}

public:
	CustomFilePathOptionView(ViewAttachParams attach): FilePathOptionView{attach, true}
	{
		loadStockItems();
		item.emplace_back(&sysCardPath);
	}
};

class CustomVideoOptionView : public VideoOptionView, public MainAppHelper<CustomVideoOptionView>
{
	using  MainAppHelper<CustomVideoOptionView>::app;
	using  MainAppHelper<CustomVideoOptionView>::system;

	BoolMenuItem spriteLimit
	{
		"Sprite Limit", &defaultFace(),
		!system().noSpriteLimit,
		[this](BoolMenuItem &item) { system().setNoSpriteLimit(!item.flipBoolValue(*this)); }
	};

	TextMenuItem visibleVideoLinesItem[5]
	{
		{"11+224", &defaultFace(), setVisibleVideoLinesDel(11, 234)},
		{"18+224", &defaultFace(), setVisibleVideoLinesDel(18, 241)},
		{"4+232",  &defaultFace(), setVisibleVideoLinesDel(4, 235)},
		{"3+239",  &defaultFace(), setVisibleVideoLinesDel(3, 241)},
		{"0+242",  &defaultFace(), setVisibleVideoLinesDel(0, 241)},
	};

	MultiChoiceMenuItem visibleVideoLines
	{
		"Default Visible Lines", &defaultFace(),
		[this]()
		{
			switch(system().defaultVisibleLines.first)
			{
				default: return 0;
				case 18: return 1;
				case 4: return 2;
				case 3: return 3;
				case 0: return 4;
			}
		}(),
		visibleVideoLinesItem
	};

	TextMenuItem::SelectDelegate setVisibleVideoLinesDel(uint8_t startLine, uint8_t endLine)
	{
		return [=, this]() { system().defaultVisibleLines = {startLine, endLine}; };
	}

	BoolMenuItem correctLineAspect
	{
		"Correct Line Aspect Ratio", &defaultFace(),
		system().correctLineAspect,
		[this](BoolMenuItem &item)
		{
			system().correctLineAspect = item.flipBoolValue(*this);
			app().viewController().placeEmuViews();
		}
	};

public:
	CustomVideoOptionView(ViewAttachParams attach): VideoOptionView{attach, true}
	{
		loadStockItems();
		item.emplace_back(&systemSpecificHeading);
		item.emplace_back(&spriteLimit);
		item.emplace_back(&visibleVideoLines);
		item.emplace_back(&correctLineAspect);
	}
};

class CustomSystemOptionView : public SystemOptionView, public MainAppHelper<CustomSystemOptionView>
{
	using MainAppHelper<CustomSystemOptionView>::system;

	TextMenuItem cdSpeedItem[5]
	{
		{"1x", &defaultFace(), setCdSpeedDel(), 1},
		{"2x", &defaultFace(), setCdSpeedDel(), 2},
		{"4x", &defaultFace(), setCdSpeedDel(), 4},
		{"8x", &defaultFace(), setCdSpeedDel(), 8},
	};

	MultiChoiceMenuItem cdSpeed
	{
		"CD Access Speed", &defaultFace(),
		(MenuItem::Id)system().cdSpeed,
		cdSpeedItem
	};

	TextMenuItem::SelectDelegate setCdSpeedDel()
	{
		return [this](TextMenuItem &item) { system().setCdSpeed(item.id()); };
	}

public:
	CustomSystemOptionView(ViewAttachParams attach): SystemOptionView{attach, true}
	{
		loadStockItems();
		item.emplace_back(&cdSpeed);
	}
};

class CustomAudioOptionView : public AudioOptionView, public MainAppHelper<CustomAudioOptionView>
{
	using MainAppHelper<CustomAudioOptionView>::system;
	using MainAppHelper<CustomAudioOptionView>::app;

	TextHeadingMenuItem mixer{"Mixer", &defaultBoldFace()};

	struct VolumeTypeDesc
	{
		std::string_view name{};
		size_t idx{};
	};

	static constexpr VolumeTypeDesc desc(VolumeType type)
	{
		switch(type)
		{
			case VolumeType::CDDA: return {"CD-DA Volume", 0};
			case VolumeType::ADPCM: return {"ADPCM Volume", 1};
		}
		bug_unreachable("invalid VolumeType");
	}

	using VolumeChoiceItemArr = std::array<TextMenuItem, 3>;

	VolumeChoiceItemArr volumeLevelChoiceItems(VolumeType type)
	{
		return
		{
			TextMenuItem
			{
				"Default", &defaultFace(),
				[=, this]() { system().setVolume(type, 100); },
				100
			},
			TextMenuItem
			{
				"Off", &defaultFace(),
				[=, this]() { system().setVolume(type, 0); },
				0
			},
			TextMenuItem
			{
				"Custom Value", &defaultFace(),
				[=, this](Input::Event e)
				{
					app().pushAndShowNewCollectValueRangeInputView<int, 0, 200>(attachParams(), e, "Input 0 to 200", "",
						[=, this](EmuApp &, auto val)
						{
							system().setVolume(type, val);
							volumeLevel[desc(type).idx].setSelected((MenuItem::Id)val, *this);
							dismissPrevious();
							return true;
						});
					return false;
				}, MenuItem::DEFAULT_ID
			}
		};
	}

	std::array<VolumeChoiceItemArr, 2> volumeLevelItem
	{
		volumeLevelChoiceItems(VolumeType::CDDA),
		volumeLevelChoiceItems(VolumeType::ADPCM),
	};

	MultiChoiceMenuItem volumeLevelMenuItem(VolumeType type)
	{
		return
		{
			desc(type).name, &defaultFace(),
			[=, this](size_t idx, Gfx::Text &t)
			{
				t.setString(fmt::format("{}%", system().volume(type)));
				return true;
			},
			(MenuItem::Id)system().volume(type),
			volumeLevelItem[desc(type).idx]
		};
	}

	std::array<MultiChoiceMenuItem, 2> volumeLevel
	{
		volumeLevelMenuItem(VolumeType::CDDA),
		volumeLevelMenuItem(VolumeType::ADPCM),
	};

	BoolMenuItem adpcmFilter
	{
		"ADPCM Low-pass Filter", &defaultFace(),
		system().adpcmFilter,
		[this](BoolMenuItem &item) { system().setAdpcmFilter(item.flipBoolValue(*this)); }
	};

public:
	CustomAudioOptionView(ViewAttachParams attach): AudioOptionView{attach, true}
	{
		loadStockItems();
		item.emplace_back(&adpcmFilter);
		item.emplace_back(&mixer);
		item.emplace_back(&volumeLevel[0]);
		item.emplace_back(&volumeLevel[1]);
	}
};

std::unique_ptr<View> EmuApp::makeCustomView(ViewAttachParams attach, ViewID id)
{
	switch(id)
	{
		case ViewID::SYSTEM_ACTIONS: return std::make_unique<CustomSystemActionsView>(attach);
		case ViewID::AUDIO_OPTIONS: return std::make_unique<CustomAudioOptionView>(attach);
		case ViewID::VIDEO_OPTIONS: return std::make_unique<CustomVideoOptionView>(attach);
		case ViewID::SYSTEM_OPTIONS: return std::make_unique<CustomSystemOptionView>(attach);
		case ViewID::FILE_PATH_OPTIONS: return std::make_unique<CustomFilePathOptionView>(attach);
		default: return nullptr;
	}
}

}
