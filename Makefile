## a simple Makefile to build Acinonyx as an R dynamic library
## currently it uses Mac back-end only

ASRC = Acinonyx/AContainer.cpp Acinonyx/AObject.cpp Acinonyx/RIF/RCalls.cpp Acinonyx/RIF/REngine.cpp Acinonyx/RIF/RGrDevice.cpp
CSRC = Acinonyx/ATools.c
MMSRC = Cocoa/CocoaApp.mm Cocoa/CocoaView.mm Cocoa/CocoaWindow.mm
MSRC = Cocoa/GLString.m
GLUTSRC = GLUT/AGLUTWindow.cpp
WINSRC = Win32/AWin32Window.cpp

OBJ = $(ASRC:%.cpp=%.o) $(CSRC:%.c=%.o) $(MMSRC:%.mm=%.o) $(MSRC:%.m=%.o)

FOBJ=AContainer.o AObject.o ATools.o CocoaApp.o CocoaView.o CocoaWindow.o RCalls.o REngine.o GLString.o


Acinonyx.so: $(ASRC) $(CSRC) $(MMSRC) $(MSRC)
	PKG_CPPFLAGS='$(DEBUG) -IAcinonyx -IAcinonyx/RIF -ICocoa' R CMD SHLIB -o $@ $^

glut.so: $(ASRC) $(CSRC) $(GLUTSRC)
	PKG_LIBS='-framework GLUT -framework OpenGL' PKG_CPPFLAGS='-IAcinonyx -IAcinonyx/RIF -IGLUT -DGLUT' R CMD SHLIB -o $@ $^

Acinonyx.dll: $(ASRC) $(CSRC) $(WINSRC)
	PKG_CXXFLAGS='-fpermissive' PKG_LIBS='-lopengl32 -lglu32 -lrgraphapp -lgdi32' PKG_CPPFLAGS='$(DEBUG) -IAcinonyx -IAcinonyx/RIF -IWin32' R CMD SHLIB -o $@ $^

#Acinonyx.so: $(ASRC) $(CSRC) $(MMSRC) $(MSRC)
#	g++ -c -IAcinonyx -IAcinonyx/RIF -ICocoa -I/Library/Frameworks/R.framework/Headers -g -O0 $^
#	g++ -dynamiclib -Wl,-headerpad_max_install_names -mmacosx-version-min=10.4 -undefined dynamic_lookup -single_module -multiply_defined suppress -o $@ $(FOBJ)

clean:
	rm -rf $(OBJ) $(FOBJ) Acinonyx.so glut.so
