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

#include <vector>
#include <imagine/util/string/CStringView.hh>

namespace IG
{
class ApplicationContext;
}

namespace IG::FS
{
class PathString;
}

namespace EmuEx
{

class EmuSystem;

std::vector<FS::PathString> subDirectoryStrings(IG::ApplicationContext, IG::CStringView path);
void flattenSubDirectories(IG::ApplicationContext, const std::vector<FS::PathString> &subDirs, IG::CStringView outPath);
void updateLegacySavePathOnStoragePath(IG::ApplicationContext, EmuSystem &);
bool hasWriteAccessToDir(IG::CStringView path);

}
