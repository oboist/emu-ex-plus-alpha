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
#include <imagine/time/Time.hh>
#include <imagine/base/baseDefs.hh>
#include <imagine/base/SimpleFrameTimer.hh>
#include <imagine/base/android/Choreographer.hh>
#include <imagine/base/FrameTimerInterface.hh>
#include <imagine/util/jni.hh>
#include <utility>
#include <compare>
#include <memory>

namespace IG
{

class ApplicationContext;

using FrameTimerVariant = std::variant<NativeChoreographerFrameTimer, JavaChoreographerFrameTimer, SimpleFrameTimer>;

class FrameTimer : public FrameTimerInterface<FrameTimerVariant>
{
public:
	using FrameTimerInterface::FrameTimerInterface;
};

class AndroidScreen
{
public:
	struct InitParams
	{
		JNIEnv *env;
		jobject aDisplay;
		jobject metrics;
		int id;
		float refreshRate;
		Rotation rotation;
	};

	AndroidScreen(ApplicationContext, InitParams);
	float densityDPI() const;
	float scaledDensityDPI() const;
	jobject displayObject() const;
	int id() const;
	void updateRefreshRate(float refreshRate);
	bool operator ==(AndroidScreen const &rhs) const;
	explicit operator bool() const;

	constexpr bool operator ==(ScreenId id) const
	{
		return id_ == id;
	}

protected:
	JNI::UniqueGlobalRef aDisplay{};
	FrameTimer frameTimer;
	IG::FloatSeconds frameTime_{};
	float densityDPI_{};
	float scaledDensityDPI_{};
	float refreshRate_{};
	int width_{}, height_{};
	int id_{};
	bool reliableRefreshRate = true;
};

using ScreenImpl = AndroidScreen;

}
