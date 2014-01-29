#/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
# *   Mupen64plus - Makefile                                                *
# *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
# *   Copyright (C) 2007-2008 DarkJeztr Tillin9 Richard42                   *
# *                                                                         *
# *   This program is free software; you can redistribute it and/or modify  *
# *   it under the terms of the GNU General Public License as published by  *
# *   the Free Software Foundation; either version 2 of the License, or     *
# *   (at your option) any later version.                                   *
# *                                                                         *
# *   This program is distributed in the hope that it will be useful,       *
# *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
# *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
# *   GNU General Public License for more details.                          *
# *                                                                         *
# *   You should have received a copy of the GNU General Public License     *
# *   along with this program; if not, write to the                         *
# *   Free Software Foundation, Inc.,                                       *
# *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
# * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
# Makefile for glN64 in Mupen64plus.

ARCH = ARM
OS = LINUX

DEBUG ?= 0

CFLAGS  = -Wall -pipe -O3
LDFLAGS =

ifeq ($(DEBUG), 1)
    CFLAGS += -g
    LDFLAGS += -g
endif

COMPILER_DIR = /usr
SO_EXTENSION = so
CXX =  $(COMPILER_DIR)/bin/g++

LD =  $(COMPILER_DIR)/bin/g++
INCLUDE = /usr/local/include
CFLAGS  += -I$(INCLUDE)/libpng12
CFLAGS  += -I/usr/include/SDL2/
CFLAGS += -I$(INCLUDE)
CFLAGS += -I/opt/vc/include/
CFLAGS += -I/opt/vc/include/interface/vcos/pthreads/
CFLAGS += -I/opt/vc/include/interface/vmcs_host/linux

CFLAGS += -mfpu=neon -mfloat-abi=hard -flto
CFLAGS += -D__CRC_OPT -D__TRIBUFFER_OPT -D__VEC4_OPT -DARM -DUSE_SDL 

ifeq ($(OS), LINUX)
CFLAGS += -Wall -D__LINUX__ -fPIC
LDFLAGS += -L/usr/lib
LDFLAGS += -lpng -lGLESv2 -lSDL2
LDFLAGS += -shared
else
CFLAGS += -Wall
LDFLAGS += -lGLESv2 -lpng -lSDL2
endif

#CFLAGS += -DPROFILE_GBI
#CFLAGS += -DSHADER_TEST

# set installation options
ifeq ($(PREFIX),)
  PREFIX := /usr/local
endif
ifeq ($(SHAREDIR),)
  SHAREDIR := $(PREFIX)/share/mupen64plus
endif
ifeq ($(LIBDIR),)
  LIBDIR := $(PREFIX)/lib
endif
ifeq ($(PLUGINDIR),)
  PLUGINDIR := $(LIBDIR)/mupen64plus
endif

# set mupen64plus core API header path
ifneq ("$(APIDIR)","")
  CFLAGS += "-I$(APIDIR)"
else
  TRYDIR = ../mupen64plus-core/src/api
  ifneq ("$(wildcard $(TRYDIR)/m64p_types.h)","")
    CFLAGS += -I$(TRYDIR)
  else
    TRYDIR = /usr/local/include/mupen64plus
    ifneq ("$(wildcard $(TRYDIR)/m64p_types.h)","")
      CFLAGS += -I$(TRYDIR)
    else
      TRYDIR = /usr/include/mupen64plus
      ifneq ("$(wildcard $(TRYDIR)/m64p_types.h)","")
        CFLAGS += -I$(TRYDIR)
      else
        $(error Mupen64Plus API header files not found! Use makefile parameter APIDIR to force a location.)
      endif
    endif
  endif
endif

# only export api symbols
LDFLAGS += -Wl,-version-script,$(SRCDIR)/video_api_export.ver

# reduced compile output when running make without V=1
ifneq ($(findstring $(MAKEFLAGS),s),s)
ifndef V
	Q_CC  = @echo '    CC  '$@;
	Q_CXX = @echo '    CXX '$@;
	Q_LD  = @echo '    LD  '$@;
endif
endif

# set base program pointers and flags
CC        = $(CROSS_COMPILE)gcc
RM       ?= rm -f
INSTALL  ?= install
MKDIR ?= mkdir -p
COMPILE.c = $(Q_CC)$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
COMPILE.cc = $(Q_CXX)$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
LINK.o = $(Q_LD)$(CC) $(CFLAGS) $(LDFLAGS) $(TARGET_ARCH)

SRCDIR = src
OBJDIR = _obj$(POSTFIX)

# list of source files to compile
SOURCE = \
    $(SRCDIR)/2xSAI.cpp             \
    $(SRCDIR)/Config.cpp            \
    $(SRCDIR)/CRC.cpp               \
    $(SRCDIR)/DepthBuffer.cpp       \
    $(SRCDIR)/F3D.cpp               \
    $(SRCDIR)/F3DCBFD.cpp           \
    $(SRCDIR)/F3DDKR.cpp            \
    $(SRCDIR)/F3DEX.cpp             \
    $(SRCDIR)/F3DEX2.cpp            \
    $(SRCDIR)/F3DPD.cpp             \
    $(SRCDIR)/F3DWRUS.cpp           \
    $(SRCDIR)/GBI.cpp               \
    $(SRCDIR)/gDP.cpp               \
    $(SRCDIR)/gles2N64.cpp          \
    $(SRCDIR)/gSP.cpp               \
    $(SRCDIR)/gSPNeon.cpp           \
    $(SRCDIR)/L3D.cpp               \
    $(SRCDIR)/L3DEX.cpp             \
    $(SRCDIR)/L3DEX2.cpp            \
    $(SRCDIR)/N64.cpp               \
    $(SRCDIR)/OpenGL.cpp            \
    $(SRCDIR)/RDP.cpp               \
    $(SRCDIR)/RSP.cpp               \
    $(SRCDIR)/S2DEX.cpp             \
    $(SRCDIR)/S2DEX2.cpp            \
    $(SRCDIR)/ShaderCombiner.cpp    \
    $(SRCDIR)/Textures.cpp          \
    $(SRCDIR)/VI.cpp                \

# generate a list of object files build, make a temporary directory for them
OBJECTS := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(filter %.c, $(SOURCE)))
OBJECTS += $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(filter %.cpp, $(SOURCE)))
OBJDIRS = $(dir $(OBJECTS))
$(shell $(MKDIR) $(OBJDIRS))

# build targets
TARGET = mupen64plus-video-gles2n64-pi$(POSTFIX).$(SO_EXTENSION)

all: $(TARGET)

install: $(TARGET)
	$(INSTALL) -d "$(DESTDIR)$(PLUGINDIR)"
	$(INSTALL) -m 0644 $(INSTALL_STRIP_FLAG) $(TARGET) "$(DESTDIR)$(PLUGINDIR)"
	$(INSTALL) -d "$(DESTDIR)$(SHAREDIR)"

clean:
	$(RM) -r $(OBJDIR) $(TARGET)

# build dependency files
CFLAGS += -MD
-include $(OBJECTS:.o=.d)

CXXFLAGS += $(CFLAGS)

# standard build rules
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(COMPILE.c) -o $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(COMPILE.cc) -o $@ $<

$(TARGET): $(OBJECTS)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

.PHONY: all clean install uninstall targets

