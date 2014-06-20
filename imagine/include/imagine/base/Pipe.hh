#pragma once

/*  This file is part of Imagine.

	Imagine is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Imagine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Imagine.  If not, see <http://www.gnu.org/licenses/> */

#include <imagine/engine-globals.h>
#include <imagine/util/DelegateFunc.hh>
#include <imagine/base/EventLoopFileSource.hh>

namespace Base
{

class Pipe
{
private:
	int msgPipe[2] {-1, -1};
	EventLoopFileSource fdSrc;

public:
	using Delegate = DelegateFunc<int (Pipe &pipe)>;
	Delegate del;

	void init(Delegate del);
	void deinit();
	bool read(void *data, uint size);
	bool write(const void *data, uint size);
	bool hasData();
};

}