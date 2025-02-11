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

#define LOGTAG "GLRenderer"
#include <imagine/gfx/Renderer.hh>
#include <imagine/gfx/RendererTask.hh>
#include <imagine/gfx/Projection.hh>
#include <imagine/logger/logger.h>
#include <imagine/base/Window.hh>
#include <imagine/base/GLContext.hh>
#include <imagine/base/ApplicationContext.hh>
#include <imagine/base/Viewport.hh>
#include <imagine/base/Error.hh>
#include <imagine/data-type/image/PixmapSource.hh>
#include "internalDefs.hh"

namespace IG::Gfx
{

static_assert((uint8_t)TextureBufferMode::DEFAULT == 0, "TextureBufferMode::DEFAULT != 0");

Renderer::Renderer(ApplicationContext ctx):
	GLRenderer{ctx}
{}

Renderer::~Renderer()
{
	for(auto &w : appContext().windows())
	{
		detachWindow(*w);
	}
}

GLRenderer::GLRenderer(ApplicationContext ctx):
	glManager{ctx.nativeDisplayConnection(), glAPI},
	mainTask{ctx, "Main GL Context Messages", *static_cast<Renderer*>(this)},
	releaseShaderCompilerEvent{"GLRenderer::releaseShaderCompilerEvent"}
{
	if(!glManager)
	{
		throw std::runtime_error("Renderer error getting GL display");
	}
	glManager.logInfo();
}

void Renderer::initMainTask(Window *initialWindow, DrawableConfig drawableConfig)
{
	if(mainTask.glContext())
	{
		return;
	}
	auto ctx = appContext();
	auto bufferConfig = makeGLBufferConfig(ctx, drawableConfig.pixelFormat, initialWindow);
	if(!bufferConfig) [[unlikely]]
	{
		throw std::runtime_error("Renderer error finding a GL configuration");
	}
	Drawable initialDrawable{};
	if(initialWindow)
	{
		if(!GLRenderer::attachWindow(*initialWindow, *bufferConfig, (GLColorSpace)drawableConfig.colorSpace))
		{
			throw std::runtime_error("Renderer error creating window surface");
		}
		initialDrawable = (Drawable)winData(*initialWindow).drawable;
	}
	constexpr int DRAW_THREAD_PRIORITY = -4;
	GLTaskConfig conf
	{
		.glManagerPtr = &glManager,
		.bufferConfig = *bufferConfig,
		.initialDrawable = initialDrawable,
		.threadPriority = DRAW_THREAD_PRIORITY,
	};
	if(!mainTask.makeGLContext(conf)) [[unlikely]]
	{
		throw std::runtime_error("Renderer error creating GL context");
	}
	addEventHandlers(ctx, mainTask);
	configureRenderer();
	if(!initBasicEffect()) [[unlikely]]
	{
		throw std::runtime_error("Renderer error creating basic shader program");
	}
}

NativeWindowFormat GLRenderer::nativeWindowFormat(GLBufferConfig bufferConfig) const
{
	return glManager.nativeWindowFormat(mainTask.appContext(), bufferConfig);
}

bool GLRenderer::attachWindow(Window &win, GLBufferConfig bufferConfig, GLColorSpace colorSpace)
{
	if(!win.hasSurface()) [[unlikely]]
	{
		logMsg("can't attach uninitialized window");
		return false;
	}
	logMsg("attaching window:%p", &win);
	auto &rData = win.makeRendererData<GLRendererWindowData>();
	if(!makeWindowDrawable(mainTask, win, bufferConfig, colorSpace)) [[unlikely]]
	{
		return false;
	}
	if(win.isMainWindow())
	{
		if(!Config::SYSTEM_ROTATES_WINDOWS)
		{
			rData.projAngleM = rotationRadians(win.softOrientation());
			win.appContext().setOnDeviceOrientationChanged(
				[this, &win](ApplicationContext, Rotation newO)
				{
					auto oldWinO = win.softOrientation();
					if(win.requestOrientationChange(newO))
					{
						static_cast<Renderer*>(this)->animateWindowRotation(win, rotationRadians(oldWinO), rotationRadians(newO));
					}
				});
		}
		else if(Config::SYSTEM_ROTATES_WINDOWS && !win.appContext().systemAnimatesWindowRotation())
		{
			win.appContext().setOnSystemOrientationChanged(
				[this, &win](ApplicationContext, Rotation oldO, Rotation newO)
				{
					const float orientationDiffTable[4][4]
					{
						{0, radians(90.), radians(-180.), radians(-90.)},
						{radians(-90.), 0, radians(90.), radians(-180.)},
						{radians(-180.), radians(-90.), 0, radians(90.)},
						{radians(90.), radians(-180.), radians(-90.), 0},
					};
					auto rotAngle = orientationDiffTable[std::to_underlying(oldO)][std::to_underlying(newO)];
					logMsg("animating from %d degrees", (int)degrees(rotAngle));
					static_cast<Renderer*>(this)->animateWindowRotation(win, rotAngle, 0.);
				});
		}
	}
	return true;
}

bool GLRenderer::makeWindowDrawable(RendererTask &task, Window &win, GLBufferConfig bufferConfig, GLColorSpace colorSpace)
{
	auto &rData = winData(win);
	rData.bufferConfig = bufferConfig;
	rData.colorSpace = colorSpace;
	task.destroyDrawable(rData.drawable);
	GLDrawableAttributes attr{bufferConfig};
	attr.colorSpace = colorSpace;
	try
	{
		rData.drawable = glManager.makeDrawable(win, attr);
	}
	catch(...)
	{
		return false;
	}
	return true;
}

bool Renderer::attachWindow(Window &win, DrawableConfig drawableConfig)
{
	GLBufferConfig bufferConfig = mainTask.glBufferConfig();
	if(canRenderToMultiplePixelFormats())
	{
		auto bufferConfigOpt = makeGLBufferConfig(appContext(), drawableConfig.pixelFormat, &win);
		if(!bufferConfigOpt) [[unlikely]]
		{
			return false;
		}
		bufferConfig = *bufferConfigOpt;
	}
	return GLRenderer::attachWindow(win, bufferConfig, (GLColorSpace)drawableConfig.colorSpace);
}

void Renderer::detachWindow(Window &win)
{
	win.resetRendererData();
	if(win.isMainWindow())
	{
		if(!Config::SYSTEM_ROTATES_WINDOWS)
		{
			win.appContext().setOnDeviceOrientationChanged({});
		}
		else if(Config::SYSTEM_ROTATES_WINDOWS && !win.appContext().systemAnimatesWindowRotation())
		{
			win.appContext().setOnSystemOrientationChanged({});
		}
	}
}

bool Renderer::setDrawableConfig(Window &win, DrawableConfig config)
{
	auto bufferConfig = makeGLBufferConfig(appContext(), config.pixelFormat, &win);
	if(!bufferConfig) [[unlikely]]
	{
		return false;
	}
	if(winData(win).bufferConfig == *bufferConfig && winData(win).colorSpace == (GLColorSpace)config.colorSpace)
	{
		return true;
	}
	if(mainTask.glBufferConfig() != *bufferConfig && !canRenderToMultiplePixelFormats())
	{
		// context only supports config it was created with
		return false;
	}
	win.setFormat(config.pixelFormat);
	return makeWindowDrawable(mainTask, win, *bufferConfig, (GLColorSpace)config.colorSpace);
}

void Renderer::setDefaultViewport(Window &win, Viewport v)
{
	winData(win).viewportRect = asYUpRelRect(v);
}

bool Renderer::canRenderToMultiplePixelFormats() const
{
	return glManager.hasNoConfigContext();
}

void Renderer::releaseShaderCompiler()
{
	if(!support.useFixedFunctionPipeline)
		task().releaseShaderCompiler();
}

void Renderer::autoReleaseShaderCompiler()
{
	if(!support.useFixedFunctionPipeline)
		releaseShaderCompilerEvent.notify();
}

ClipRect Renderer::makeClipRect(const Window &win, IG::WindowRect rect)
{
	int x = rect.x;
	int y = rect.y;
	int w = rect.xSize();
	int h = rect.ySize();
	//logMsg("scissor before transform %d,%d size %d,%d", x, y, w, h);
	// translate from view to window coordinates
	if(!Config::SYSTEM_ROTATES_WINDOWS)
	{
		switch(win.softOrientation())
		{
			default:
				//x += win.viewport.rect.x;
				y = win.height() - (y + h);
			bcase Rotation::RIGHT:
				//x += win.viewport.rect.y;
				//y = win.width() - (y + h /*+ (win.w - win.viewport.rect.x2)*/);
				std::swap(x, y);
				std::swap(w, h);
				x = (win.realWidth() - x) - w;
				y = (win.realHeight() - y) - h;
			bcase Rotation::LEFT:
				//x += win.viewport.rect.y;
				//y += win.viewport.rect.x;
				std::swap(x, y);
				std::swap(w, h);
			bcase Rotation::DOWN:
				x = (win.realWidth() - x) - w;
				//y = win.height() - (y + h);
				//std::swap(x, y);
				//std::swap(w, h);
				//x += win.viewport.rect.x;
				//y += win.height() - win.viewport.bounds().y2;
		}
	}
	else
	{
		//x += win.viewport.rect.x;
		y = win.height() - (y + h /*+ win.viewport.rect.y*/);
	}
	return {{x, y}, {w, h}};
}

bool Renderer::supportsSyncFences() const
{
	return support.hasSyncFences();
}

void Renderer::setPresentationTime(Window &win, IG::FrameTime time) const
{
	#ifdef __ANDROID__
	if(!supportsPresentationTime())
		return;
	auto drawable = (Drawable)winData(win).drawable;
	bool success = support.eglPresentationTimeANDROID(glDisplay(), drawable, time.count());
	if(Config::DEBUG_BUILD && !success)
	{
		logErr("error:%s in eglPresentationTimeANDROID(%p, %llu)",
			GLManager::errorString(eglGetError()), (EGLSurface)drawable, (unsigned long long)time.count());
	}
	#endif
}

bool Renderer::supportsPresentationTime() const
{
	#ifdef __ANDROID__
	return support.eglPresentationTimeANDROID;
	#else
	return false;
	#endif
}

int Renderer::maxSwapChainImages() const
{
	#ifdef __ANDROID__
	if(appContext().androidSDK() < 18)
		return 2;
	#endif
	return 3; // assume triple-buffering by default
}

bool Renderer::supportsColorSpace() const
{
	return glManager.hasSrgbColorSpace();
}

bool Renderer::hasSrgbColorSpaceWriteControl() const
{
	return support.hasSrgbWriteControl;
}

ColorSpace Renderer::supportedColorSpace(PixelFormat fmt, ColorSpace wantedColorSpace)
{
	switch(wantedColorSpace)
	{
		case ColorSpace::LINEAR: return ColorSpace::LINEAR;
		case ColorSpace::SRGB:
			switch(fmt.id())
			{
				case PIXEL_FMT_RGBA8888:
				case PIXEL_FMT_BGRA8888:
					return ColorSpace::SRGB;
				default: return ColorSpace::LINEAR;
			}
	}
	return ColorSpace::LINEAR;
}

ApplicationContext Renderer::appContext() const
{
	return task().appContext();
}

GLRendererWindowData &winData(Window &win)
{
	assumeExpr(win.rendererData<GLRendererWindowData>());
	return *win.rendererData<GLRendererWindowData>();
}

const GLRendererWindowData &winData(const Window &win)
{
	assumeExpr(win.rendererData<GLRendererWindowData>());
	return *win.rendererData<GLRendererWindowData>();
}

GLDisplay GLRenderer::glDisplay() const
{
	return glManager.display();
}

std::vector<DrawableConfigDesc> Renderer::supportedDrawableConfigs() const
{
	std::vector<DrawableConfigDesc> formats{};
	formats.reserve(3);
	static constexpr DrawableConfigDesc testDescs[]
	{
		{
			.name = "RGBA8888",
			.config{ .pixelFormat = PIXEL_RGBA8888 }
		},
		{
			.name = "RGBA8888:sRGB",
			.config{ .pixelFormat = PIXEL_RGBA8888, .colorSpace = ColorSpace::SRGB }
		},
		{
			.name = "RGB565",
			.config{ .pixelFormat = PIXEL_RGB565 }
		},
	};
	for(auto desc : testDescs)
	{
		if(glManager.hasDrawableConfig({desc.config.pixelFormat}, (GLColorSpace)desc.config.colorSpace))
		{
			formats.emplace_back(desc);
		}
	}
	return formats;
}

bool Renderer::hasBgraFormat(TextureBufferMode mode) const
{
	if constexpr(Config::envIsAndroid)
	{
		if(mode == TextureBufferMode::ANDROID_HARDWARE_BUFFER || mode == TextureBufferMode::ANDROID_SURFACE_TEXTURE)
			return false;
	}
	if(!support.hasBGRPixels)
		return false;
	if(Config::Gfx::OPENGL_ES && support.hasImmutableTexStorage)
	{
		return false;
	}
	return true;
}

bool GLRenderer::initBasicEffect()
{
	if(support.useFixedFunctionPipeline)
		return true;
	auto &rTask = mainTask;

	std::string_view vertShaderSrc =
R"(in vec4 pos;
in vec4 color;
in vec2 texUV;
out vec4 colorOut;
out vec2 texUVOut;
uniform mat4 modelView;
uniform mat4 proj;
void main() {
	colorOut = color;
	texUVOut = texUV;
	gl_Position = proj * modelView * pos;
})";

