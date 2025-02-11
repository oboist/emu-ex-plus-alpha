#ifndef GAMBATTE_LOADRES_H
#define GAMBATTE_LOADRES_H

#include <string>
#include <imagine/io/IOStream.hh>
#include <imagine/io/FileIO.hh>
#include <imagine/util/DelegateFunc.hh>

namespace gambatte {

enum LoadRes {
	LOADRES_BAD_FILE_OR_UNKNOWN_MBC = -0x7FFF,
	LOADRES_IO_ERROR,
	LOADRES_UNSUPPORTED_MBC_HUC3  = -0x1FE,
	LOADRES_UNSUPPORTED_MBC_TAMA5,
	LOADRES_UNSUPPORTED_MBC_POCKET_CAMERA,
	LOADRES_UNSUPPORTED_MBC_MBC7 = -0x122,
	LOADRES_UNSUPPORTED_MBC_MBC6 = -0x120,
	LOADRES_UNSUPPORTED_MBC_MBC4  = -0x117,
	LOADRES_UNSUPPORTED_MBC_MMM01 = -0x10D,
	LOADRES_OK = 0
};

std::string const to_string(LoadRes);

using InputStreamDelegate = IG::DelegateFunc<IG::IFStream(std::string_view basePath, std::string_view filename)>;
using OutputStreamDelegate = IG::DelegateFunc<IG::OFStream(std::string_view basePath, std::string_view filename)>;

}

#endif
