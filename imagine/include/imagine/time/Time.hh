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

#include <imagine/util/utility.h>
#include <chrono>
#if defined __APPLE__
#include <TargetConditionals.h>
#endif

namespace IG
{

using Nanoseconds = std::chrono::nanoseconds;
using Microseconds = std::chrono::microseconds;
using Milliseconds = std::chrono::milliseconds;
using Seconds = std::chrono::seconds;
using FloatSeconds = std::chrono::duration<double>;
using Minutes = std::chrono::minutes;

#if defined __APPLE__ && TARGET_OS_IPHONE
using FrameTime = FloatSeconds;
#else
using FrameTime = Nanoseconds;
#endif

using Time = Nanoseconds; // default time resolution

template <class T>
concept ChronoDuration =
	requires
	{
		typename T::rep;
		typename T::period;
	};

static Time steadyClockTimestamp()
{
	auto timePoint = std::chrono::steady_clock::now();
	return Time{timePoint.time_since_epoch()};
}

static Time timeFunc(auto &&func, auto &&...args)
{
	auto before = steadyClockTimestamp();
	func(IG_forward(args)...);
	auto after = steadyClockTimestamp();
	return after - before;
}

static Time timeFuncDebug(auto &&func, auto &&...args)
{
	#ifdef NDEBUG
	// execute directly without timing
	func(IG_forward(args)...);
	return {};
	#else
	return timeFunc(IG_forward(func), IG_forward(args)...);
	#endif
}

class FrameParams
{
public:
	constexpr FrameParams(FrameTime timestamp_, FloatSeconds frameTime_):
		timestamp_{timestamp_}, frameTime_{frameTime_}
	{}
	FrameTime timestamp() const { return timestamp_; }
	FloatSeconds frameTime() const { return frameTime_; }
	FrameTime presentTime() const;
	uint32_t elapsedFrames(FrameTime lastTimestamp) const;
	static uint32_t elapsedFrames(FrameTime timestamp, FrameTime lastTimestamp, FloatSeconds frameTime);

protected:
	FrameTime timestamp_;
	FrameTime lastTimestamp_;
	FloatSeconds frameTime_;
};

}
