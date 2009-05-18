## a simple Makefile to build Acinonyx as an R dynamic library
## currently it uses Mac back-end only

ASRC = Acinonyx/AContainer.cpp Acinonyx/AObject.cpp Acinonyx/RIF/RCalls.cpp Acinonyx/RIF/REngine.cpp
CSRC = Acinonyx/ATools.c
MACSRC = Cocoa/CocoaApp.mm Cocoa/CocoaView.mm Cocoa/CocoaWindow.mm

OBJ = $(ASRC:%.cpp=%.o) $(CSRC:%.c=%.o) $(MACSRC:%.mm=%.o)

Acinonyx.so: $(ASRC) $(CSRC) $(MACSRC)
	PKG_CPPFLAGS='-IAcinonyx -IAcinonyx/RIF -ICocoa' R CMD SHLIB -o $@ $^

clean:
	rm -rf $(OBJ) Acynonyx.so