	std::string_view fragShaderSrc =
R"(in mediump vec4 colorOut;
in lowp vec2 texUVOut;
uniform sampler2D tex;
uniform lowp int textureMode;
void main() {
	if(textureMode == 1)
	{
		FRAGCOLOR = colorOut * texture(tex, texUVOut);
	}
#ifdef alphaTexSwizzle
	else if(textureMode == 2)
	{
		FRAGCOLOR.rgb = colorOut.rgb;
		FRAGCOLOR.a = colorOut.a * texture(tex, texUVOut).a;
	}
#endif
	else
	{
		FRAGCOLOR = colorOut;
	}
})";

	std::string_view fragDefsSrc = support.hasTextureSwizzle ? "" : "#define alphaTexSwizzle\n";
	std::array<std::string_view, 2> fragSrcs{fragDefsSrc, fragShaderSrc};
	if(!basicEffect_.setShaders(rTask, {&vertShaderSrc, 1}, fragSrcs)) [[unlikely]]
		return false;
	releaseShaderCompilerEvent.notify();
	return true;
}

BasicEffect &Renderer::basicEffect()
{
	return basicEffect_;
}

void Renderer::animateWindowRotation(Window &win, float srcAngle, float destAngle)
{
	winData(win).projAngleM = {srcAngle, destAngle, {}, steadyClockTimestamp(), Milliseconds{165}};
	win.addOnFrame([this, &win](FrameParams params)
	{
		win.signalSurfaceChanged(WindowSurfaceChange::CONTENT_RECT_RESIZED);
		bool didUpdate = winData(win).projAngleM.update(params.timestamp());
		return didUpdate;
	});
}

Projection Renderer::projection(const Window &win, Viewport viewport, Mat4 matrix) const
{
	return {viewport, matrix, winData(win).projAngleM};
}

Texture Renderer::makeTexture(TextureConfig config)
{
	return {task(), config};
}

Texture Renderer::makeTexture(Data::PixmapSource img, TextureSamplerConfig samplerConf, bool makeMipmaps)
{
	return {task(), img, samplerConf, makeMipmaps};
}

PixmapBufferTexture Renderer::makePixmapBufferTexture(TextureConfig config, TextureBufferMode mode, bool singleBuffer)
{
	return {task(), config, mode, singleBuffer};
}

TextureSampler Renderer::makeTextureSampler(TextureSamplerConfig config)
{
	return {task(), config};
}

void destroyGLBuffer(RendererTask &task, NativeBuffer buff)
{
	logMsg("deleting GL buffer:%u", buff);
	task.run(
		[buff]()
		{
			glDeleteBuffers(1, &buff);
		});
}

}
