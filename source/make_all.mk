$(shell mkdir -p $(BINDIR))
.PHONY: all
all: $(BINDIR)fgbl 
FLAGSLibTpDlib =  -w -DDLIB_ISO_CPP_ONLY -isystem LibTpDlib/
SDIRLibTpDlib = LibTpDlib/
ODIRLibTpDlib = $(BUILDIR)LibTpDlib/
$(shell mkdir -p $(ODIRLibTpDlib))
INCSLibTpDlib := $(wildcard LibTpDlib/*.hpp) 
$(BUILDIR)LibTpDlib.a: $(ODIRLibTpDlib)dlib_all_source.o 
	$(AR) rc $(BUILDIR)LibTpDlib.a $(ODIRLibTpDlib)dlib_all_source.o 
	$(RANLIB) $(BUILDIR)LibTpDlib.a
$(ODIRLibTpDlib)dlib_all_source.o: $(SDIRLibTpDlib)dlib/all/source.cpp $(INCSLibTpDlib)
	$(CXX) -o $(ODIRLibTpDlib)dlib_all_source.o -c $(CXXFLAGS) $(FLAGSLibTpDlib) $(SDIRLibTpDlib)dlib/all/source.cpp
FLAGSLibFgBase =  -Wall -Wextra -ILibFgBase/src/ -isystem LibTpDlib/ -isystem LibTpStb/ -isystem LibTpEigen/
SDIRLibFgBase = LibFgBase/src/
ODIRLibFgBase = $(BUILDIR)LibFgBase/
$(shell mkdir -p $(ODIRLibFgBase))
INCSLibFgBase := $(wildcard LibFgBase/src/*.hpp) $(wildcard LibTpDlib/*.hpp) $(wildcard LibTpStb/*.hpp) $(wildcard LibTpEigen/Eigen/*.hpp) 
$(BUILDIR)LibFgBase.a: $(ODIRLibFgBase)Fg3dDisplay.o $(ODIRLibFgBase)Fg3dMesh.o $(ODIRLibFgBase)Fg3dMesh3ds.o $(ODIRLibFgBase)Fg3dMeshDae.o $(ODIRLibFgBase)Fg3dMeshFbx.o $(ODIRLibFgBase)Fg3dMeshIo.o $(ODIRLibFgBase)Fg3dMeshLegacy.o $(ODIRLibFgBase)Fg3dMeshLwo.o $(ODIRLibFgBase)Fg3dMeshMa.o $(ODIRLibFgBase)Fg3dMeshObj.o $(ODIRLibFgBase)Fg3dMeshPly.o $(ODIRLibFgBase)Fg3dMeshStl.o $(ODIRLibFgBase)Fg3dMeshTri.o $(ODIRLibFgBase)Fg3dMeshVrml.o $(ODIRLibFgBase)Fg3dMeshXsi.o $(ODIRLibFgBase)Fg3dSurface.o $(ODIRLibFgBase)FgAnthropometry.o $(ODIRLibFgBase)FgApproxFunc.o $(ODIRLibFgBase)FgBuild.o $(ODIRLibFgBase)FgBuildMakefiles.o $(ODIRLibFgBase)FgBuildVisualStudioSln.o $(ODIRLibFgBase)FgCamera.o $(ODIRLibFgBase)FgCl.o $(ODIRLibFgBase)FgCmdBase.o $(ODIRLibFgBase)FgCmdImage.o $(ODIRLibFgBase)FgCmdMesh.o $(ODIRLibFgBase)FgCmdMorph.o $(ODIRLibFgBase)FgCmdRender.o $(ODIRLibFgBase)FgCmdTest.o $(ODIRLibFgBase)FgCmdView.o $(ODIRLibFgBase)FgCommand.o $(ODIRLibFgBase)FgDataflow.o $(ODIRLibFgBase)FgDiagnostics.o $(ODIRLibFgBase)FgFile.o $(ODIRLibFgBase)FgFileSystem.o $(ODIRLibFgBase)FgGeometry.o $(ODIRLibFgBase)FgGridIndex.o $(ODIRLibFgBase)FgGuiApi.o $(ODIRLibFgBase)FgGuiApi3d.o $(ODIRLibFgBase)FgGuiApiCheckbox.o $(ODIRLibFgBase)FgGuiApiDialogs.o $(ODIRLibFgBase)FgGuiApiImage.o $(ODIRLibFgBase)FgGuiApiRadio.o $(ODIRLibFgBase)FgGuiApiSlider.o $(ODIRLibFgBase)FgGuiApiSplit.o $(ODIRLibFgBase)FgGuiApiText.o $(ODIRLibFgBase)FgImage.o $(ODIRLibFgBase)FgImageDraw.o $(ODIRLibFgBase)FgImageIo.o $(ODIRLibFgBase)FgImageIoStb.o $(ODIRLibFgBase)FgImageTest.o $(ODIRLibFgBase)FgImgDisplay.o $(ODIRLibFgBase)FgKdTree.o $(ODIRLibFgBase)FgMain.o $(ODIRLibFgBase)FgMath.o $(ODIRLibFgBase)FgMatrixC.o $(ODIRLibFgBase)FgMatrixSolver.o $(ODIRLibFgBase)FgMatrixSolverEigen.o $(ODIRLibFgBase)FgMatrixV.o $(ODIRLibFgBase)FgNc.o $(ODIRLibFgBase)FgParse.o $(ODIRLibFgBase)FgRandom.o $(ODIRLibFgBase)FgRender.o $(ODIRLibFgBase)FgSerial.o $(ODIRLibFgBase)FgStdExtensions.o $(ODIRLibFgBase)FgString.o $(ODIRLibFgBase)FgStringTest.o $(ODIRLibFgBase)FgTcpTest.o $(ODIRLibFgBase)FgTestUtils.o $(ODIRLibFgBase)FgTime.o $(ODIRLibFgBase)FgTopology.o $(ODIRLibFgBase)FgTransform.o $(ODIRLibFgBase)FgTypes.o $(ODIRLibFgBase)FgVolume.o $(ODIRLibFgBase)MurmurHash2.o $(ODIRLibFgBase)stdafx.o $(ODIRLibFgBase)nix_FgConioNix.o $(ODIRLibFgBase)nix_FgFileSystemNix.o $(ODIRLibFgBase)nix_FgGuiNix.o $(ODIRLibFgBase)nix_FgSystemNix.o $(ODIRLibFgBase)nix_FgTcpNix.o $(ODIRLibFgBase)nix_FgTimeNix.o 
	$(AR) rc $(BUILDIR)LibFgBase.a $(ODIRLibFgBase)Fg3dDisplay.o $(ODIRLibFgBase)Fg3dMesh.o $(ODIRLibFgBase)Fg3dMesh3ds.o $(ODIRLibFgBase)Fg3dMeshDae.o $(ODIRLibFgBase)Fg3dMeshFbx.o $(ODIRLibFgBase)Fg3dMeshIo.o $(ODIRLibFgBase)Fg3dMeshLegacy.o $(ODIRLibFgBase)Fg3dMeshLwo.o $(ODIRLibFgBase)Fg3dMeshMa.o $(ODIRLibFgBase)Fg3dMeshObj.o $(ODIRLibFgBase)Fg3dMeshPly.o $(ODIRLibFgBase)Fg3dMeshStl.o $(ODIRLibFgBase)Fg3dMeshTri.o $(ODIRLibFgBase)Fg3dMeshVrml.o $(ODIRLibFgBase)Fg3dMeshXsi.o $(ODIRLibFgBase)Fg3dSurface.o $(ODIRLibFgBase)FgAnthropometry.o $(ODIRLibFgBase)FgApproxFunc.o $(ODIRLibFgBase)FgBuild.o $(ODIRLibFgBase)FgBuildMakefiles.o $(ODIRLibFgBase)FgBuildVisualStudioSln.o $(ODIRLibFgBase)FgCamera.o $(ODIRLibFgBase)FgCl.o $(ODIRLibFgBase)FgCmdBase.o $(ODIRLibFgBase)FgCmdImage.o $(ODIRLibFgBase)FgCmdMesh.o $(ODIRLibFgBase)FgCmdMorph.o $(ODIRLibFgBase)FgCmdRender.o $(ODIRLibFgBase)FgCmdTest.o $(ODIRLibFgBase)FgCmdView.o $(ODIRLibFgBase)FgCommand.o $(ODIRLibFgBase)FgDataflow.o $(ODIRLibFgBase)FgDiagnostics.o $(ODIRLibFgBase)FgFile.o $(ODIRLibFgBase)FgFileSystem.o $(ODIRLibFgBase)FgGeometry.o $(ODIRLibFgBase)FgGridIndex.o $(ODIRLibFgBase)FgGuiApi.o $(ODIRLibFgBase)FgGuiApi3d.o $(ODIRLibFgBase)FgGuiApiCheckbox.o $(ODIRLibFgBase)FgGuiApiDialogs.o $(ODIRLibFgBase)FgGuiApiImage.o $(ODIRLibFgBase)FgGuiApiRadio.o $(ODIRLibFgBase)FgGuiApiSlider.o $(ODIRLibFgBase)FgGuiApiSplit.o $(ODIRLibFgBase)FgGuiApiText.o $(ODIRLibFgBase)FgImage.o $(ODIRLibFgBase)FgImageDraw.o $(ODIRLibFgBase)FgImageIo.o $(ODIRLibFgBase)FgImageIoStb.o $(ODIRLibFgBase)FgImageTest.o $(ODIRLibFgBase)FgImgDisplay.o $(ODIRLibFgBase)FgKdTree.o $(ODIRLibFgBase)FgMain.o $(ODIRLibFgBase)FgMath.o $(ODIRLibFgBase)FgMatrixC.o $(ODIRLibFgBase)FgMatrixSolver.o $(ODIRLibFgBase)FgMatrixSolverEigen.o $(ODIRLibFgBase)FgMatrixV.o $(ODIRLibFgBase)FgNc.o $(ODIRLibFgBase)FgParse.o $(ODIRLibFgBase)FgRandom.o $(ODIRLibFgBase)FgRender.o $(ODIRLibFgBase)FgSerial.o $(ODIRLibFgBase)FgStdExtensions.o $(ODIRLibFgBase)FgString.o $(ODIRLibFgBase)FgStringTest.o $(ODIRLibFgBase)FgTcpTest.o $(ODIRLibFgBase)FgTestUtils.o $(ODIRLibFgBase)FgTime.o $(ODIRLibFgBase)FgTopology.o $(ODIRLibFgBase)FgTransform.o $(ODIRLibFgBase)FgTypes.o $(ODIRLibFgBase)FgVolume.o $(ODIRLibFgBase)MurmurHash2.o $(ODIRLibFgBase)stdafx.o $(ODIRLibFgBase)nix_FgConioNix.o $(ODIRLibFgBase)nix_FgFileSystemNix.o $(ODIRLibFgBase)nix_FgGuiNix.o $(ODIRLibFgBase)nix_FgSystemNix.o $(ODIRLibFgBase)nix_FgTcpNix.o $(ODIRLibFgBase)nix_FgTimeNix.o 
	$(RANLIB) $(BUILDIR)LibFgBase.a
$(ODIRLibFgBase)Fg3dDisplay.o: $(SDIRLibFgBase)Fg3dDisplay.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dDisplay.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dDisplay.cpp
$(ODIRLibFgBase)Fg3dMesh.o: $(SDIRLibFgBase)Fg3dMesh.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMesh.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMesh.cpp
$(ODIRLibFgBase)Fg3dMesh3ds.o: $(SDIRLibFgBase)Fg3dMesh3ds.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMesh3ds.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMesh3ds.cpp
$(ODIRLibFgBase)Fg3dMeshDae.o: $(SDIRLibFgBase)Fg3dMeshDae.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMeshDae.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMeshDae.cpp
$(ODIRLibFgBase)Fg3dMeshFbx.o: $(SDIRLibFgBase)Fg3dMeshFbx.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMeshFbx.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMeshFbx.cpp
$(ODIRLibFgBase)Fg3dMeshIo.o: $(SDIRLibFgBase)Fg3dMeshIo.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMeshIo.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMeshIo.cpp
$(ODIRLibFgBase)Fg3dMeshLegacy.o: $(SDIRLibFgBase)Fg3dMeshLegacy.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMeshLegacy.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMeshLegacy.cpp
$(ODIRLibFgBase)Fg3dMeshLwo.o: $(SDIRLibFgBase)Fg3dMeshLwo.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMeshLwo.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMeshLwo.cpp
$(ODIRLibFgBase)Fg3dMeshMa.o: $(SDIRLibFgBase)Fg3dMeshMa.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMeshMa.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMeshMa.cpp
$(ODIRLibFgBase)Fg3dMeshObj.o: $(SDIRLibFgBase)Fg3dMeshObj.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMeshObj.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMeshObj.cpp
$(ODIRLibFgBase)Fg3dMeshPly.o: $(SDIRLibFgBase)Fg3dMeshPly.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMeshPly.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMeshPly.cpp
$(ODIRLibFgBase)Fg3dMeshStl.o: $(SDIRLibFgBase)Fg3dMeshStl.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMeshStl.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMeshStl.cpp
$(ODIRLibFgBase)Fg3dMeshTri.o: $(SDIRLibFgBase)Fg3dMeshTri.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMeshTri.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMeshTri.cpp
$(ODIRLibFgBase)Fg3dMeshVrml.o: $(SDIRLibFgBase)Fg3dMeshVrml.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMeshVrml.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMeshVrml.cpp
$(ODIRLibFgBase)Fg3dMeshXsi.o: $(SDIRLibFgBase)Fg3dMeshXsi.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMeshXsi.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMeshXsi.cpp
$(ODIRLibFgBase)Fg3dSurface.o: $(SDIRLibFgBase)Fg3dSurface.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dSurface.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dSurface.cpp
$(ODIRLibFgBase)FgAnthropometry.o: $(SDIRLibFgBase)FgAnthropometry.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgAnthropometry.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgAnthropometry.cpp
$(ODIRLibFgBase)FgApproxFunc.o: $(SDIRLibFgBase)FgApproxFunc.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgApproxFunc.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgApproxFunc.cpp
$(ODIRLibFgBase)FgBuild.o: $(SDIRLibFgBase)FgBuild.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgBuild.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgBuild.cpp
$(ODIRLibFgBase)FgBuildMakefiles.o: $(SDIRLibFgBase)FgBuildMakefiles.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgBuildMakefiles.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgBuildMakefiles.cpp
$(ODIRLibFgBase)FgBuildVisualStudioSln.o: $(SDIRLibFgBase)FgBuildVisualStudioSln.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgBuildVisualStudioSln.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgBuildVisualStudioSln.cpp
$(ODIRLibFgBase)FgCamera.o: $(SDIRLibFgBase)FgCamera.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgCamera.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgCamera.cpp
$(ODIRLibFgBase)FgCl.o: $(SDIRLibFgBase)FgCl.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgCl.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgCl.cpp
$(ODIRLibFgBase)FgCmdBase.o: $(SDIRLibFgBase)FgCmdBase.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgCmdBase.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgCmdBase.cpp
$(ODIRLibFgBase)FgCmdImage.o: $(SDIRLibFgBase)FgCmdImage.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgCmdImage.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgCmdImage.cpp
$(ODIRLibFgBase)FgCmdMesh.o: $(SDIRLibFgBase)FgCmdMesh.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgCmdMesh.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgCmdMesh.cpp
$(ODIRLibFgBase)FgCmdMorph.o: $(SDIRLibFgBase)FgCmdMorph.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgCmdMorph.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgCmdMorph.cpp
$(ODIRLibFgBase)FgCmdRender.o: $(SDIRLibFgBase)FgCmdRender.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgCmdRender.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgCmdRender.cpp
$(ODIRLibFgBase)FgCmdTest.o: $(SDIRLibFgBase)FgCmdTest.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgCmdTest.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgCmdTest.cpp
$(ODIRLibFgBase)FgCmdView.o: $(SDIRLibFgBase)FgCmdView.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgCmdView.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgCmdView.cpp
$(ODIRLibFgBase)FgCommand.o: $(SDIRLibFgBase)FgCommand.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgCommand.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgCommand.cpp
$(ODIRLibFgBase)FgDataflow.o: $(SDIRLibFgBase)FgDataflow.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgDataflow.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgDataflow.cpp
$(ODIRLibFgBase)FgDiagnostics.o: $(SDIRLibFgBase)FgDiagnostics.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgDiagnostics.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgDiagnostics.cpp
$(ODIRLibFgBase)FgFile.o: $(SDIRLibFgBase)FgFile.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgFile.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgFile.cpp
$(ODIRLibFgBase)FgFileSystem.o: $(SDIRLibFgBase)FgFileSystem.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgFileSystem.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgFileSystem.cpp
$(ODIRLibFgBase)FgGeometry.o: $(SDIRLibFgBase)FgGeometry.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgGeometry.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgGeometry.cpp
$(ODIRLibFgBase)FgGridIndex.o: $(SDIRLibFgBase)FgGridIndex.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgGridIndex.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgGridIndex.cpp
$(ODIRLibFgBase)FgGuiApi.o: $(SDIRLibFgBase)FgGuiApi.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgGuiApi.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgGuiApi.cpp
$(ODIRLibFgBase)FgGuiApi3d.o: $(SDIRLibFgBase)FgGuiApi3d.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgGuiApi3d.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgGuiApi3d.cpp
$(ODIRLibFgBase)FgGuiApiCheckbox.o: $(SDIRLibFgBase)FgGuiApiCheckbox.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgGuiApiCheckbox.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgGuiApiCheckbox.cpp
$(ODIRLibFgBase)FgGuiApiDialogs.o: $(SDIRLibFgBase)FgGuiApiDialogs.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgGuiApiDialogs.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgGuiApiDialogs.cpp
$(ODIRLibFgBase)FgGuiApiImage.o: $(SDIRLibFgBase)FgGuiApiImage.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgGuiApiImage.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgGuiApiImage.cpp
$(ODIRLibFgBase)FgGuiApiRadio.o: $(SDIRLibFgBase)FgGuiApiRadio.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgGuiApiRadio.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgGuiApiRadio.cpp
$(ODIRLibFgBase)FgGuiApiSlider.o: $(SDIRLibFgBase)FgGuiApiSlider.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgGuiApiSlider.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgGuiApiSlider.cpp
$(ODIRLibFgBase)FgGuiApiSplit.o: $(SDIRLibFgBase)FgGuiApiSplit.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgGuiApiSplit.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgGuiApiSplit.cpp
$(ODIRLibFgBase)FgGuiApiText.o: $(SDIRLibFgBase)FgGuiApiText.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgGuiApiText.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgGuiApiText.cpp
$(ODIRLibFgBase)FgImage.o: $(SDIRLibFgBase)FgImage.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgImage.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgImage.cpp
$(ODIRLibFgBase)FgImageDraw.o: $(SDIRLibFgBase)FgImageDraw.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgImageDraw.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgImageDraw.cpp
$(ODIRLibFgBase)FgImageIo.o: $(SDIRLibFgBase)FgImageIo.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgImageIo.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgImageIo.cpp
$(ODIRLibFgBase)FgImageIoStb.o: $(SDIRLibFgBase)FgImageIoStb.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgImageIoStb.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgImageIoStb.cpp
$(ODIRLibFgBase)FgImageTest.o: $(SDIRLibFgBase)FgImageTest.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgImageTest.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgImageTest.cpp
$(ODIRLibFgBase)FgImgDisplay.o: $(SDIRLibFgBase)FgImgDisplay.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgImgDisplay.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgImgDisplay.cpp
$(ODIRLibFgBase)FgKdTree.o: $(SDIRLibFgBase)FgKdTree.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgKdTree.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgKdTree.cpp
$(ODIRLibFgBase)FgMain.o: $(SDIRLibFgBase)FgMain.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgMain.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgMain.cpp
$(ODIRLibFgBase)FgMath.o: $(SDIRLibFgBase)FgMath.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgMath.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgMath.cpp
$(ODIRLibFgBase)FgMatrixC.o: $(SDIRLibFgBase)FgMatrixC.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgMatrixC.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgMatrixC.cpp
$(ODIRLibFgBase)FgMatrixSolver.o: $(SDIRLibFgBase)FgMatrixSolver.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgMatrixSolver.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgMatrixSolver.cpp
$(ODIRLibFgBase)FgMatrixSolverEigen.o: $(SDIRLibFgBase)FgMatrixSolverEigen.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgMatrixSolverEigen.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgMatrixSolverEigen.cpp
$(ODIRLibFgBase)FgMatrixV.o: $(SDIRLibFgBase)FgMatrixV.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgMatrixV.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgMatrixV.cpp
$(ODIRLibFgBase)FgNc.o: $(SDIRLibFgBase)FgNc.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgNc.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgNc.cpp
$(ODIRLibFgBase)FgParse.o: $(SDIRLibFgBase)FgParse.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgParse.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgParse.cpp
$(ODIRLibFgBase)FgRandom.o: $(SDIRLibFgBase)FgRandom.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgRandom.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgRandom.cpp
$(ODIRLibFgBase)FgRender.o: $(SDIRLibFgBase)FgRender.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgRender.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgRender.cpp
$(ODIRLibFgBase)FgSerial.o: $(SDIRLibFgBase)FgSerial.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgSerial.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgSerial.cpp
$(ODIRLibFgBase)FgStdExtensions.o: $(SDIRLibFgBase)FgStdExtensions.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgStdExtensions.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgStdExtensions.cpp
$(ODIRLibFgBase)FgString.o: $(SDIRLibFgBase)FgString.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgString.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgString.cpp
$(ODIRLibFgBase)FgStringTest.o: $(SDIRLibFgBase)FgStringTest.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgStringTest.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgStringTest.cpp
$(ODIRLibFgBase)FgTcpTest.o: $(SDIRLibFgBase)FgTcpTest.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgTcpTest.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgTcpTest.cpp
$(ODIRLibFgBase)FgTestUtils.o: $(SDIRLibFgBase)FgTestUtils.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgTestUtils.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgTestUtils.cpp
$(ODIRLibFgBase)FgTime.o: $(SDIRLibFgBase)FgTime.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgTime.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgTime.cpp
$(ODIRLibFgBase)FgTopology.o: $(SDIRLibFgBase)FgTopology.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgTopology.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgTopology.cpp
$(ODIRLibFgBase)FgTransform.o: $(SDIRLibFgBase)FgTransform.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgTransform.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgTransform.cpp
$(ODIRLibFgBase)FgTypes.o: $(SDIRLibFgBase)FgTypes.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgTypes.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgTypes.cpp
$(ODIRLibFgBase)FgVolume.o: $(SDIRLibFgBase)FgVolume.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgVolume.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgVolume.cpp
$(ODIRLibFgBase)MurmurHash2.o: $(SDIRLibFgBase)MurmurHash2.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)MurmurHash2.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)MurmurHash2.cpp
$(ODIRLibFgBase)stdafx.o: $(SDIRLibFgBase)stdafx.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)stdafx.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)stdafx.cpp
$(ODIRLibFgBase)nix_FgConioNix.o: $(SDIRLibFgBase)nix/FgConioNix.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)nix_FgConioNix.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)nix/FgConioNix.cpp
$(ODIRLibFgBase)nix_FgFileSystemNix.o: $(SDIRLibFgBase)nix/FgFileSystemNix.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)nix_FgFileSystemNix.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)nix/FgFileSystemNix.cpp
$(ODIRLibFgBase)nix_FgGuiNix.o: $(SDIRLibFgBase)nix/FgGuiNix.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)nix_FgGuiNix.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)nix/FgGuiNix.cpp
$(ODIRLibFgBase)nix_FgSystemNix.o: $(SDIRLibFgBase)nix/FgSystemNix.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)nix_FgSystemNix.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)nix/FgSystemNix.cpp
$(ODIRLibFgBase)nix_FgTcpNix.o: $(SDIRLibFgBase)nix/FgTcpNix.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)nix_FgTcpNix.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)nix/FgTcpNix.cpp
$(ODIRLibFgBase)nix_FgTimeNix.o: $(SDIRLibFgBase)nix/FgTimeNix.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)nix_FgTimeNix.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)nix/FgTimeNix.cpp
FLAGSfgbl =  -Wall -Wextra -ILibFgBase/src/ -isystem LibTpDlib/
SDIRfgbl = fgbl/
ODIRfgbl = $(BUILDIR)fgbl/
$(shell mkdir -p $(ODIRfgbl))
INCSfgbl := $(wildcard LibFgBase/src/*.hpp) $(wildcard LibTpDlib/*.hpp) 
$(BINDIR)fgbl: $(ODIRfgbl)fgbl.o $(BUILDIR)LibFgBase.a $(BUILDIR)LibTpDlib.a 
	$(LINK) $(LFLAGS) -o $(BINDIR)fgbl $(ODIRfgbl)fgbl.o $(BUILDIR)LibFgBase.a $(BUILDIR)LibTpDlib.a 
$(ODIRfgbl)fgbl.o: $(SDIRfgbl)fgbl.cpp $(INCSfgbl)
	$(CXX) -o $(ODIRfgbl)fgbl.o -c $(CXXFLAGS) $(FLAGSfgbl) $(SDIRfgbl)fgbl.cpp
.PHONY: clean cleanObjs cleanTargs
clean: cleanObjs cleanTargs
cleanObjs:
	rm -r $(BUILDIR)
cleanTargs:
	rm -r $(BINDIR)
