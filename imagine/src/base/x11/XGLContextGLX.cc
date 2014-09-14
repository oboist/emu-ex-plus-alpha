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

#include <imagine/base/GLContext.hh>
#include <imagine/util/time/sys.hh>
#include <imagine/logger/logger.h>
#include "internal.hh"

// TODO: remove GLX implementation once there is sufficient driver support from desktop GPU vendors for EGL

namespace Base
{

static GLXPbuffer dummyPbuff{};
using GLXAttrList = StaticArrayList<int, 24>;

static GLXAttrList glConfigAttrsToGLXAttrs(const GLBufferConfigAttributes &attr)
{
	GLXAttrList list;

	list.push_back(GLX_DOUBLEBUFFER);
	list.push_back(True);

	if(attr.preferredColorBits() > 16)
	{
		list.push_back(GLX_RED_SIZE);
		list.push_back(1);
		list.push_back(GLX_GREEN_SIZE);
		list.push_back(1);
		list.push_back(GLX_BLUE_SIZE);
		list.push_back(1);
	}
	else
	{
		logMsg("requesting lowest color config");
	}

	list.push_back(None);
	return list;
}

static const char* glxConfigCaveatToStr(int cav)
{
	switch(cav)
	{
		case GLX_NONE: return "none";
		case GLX_SLOW_CONFIG: return "slow";
		case GLX_NON_CONFORMANT_CONFIG: return "non-conformant";
	}
	return "unknown";
}

static void printGLXVisual(Display *display, XVisualInfo &config)
{
	int buffSize, redSize, greenSize, blueSize, alphaSize, depthSize, stencilSize, sampleBuff;
	glXGetConfig(display, &config, GLX_BUFFER_SIZE, &buffSize);
	glXGetConfig(display, &config, GLX_RED_SIZE, &redSize);
	glXGetConfig(display, &config, GLX_GREEN_SIZE, &greenSize);
	glXGetConfig(display, &config, GLX_BLUE_SIZE, &blueSize);
	glXGetConfig(display, &config, GLX_ALPHA_SIZE, &alphaSize);
	//glXGetConfig(display, &config, GLX_CONFIG_CAVEAT, &cav);
	glXGetConfig(display, &config, GLX_DEPTH_SIZE, &depthSize);
	glXGetConfig(display, &config, GLX_STENCIL_SIZE, &stencilSize);
	glXGetConfig(display, &config, GLX_SAMPLE_BUFFERS, &sampleBuff);
	logMsg("config %d %d:%d:%d:%d d:%d sten:%d sampleBuff:%d",
			buffSize, redSize, greenSize, blueSize, alphaSize,
			depthSize, stencilSize,
			sampleBuff);
}

static GLXContext createContextForMajorVersion(uint version, Display *dpy, GLXFBConfig config, bool &supportsSurfaceless)
{
	supportsSurfaceless = false;
	auto glXCreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");
	if(version >= 3)
	{
		{
			// Try 3.2 Core
			const int attrCore3_2[]
			{
				GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
				GLX_CONTEXT_MINOR_VERSION_ARB, 2,
				GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
				#ifndef NDEBUG
				GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_DEBUG_BIT_ARB,
				#endif
				None
			};
			GLXContext ctx = glXCreateContextAttribsARB(dpy, config, 0, True, attrCore3_2);
			if(ctx)
			{
				supportsSurfaceless = true;
				return ctx;
			}
			logErr("failed creating 3.2 core context");
		}
		{
			// Try 3.1
			const int attr3_1[] =
			{
				GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
				GLX_CONTEXT_MINOR_VERSION_ARB, 1,
				#ifndef NDEBUG
				GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_DEBUG_BIT_ARB,
				#endif
				None
			};
			GLXContext ctx = glXCreateContextAttribsARB(dpy, config, 0, True, attr3_1);
			if(ctx)
			{
				supportsSurfaceless = true;
				return ctx;
			}
			logErr("failed creating 3.1 context");
		}
		{
			// Try 3.0
			const int attr3_0[] =
			{
				GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
				GLX_CONTEXT_MINOR_VERSION_ARB, 0,
				#ifndef NDEBUG
				GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_DEBUG_BIT_ARB,
				#endif
				None
			};
			GLXContext ctx = glXCreateContextAttribsARB(dpy, config, 0, True, attr3_0);
			if(ctx)
			{
				supportsSurfaceless = true;
				return ctx;
			}
			logErr("failed creating 3.0 context");
		}
		// Fallback to 1.2
	}
	const int attr[] =
	{
		GLX_CONTEXT_MAJOR_VERSION_ARB, 1,
		GLX_CONTEXT_MINOR_VERSION_ARB, 2,
		#ifndef NDEBUG
		GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_DEBUG_BIT_ARB,
		#endif
		None
	};
	GLXContext ctx = glXCreateContextAttribsARB(dpy, config, 0, True, attr);
	if(ctx)
		return ctx;
	logErr("failed creating 1.2 context");
	return (GLXContext)0;
}

GLBufferConfig GLContext::makeBufferConfig(const GLContextAttributes &, const GLBufferConfigAttributes &attr)
{
	GLBufferConfig conf;
	int screenIdx = indexOfScreen(mainScreen());
	{
		int count;
		auto glxAttr = glConfigAttrsToGLXAttrs(attr);
		auto fbcArr = glXChooseFBConfig(dpy, screenIdx, &glxAttr[0], &count);
		if(!fbcArr)
		{
			return GLBufferConfig{};
		}
		conf.glConfig = fbcArr[0];
		XFree(fbcArr);
	}
	{
		auto viPtr = glXGetVisualFromFBConfig(dpy, conf.glConfig);
		if(!viPtr)
		{
			logErr("failed to get matching visual for fbconfig");
			return GLBufferConfig{};
		}
		if(Config::DEBUG_BUILD)
			printGLXVisual(dpy, *viPtr);
		conf.visual = viPtr->visual;
		conf.depth = viPtr->depth;
		XFree(viPtr);
	}
	return conf;
}

CallResult GLContext::init(const GLContextAttributes &attr, const GLBufferConfig &config)
{
	assert(!context);
	bool supportsSurfaceless;
	context = createContextForMajorVersion(attr.majorVersion(), dpy, config.glConfig, supportsSurfaceless);
	if(!context)
	{
		logErr("can't find a compatible context");
		return INVALID_PARAMETER;
	}
	supportsSurfaceless = false; // TODO: glXMakeContextCurrent doesn't work correctly on some drivers, see below
	if(!supportsSurfaceless && !dummyPbuff)
	{
		const int attrib[] {GLX_PBUFFER_WIDTH, 1, GLX_PBUFFER_HEIGHT, 1, None};
		dummyPbuff = glXCreatePbuffer(dpy, config.glConfig, attrib);
		if(!dummyPbuff)
		{
			bug_exit("couldn't make dummy pbuffer");
		}
	}

	if(Config::DEBUG_BUILD)
	{
		int glxMajorVersion, glxMinorVersion;
		glXQueryVersion(dpy, &glxMajorVersion, &glxMinorVersion);
		logMsg("GLX version %d.%d, direct context: %s", glxMajorVersion, glxMinorVersion, glXIsDirect(dpy, context) ? "yes" : "no");
		if(!glXIsDirect(dpy, context))
			bug_exit("direct rendering not supported, check your X configuration");
	}

	return OK;
}

static void setCurrentContext(GLXContext context, Window *win)
{
	if(!context)
	{
		logMsg("setting no context current");
		assert(!win);
		if(!glXMakeContextCurrent(dpy, None, None, nullptr))
		{
			bug_exit("error setting no context current");
		}
	}
	else if(win)
	{
		if(!glXMakeContextCurrent(dpy, win->xWin, win->xWin, context))
		{
			bug_exit("error setting window 0x%p current", win);
		}
	}
	else
	{
		// TODO: with nvidia (tested on 337.12) glXMakeContextCurrent fails with badmatch
		// even if using a 3.0 context
		/*if(!dummyPbuff)
		{
			logMsg("setting no drawable current");
			if(!glXMakeContextCurrent(c->display, None, None, context))
			{
				bug_exit("error setting no drawable current");
			}
		}
		else*/
		{
			logMsg("setting dummy pbuffer current");
			if(!glXMakeContextCurrent(dpy, dummyPbuff, dummyPbuff, context))
			{
				bug_exit("error setting dummy pbuffer current");
			}
		}
	}
}

void GLContext::setCurrent(GLContext context, Window *win)
{
	setCurrentContext(context.context, win);
}

void GLContext::setDrawable(Window *win)
{
	setDrawable(win, current());
}

void GLContext::setDrawable(Window *win, GLContext cachedCurrentContext)
{
	setCurrentContext(cachedCurrentContext.context, win);
}

GLContext GLContext::current()
{
	GLContext c;
	c.context = glXGetCurrentContext();
	return c;
}

void GLContext::present(Window &win)
{
	auto swapTime = IG::timeFuncDebug([&](){ glXSwapBuffers(dpy, win.xWin); }).toNs();
	if(swapTime > 16000000)
	{
		//logWarn("buffer swap took %lldns", (long long)swapTime);
	}
}

void GLContext::present(Window &win, GLContext cachedCurrentContext)
{
	present(win);
}

GLContext::operator bool() const
{
	return context;
}

void GLContext::deinit()
{
	if(context)
	{
		glXDestroyContext(dpy, context);
		context = nullptr;
	}
}

}
