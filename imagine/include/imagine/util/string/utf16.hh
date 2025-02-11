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
#include <concepts>
#include <string>
#include <string_view>

namespace IG
{

std::u16string makeUTF16String(std::string_view);

// Simple wrapper around u16string that converts any char-type strings from UTF-8 to UTF-16
class utf16String : public std::u16string
{
public:
	using std::u16string::u16string;
	using std::u16string::operator=;
	utf16String(std::convertible_to<std::u16string> auto &&s):std::u16string{IG_forward(s)} {}
	utf16String(std::u16string_view s):std::u16string{s} {}

	// UTF-8 -> UTF-16 conversion
	utf16String(std::string_view s):std::u16string{makeUTF16String(s)} {}
	utf16String(std::convertible_to<std::string_view> auto &&s):utf16String{std::string_view{IG_forward(s)}} {}
	[[gnu::nonnull]]
	utf16String(const char *s):utf16String{std::string_view{s}} {}
};

}
