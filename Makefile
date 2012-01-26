## a simple Makefile to build Acinonyx as an R dynamic library
## (requires GNU Make compatible due to ifeq/endif)
##
## Default configurations:
## Windows  : Win32 graphapp UI (built-in FreeType)
## Mac OS X : Cocoa
## others   : X11 + FreeType
##
## GLUT support is optional, unused and untested

ASRC = Acinonyx/AContainer.cpp Acinonyx/AObject.cpp Acinonyx/RIF/RCalls.cpp Acinonyx/RIF/REngine.cpp Acinonyx/RIF/RGrDevice.cpp
CSRC = Acinonyx/ATools.c
MMSRC = Cocoa/CocoaApp.mm Cocoa/CocoaView.mm Cocoa/CocoaWindow.mm
MSRC = Cocoa/GLString.m
GLUTSRC = GLUT/AGLUTWindow.cpp
X11SRC = X11/AX11Window.cpp
WINSRC = Win32/AWin32Window.cpp
RSRC=$(shell ls R/*.R | grep -v entry)

REP=R/entry_points.R

OBJ = $(ASRC:%.cpp=%.o) $(CSRC:%.c=%.o) $(MMSRC:%.mm=%.o) $(MSRC:%.m=%.o) $(WINSRC:%.cpp=%.o) $(X11SRC:%.cpp=%.o)

FOBJ=AContainer.o AObject.o ATools.o CocoaApp.o CocoaView.o CocoaWindow.o RCalls.o REngine.o GLString.o AWin32Window.o AX11Window.o

FTCF=`freetype-config --cflags`
FTLIB=`freetype-config --libs`

OS=$(shell uname || echo Windows)
ifeq ($(R_HOME),)
RBIN=R
else
RBIN=$(R_HOME)/bin/R
endif

ifeq ($(WINARCH),)
WINARCH=32
endif

ifeq ($(OS),Darwin)
ifeq ($(USE_X11),1)
Acinonyx.so: $(ASRC) $(CSRC) $(X11SRC)
	PKG_MAKE_DSYM=1 PKG_LIBS='-L/usr/X11/lib -lX11 -lGLU -lGL $(FTLIB)' PKG_CPPFLAGS='$(DEBUGCPPFLAGS) -IAcinonyx -IAcinonyx/RIF -IX11 -I/usr/X11/include -DUSE_X11 $(FTCF)' R CMD SHLIB -o $@ $^
else
Acinonyx.so: $(ASRC) $(CSRC) $(MMSRC) $(MSRC)
	PKG_MAKE_DSYM=1 PKG_CPPFLAGS='$(DEBUG) -IAcinonyx -IAcinonyx/RIF -ICocoa' PKG_LIBS='-framework Cocoa -framework OpenGL' R CMD SHLIB -o $@ $^
endif
else
Acinonyx.so: $(ASRC) $(CSRC) $(X11SRC)
	PKG_LIBS='-lX11 -lGLU -lGL $(FTLIB)' PKG_CPPFLAGS='-IAcinonyx -IAcinonyx/RIF -IX11 -DUSE_X11 $(FTCF)' R CMD SHLIB -o $@ $^
endif

glut.so: $(ASRC) $(CSRC) $(GLUTSRC)
	PKG_LIBS='-framework GLUT -framework OpenGL' PKG_CPPFLAGS='-IAcinonyx -IAcinonyx/RIF -IGLUT -DGLUT' R CMD SHLIB -o $@ $^

Acinonyx.dll: $(ASRC) $(CSRC) $(WINSRC)
	PKG_CXXFLAGS='-fpermissive' PKG_LIBS='-lopengl32 -lglu32 -lrgraphapp -lgdi32 Win32/lib$(WINARCH)/libfreetype.a -static-libgcc -static-libstdc++' PKG_CPPFLAGS='$(DEBUG) -IAcinonyx -IAcinonyx/RIF -IWin32' $(RBIN) CMD SHLIB -o $@ $^

$(REP): $(RSRC)
	cat $(RSRC) | sed -n 's:.*\.Call("\{0,1\}\(A_[^" ,)]\{1,\}\).*:\1:p' | sort | uniq > /tmp/entry_points
	( echo '.sym <- '; Rscript -e 'dput(readLines("/tmp/entry_points"))') > $@
	rm -f /tmp/entry_points

sym: $(REP)

clean:
	rm -rf $(OBJ) $(FOBJ) Acinonyx.so Acinonyx.dll glut.so
