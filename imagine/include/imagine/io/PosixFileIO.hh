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

#include <imagine/io/PosixIO.hh>
#include <imagine/io/MapIO.hh>
#include <imagine/util/string/CStringView.hh>
#include <variant>

namespace IG
{

class IO;

class PosixFileIO : public IOUtils<PosixFileIO>
{
public:
	using IOUtilsBase = IOUtils<PosixFileIO>;
	using IOUtilsBase::write;
	using IOUtilsBase::seekS;
	using IOUtilsBase::seekE;
	using IOUtilsBase::seekC;
	using IOUtilsBase::tell;
	using IOUtilsBase::send;
	using IOUtilsBase::buffer;
	using IOUtilsBase::get;
	using IOUtilsBase::toFileStream;

	constexpr PosixFileIO() = default;
	PosixFileIO(UniqueFileDescriptor fd, AccessHint access, OpenFlagsMask);
	PosixFileIO(UniqueFileDescriptor fd, OpenFlagsMask);
	PosixFileIO(CStringView path, AccessHint access, OpenFlagsMask oFlags = {});
	PosixFileIO(CStringView path, OpenFlagsMask oFlags = {});
	ssize_t read(void *buff, size_t bytes);
	ssize_t readAtPos(void *buff, size_t bytes, off_t offset);
	std::span<uint8_t> map();
	ssize_t write(const void *buff, size_t bytes);
	bool truncate(off_t offset);
	off_t seek(off_t offset, SeekMode mode);
	void sync();
	size_t size();
	bool eof();
	void advise(off_t offset, size_t bytes, Advice advice);
	explicit operator bool() const;
	IOBuffer releaseBuffer();
	UniqueFileDescriptor releaseFd();
	operator IO();

protected:
	std::variant<PosixIO, MapIO> ioImpl{};

	void tryMmap(AccessHint access, OpenFlagsMask);
};

}
