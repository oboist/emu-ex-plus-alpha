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

#include <imagine/config/defs.hh>

#ifdef CONFIG_GFX_OPENGL
#include <imagine/gfx/opengl/GLRendererTask.hh>
#endif

#include <imagine/gfx/defs.hh>
#include <imagine/gfx/SyncFence.hh>
#include <imagine/base/baseDefs.hh>
#include <imagine/util/utility.h>
#include <concepts>

namespace IG::Gfx
{

class RendererTask : public RendererTaskImpl
{
public:
	using RendererTaskImpl::RendererTaskImpl;
	void updateDrawableForSurfaceChange(Window &, WindowSurfaceChange);
	void setDefaultViewport(Window &win, Viewport v);
	void releaseShaderCompiler();
	void flush();
	void setDebugOutput(bool on);
	Renderer &renderer() const;
	explicit operator bool() const;

	// Run a delegate on the renderer thread
	void run(std::invocable auto &&f, bool awaitReply = false)
	{
		RendererTaskImpl::run(IG_forward(f), awaitReply);
	}

	// Run a delegate for drawing on the renderer thread
	// Returns true if the window's contents were presented synchronously
	bool draw(Window &win, WindowDrawParams winParams, DrawParams params,
		std::invocable<Window &, RendererCommands &> auto &&f)
	{
		return RendererTaskImpl::draw(win, winParams, params, IG_forward(f));
	}

	// synchronization
	SyncFence addSyncFence();
	void deleteSyncFence(SyncFence);
	void clientWaitSync(SyncFence fence, int flags = 0, std::chrono::nanoseconds timeout = SyncFence::IGNORE_TIMEOUT);
	SyncFence clientWaitSyncReset(SyncFence fence, int flags = 0, std::chrono::nanoseconds timeout = SyncFence::IGNORE_TIMEOUT);
	void waitSync(SyncFence fence);
	void awaitPending();
};

}
