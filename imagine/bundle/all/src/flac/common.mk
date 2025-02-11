ifndef CHOST
 CHOST := $(shell $(CC) -dumpmachine)
endif

include $(buildSysPath)/imagineSDKPath.mk

# Flac configure script doesn't detect Ogg via pkg-config
OGG_PREFIX = $(IMAGINE_SDK_PLATFORM_PATH)

flacVer := 1.3.4
flacSrcDir := $(tempDir)/flac-$(flacVer)
flacSrcArchive := flac-$(flacVer).tar.xz

makeFile := $(buildDir)/Makefile
outputLibFile := $(buildDir)/src/libFLAC/.libs/libFLAC.a
installIncludeDir := $(installDir)/include/FLAC

all : $(outputLibFile)

install : $(outputLibFile)
	@echo "Installing flac to: $(installDir)"
	@mkdir -p $(installIncludeDir) $(installDir)/lib/pkgconfig
	cp $(outputLibFile) $(installDir)/lib/
	cp $(flacSrcDir)/include/FLAC/*.h $(installIncludeDir)/
	cp $(buildDir)/src/libFLAC/flac.pc $(installDir)/lib/pkgconfig/

.PHONY : all install

$(flacSrcDir)/configure : | $(flacSrcArchive)
	@echo "Extracting flac..."
	@mkdir -p $(flacSrcDir)
	tar -mxJf $| -C $(flacSrcDir)/..
	cp ../gnuconfig/config.* $(flacSrcDir)
	autoreconf -vfi $(flacSrcDir)

$(outputLibFile) : $(makeFile)
	@echo "Building flac..."
ifeq ($(ARCH), x86)
	$(MAKE) -C $(<D)/src/libFLAC/ia32 libFLAC-asm.la
endif
	$(MAKE) -C $(<D)/src/libFLAC libFLAC.la

$(makeFile) : $(flacSrcDir)/configure
	@echo "Configuring flac..."
	@mkdir -p $(@D)
	dir=`pwd` && cd $(@D) && $(toolchainEnvParams) CFLAGS="$(CPPFLAGS) $(CFLAGS)" \
	LDFLAGS="$(LDFLAGS) $(LDLIBS)" $(flacSrcDir)/configure \
	--prefix='$${pcfiledir}/../..' --disable-examples --disable-oggtest --disable-xmms-plugin --disable-cpplibs \
	--disable-doxygen-docs --disable-shared --host=$(CHOST) --with-ogg=$(OGG_PREFIX) $(buildArg)
