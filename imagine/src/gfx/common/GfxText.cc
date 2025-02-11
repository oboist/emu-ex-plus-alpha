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

#include <imagine/gfx/GfxText.hh>
#include <imagine/gfx/GfxSprite.hh>
#include <imagine/gfx/GlyphTextureSet.hh>
#include <imagine/gfx/ProjectionPlane.hh>
#include <imagine/gfx/Renderer.hh>
#include <imagine/gfx/RendererCommands.hh>
#include <imagine/gfx/GeomQuad.hh>
#include <imagine/util/math/int.hh>
#include <imagine/util/ctype.hh>
#include <imagine/logger/logger.h>
#include <algorithm>

namespace IG::Gfx
{

static void drawSpan(RendererCommands &cmds, float xPos, float yPos, ProjectionPlane projP,
	std::u16string_view strView, TexQuad &vArr, GlyphTextureSet *face_, float spaceSize);

Text::Text(GlyphTextureSet *face): Text{{}, face}
{}

Text::Text(IG::utf16String str, GlyphTextureSet *face):
	textStr{std::move(str)}, face_{face}
{}

void Text::setString(IG::utf16String str)
{
	textStr = std::move(str);
}

void Text::setFace(GlyphTextureSet *face_)
{
	assert(face_);
	this->face_ = face_;
}

static float xSizeOfChar(Renderer &r, GlyphTextureSet *face_, int c, float spaceX, const ProjectionPlane &projP)
{
	assert(c != '\0');
	if(c == ' ')
		return spaceX;
	if(c == '\n')
		return 0;

	GlyphEntry *gly = face_->glyphEntry(r, c);
	if(gly)
		return projP.unprojectXSize(gly->metrics.xAdvance);
	else
		return 0;
}

void Text::makeGlyphs(Renderer &r)
{
	if(!hasText()) [[unlikely]]
		return;
	for(auto c : textStr)
	{
		face_->glyphEntry(r, c);
	}
}

bool Text::compile(Renderer &r, ProjectionPlane projP, TextLayoutConfig conf)
{
	if(!hasText()) [[unlikely]]
		return false;
	//logMsg("compiling text %s", str);
	auto metrics = face_->metrics();
	yLineStart = projP.alignYToPixel(projP.unprojectYSize(metrics.yLineStart));
	spaceSize = projP.unprojectXSize(metrics.spaceSize);
	nominalHeight_ = projP.alignYToPixel(projP.unprojectYSize(IG::makeEvenRoundedUp(metrics.nominalHeight)));
	lines = 1;
	lineInfo.clear();
	float xLineSize = 0, maxXLineSize = 0;
	int prevC = 0;
	float textBlockSize = 0;
	int textBlockIdx = 0, currLineIdx = 0;
	int charIdx = 0, charsInLine = 0;
	for(auto c : textStr)
	{
		auto cSize = xSizeOfChar(r, face_, c, spaceSize, projP);
		charsInLine++;

		// Is this the start of a text block?
		if(isgraph(c) && isspace(prevC))
		{
			textBlockIdx = charIdx;
			textBlockSize = 0;
		}
		xLineSize += cSize;
		textBlockSize += cSize;
		bool lineHasMultipleBlocks = textBlockIdx != currLineIdx;
		bool lineExceedsMaxSize = xLineSize > conf.maxLineSize;
		if(lines < conf.maxLines)
		{
			bool wentToNextLine = false;
			// Go to next line?
			if(c == '\n' || (lineExceedsMaxSize && !lineHasMultipleBlocks))
			{
				wentToNextLine = true;
				// Don't break text
				//logMsg("new line %d without text break @ char %d, %d chars in line", lines+1, charIdx, charsInLine);
				lineInfo.emplace_back(xLineSize, charsInLine);
				maxXLineSize = std::max(xLineSize, maxXLineSize);
				xLineSize = 0;
				charsInLine = 0;
			}
			else if(lineExceedsMaxSize && lineHasMultipleBlocks)
			{
				wentToNextLine = true;
				// Line has more than 1 block and is too big, needs text break
				//logMsg("new line %d with text break @ char %d, %d chars in line", lines+1, charIdx, charsInLine);
				xLineSize -= textBlockSize;
				int charsInNextLine = (charIdx - textBlockIdx) + 1;
				lineInfo.emplace_back(xLineSize, charsInLine - charsInNextLine);
				maxXLineSize = std::max(xLineSize, maxXLineSize);
				xLineSize = textBlockSize;
				charsInLine = charsInNextLine;
				//logMsg("break @ char %d with line starting @ %d, %d chars moved to next line, leaving %d", textBlockIdx, currLineIdx, charsInNextLine, lineInfo[lines-1].chars);
			}
			if(wentToNextLine)
			{
				textBlockIdx = currLineIdx = charIdx+1;
				textBlockSize = 0;
				lines++;
			}
		}
		charIdx++;
		prevC = c;
	}
	if(lines > 1) // Add info of last line (1 line case doesn't use per-line info)
	{
		lineInfo.emplace_back(xLineSize, charsInLine);
	}
	maxXLineSize = std::max(xLineSize, maxXLineSize);
	xSize = maxXLineSize;
	ySize = nominalHeight_ * (float)lines;
	return true;
}

void Text::draw(RendererCommands &cmds, float xPos, float yPos, _2DOrigin o, ProjectionPlane projP) const
{
	if(!hasText()) [[unlikely]]
		return;
	//logMsg("drawing with origin: %s,%s", o.toString(o.x), o.toString(o.y));
	cmds.set(BlendMode::ALPHA);
	TexQuad vArr;
	cmds.bindTempVertexBuffer();
	cmds.setVertexAttribs(vArr.data());
	_2DOrigin align = o;
	xPos = o.adjustX(xPos, xSize, LT2DO);
	//logMsg("aligned to %f, converted to %d", Gfx::alignYToPixel(yPos), toIYPos(Gfx::alignYToPixel(yPos)));
	yPos = o.adjustY(yPos, projP.alignYToPixel(ySize/2.f), ySize, LT2DO);
	if(IG::isOdd(projP.windowBounds().ySize()))
		yPos = projP.alignYToPixel(yPos);
	yPos -= nominalHeight_ - yLineStart;
	float xOrig = xPos;
	//logMsg("drawing text @ %f,%f: str", xPos, yPos, str);
	auto startingXPos =
		[&](float xLineSize)
		{
			return projP.alignXToPixel(LT2DO.adjustX(xOrig, xSize-xLineSize, align));
		};
	if(lines > 1)
	{
		auto s = textStr.data();
		for(auto &span : lineInfo)
		{
			// Get line info (1 line case doesn't use per-line info)
			float xLineSize = span.size;
			auto charsToDraw = span.chars;
			xPos = startingXPos(xLineSize);
			//logMsg("line %d, %d chars", l, charsToDraw);
			drawSpan(cmds, xPos, yPos, projP, std::u16string_view{s, charsToDraw}, vArr, face_, spaceSize);
			s += charsToDraw;
			yPos -= nominalHeight_;
			yPos = projP.alignYToPixel(yPos);
		}
	}
	else
	{
		float xLineSize = xSize;
		xPos = startingXPos(xLineSize);
		//logMsg("line %d, %d chars", l, charsToDraw);
		drawSpan(cmds, xPos, yPos, projP, std::u16string_view{textStr}, vArr, face_, spaceSize);
	}
}

void Text::draw(RendererCommands &cmds, FP p, _2DOrigin o, ProjectionPlane projP) const
{
	draw(cmds, p.x, p.y, o, projP);
}

static void drawSpan(RendererCommands &cmds, float xPos, float yPos, ProjectionPlane projP,
	std::u16string_view strView, TexQuad &vArr, GlyphTextureSet *face_, float spaceSize)
{
	auto xViewLimit = projP.wHalf();
	for(auto c : strView)
	{
		if(c == '\n')
		{
			continue;
		}
		GlyphEntry *gly = face_->glyphEntry(cmds.renderer(), c, false);
		if(!gly)
		{
			//logMsg("no glyph for %X", c);
			xPos += spaceSize;
			continue;
		}
		if(xPos >= xViewLimit)
		{
			//logMsg("skipped %c, off right screen edge", s[i]);
			continue;
		}
		float xSize = projP.unprojectXSize(gly->metrics.xSize);
		auto x = xPos + projP.unprojectXSize(gly->metrics.xOffset);
		auto y = yPos - projP.unprojectYSize(gly->metrics.ySize - gly->metrics.yOffset);
		auto &glyph = gly->glyph();
		vArr = {{{x, y}, {x + xSize, y + projP.unprojectYSize(gly->metrics.ySize)}}, glyph};
		cmds.vertexBufferData(vArr.data(), sizeof(vArr));
		cmds.setTexture(glyph);
		//logMsg("drawing");
		cmds.drawPrimitives(Primitive::TRIANGLE_STRIP, 0, 4);
		xPos += projP.unprojectXSize(gly->metrics.xAdvance);
	}
}

float Text::width() const
{
	return xSize;
}

float Text::height() const
{
	return ySize;
}

float Text::fullHeight() const
{
	return ySize + (nominalHeight_ * .5f);
}

float Text::nominalHeight() const
{
	return nominalHeight_;
}

float Text::spaceWidth() const
{
	return spaceSize;
}

GlyphTextureSet *Text::face() const
{
	return face_;
}

uint16_t Text::currentLines() const
{
	return lines;
}

size_t Text::stringSize() const
{
	return textStr.size();
}

bool Text::isVisible() const
{
	return stringSize();
}

std::u16string_view Text::stringView() const
{
	return textStr;
}

std::u16string Text::string() const
{
	return textStr;
}

bool Text::hasText() const
{
	return face_ && stringSize();
}

}
