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
#include <imagine/base/CustomEvent.hh>
#include <imagine/base/ApplicationContext.hh>
#include <imagine/util/DelegateFunc.hh>
#include <imagine/util/DelegateFuncSet.hh>
#include <imagine/util/Point2D.hh>
#include <imagine/util/used.hh>
#include <memory>

namespace IG
{

class BaseWindow
{
public:
	using SurfaceChange = WindowSurfaceChange;
	using DrawParams = WindowDrawParams;
	using InitDelegate = WindowInitDelegate;
	using SurfaceChangeDelegate = WindowSurfaceChangeDelegate;
	using DrawDelegate = WindowDrawDelegate;
	using InputEventDelegate = WindowInputEventDelegate;
	using FocusChangeDelegate = WindowFocusChangeDelegate;
	using DragDropDelegate = WindowDragDropDelegate;
	using DismissRequestDelegate = WindowDismissRequestDelegate;
	using DismissDelegate = WindowDismissDelegate;

	static constexpr bool shouldRunOnInitAfterAddingWindow = true;

	BaseWindow(ApplicationContext, WindowConfig);
	BaseWindow &operator=(BaseWindow &&) = delete;

protected:
	enum class DrawPhase : uint8_t
	{
		READY,	// Ready to run any onFrame delegates and enter UPDATE phase afterwards
		UPDATE,	// Stay in this phase until drawing is needed, then enter DRAW phase
		DRAW		// Drawing in progress, return to READY phase when finished
	};

	OnExit onExit;
	SurfaceChangeDelegate onSurfaceChange{};
	DrawDelegate onDraw{};
	InputEventDelegate onInputEvent{};
	FocusChangeDelegate onFocusChange{};
	DragDropDelegate onDragDrop{};
	DismissRequestDelegate onDismissRequest{};
	DismissDelegate onDismiss{};
	DelegateFuncSet<OnFrameDelegate> onFrame{};
	std::shared_ptr<void> appDataPtr{};
	std::shared_ptr<void> rendererDataPtr{};
	IG_UseMemberIf(Config::BASE_MULTI_SCREEN, Screen*, screen_){};
	CustomEvent drawEvent{"Window::drawEvent"};
	IG::Point2D<int> winSizePixels{}; // size of full window surface
	IG::Point2D<float> winSizeMM{}; // size in millimeter
	IG::Point2D<float> mmToPixelScaler{};
	 // size in millimeter scaled by OS
	IG_UseMemberIf(Config::envIsAndroid, IG::Point2D<float>, winSizeSMM){};
	IG_UseMemberIf(Config::envIsAndroid, IG::Point2D<float>, smmToPixelScaler){};
	bool drawNeeded{};
	DrawPhase drawPhase{DrawPhase::READY};
	int8_t drawEventPriority_{};
	// all windows need an initial onSurfaceChange call
	uint8_t surfaceChangeFlags{SurfaceChange::SURFACE_RESIZED | SurfaceChange::CONTENT_RECT_RESIZED};
	IG_UseMemberIfOrConstant(!Config::SYSTEM_ROTATES_WINDOWS, Rotation, Rotation::UP, softOrientation_){Rotation::UP};

	void setOnSurfaceChange(SurfaceChangeDelegate del);
	void setOnDraw(DrawDelegate del);
	void setOnInputEvent(InputEventDelegate del);
	void setOnFocusChange(FocusChangeDelegate del);
	void setOnDragDrop(DragDropDelegate del);
	void setOnDismissRequest(DismissRequestDelegate del);
	void setOnDismiss(DismissDelegate del);
	IG::Point2D<float> smmPixelScaler() const;
	void attachDrawEvent();
};

}
