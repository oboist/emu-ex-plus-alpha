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

#include <emuframework/EmuSystem.hh>
#include <emuframework/EmuAppHelper.hh>
#include <imagine/gui/FSPicker.hh>

namespace EmuEx
{

using namespace IG;

class EmuFilePicker : public FSPicker, public EmuAppHelper<EmuFilePicker>
{
public:
	EmuFilePicker(ViewAttachParams, FSPicker::Mode, EmuSystem::NameFilterFunc, const Input::Event &, bool includeArchives = true);
	EmuFilePicker(ViewAttachParams, EmuApp &, FSPicker::Mode, EmuSystem::NameFilterFunc, const Input::Event &, bool includeArchives = true);
	static std::unique_ptr<EmuFilePicker> makeForBenchmarking(ViewAttachParams, const Input::Event &, bool singleDir = false);
	static std::unique_ptr<EmuFilePicker> makeForLoading(ViewAttachParams, const Input::Event &, bool singleDir = false,
		EmuSystemCreateParams params = {});
	static std::unique_ptr<EmuFilePicker> makeForMediaChange(ViewAttachParams, const Input::Event &,
		EmuSystem::NameFilterFunc filter, FSPicker::OnSelectPathDelegate, bool singleDir = false);
	static std::unique_ptr<EmuFilePicker> makeForMediaCreation(ViewAttachParams, const Input::Event &);
	static std::unique_ptr<EmuFilePicker> makeForMediaCreation(ViewAttachParams);
};

}
