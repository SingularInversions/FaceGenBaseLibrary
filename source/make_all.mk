$(shell mkdir -p $(BINDIR))
.PHONY: all
all: $(BUILDIR)LibFgBase.a 
FLAGSLibTpBoost =  -w -DBOOST_ALL_NO_LIB -DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE -ILibTpBoost/boost_1_67_0/
SDIRLibTpBoost = LibTpBoost/boost_1_67_0/
ODIRLibTpBoost = $(BUILDIR)LibTpBoost/
$(shell mkdir -p $(ODIRLibTpBoost))
INCSLibTpBoost := $(wildcard LibTpBoost/boost_1_67_0/boost/*.hpp) 
$(BUILDIR)LibTpBoost.a: 
	$(AR) rc $(BUILDIR)LibTpBoost.a 
	$(RANLIB) $(BUILDIR)LibTpBoost.a
FLAGSLibJpegIjg6b =  -w -ILibJpegIjg6b/
SDIRLibJpegIjg6b = LibJpegIjg6b/
ODIRLibJpegIjg6b = $(BUILDIR)LibJpegIjg6b/
$(shell mkdir -p $(ODIRLibJpegIjg6b))
INCSLibJpegIjg6b := $(wildcard LibJpegIjg6b/*.hpp) 
$(BUILDIR)LibJpegIjg6b.a: 
	$(AR) rc $(BUILDIR)LibJpegIjg6b.a 
	$(RANLIB) $(BUILDIR)LibJpegIjg6b.a
FLAGSLibFgBase =  -Wall -Wextra -DBOOST_ALL_NO_LIB -DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE -ILibFgBase/src/ -ILibTpEigen/ -ILibJpegIjg6b/ -ILibTpStb/stb/ -ILibTpBoost/boost_1_67_0/
SDIRLibFgBase = LibFgBase/src/
ODIRLibFgBase = $(BUILDIR)LibFgBase/
$(shell mkdir -p $(ODIRLibFgBase))
INCSLibFgBase := $(wildcard LibFgBase/src/*.hpp) $(wildcard LibTpEigen/Eigen/*.hpp) $(wildcard LibJpegIjg6b/*.hpp) $(wildcard LibTpStb/stb/*.hpp) $(wildcard LibTpBoost/boost_1_67_0/boost/*.hpp) 
$(BUILDIR)LibFgBase.a: 
	$(AR) rc $(BUILDIR)LibFgBase.a 
	$(RANLIB) $(BUILDIR)LibFgBase.a
.PHONY: clean cleanObjs cleanTargs
clean: cleanObjs cleanTargs
cleanObjs:
	rm -r $(BUILDIR)
cleanTargs:
	rm -r $(BINDIR)
