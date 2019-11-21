//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Generate Visual Studio SLN and XPROJ files.
//
// NOTES:
// * Use of the exact number of tab characters is required for correct VS parsing behaviour.

#include "stdafx.h"

#include "FgStdStream.hpp"
#include "FgStdMap.hpp"
#include "FgCons.hpp"
#include "FgOut.hpp"
#include "FgException.hpp"
#include "FgFileSystem.hpp"
#include "FgBuild.hpp"
#include "FgString.hpp"
#include "FgTestUtils.hpp"

using namespace std;

namespace Fg {

static
string
fgConsVsPreprocessorDefs(
    bool                release,
    const FgConsProj &  proj,
    Strings const &      defs)
{
    string  ret = "WIN32";
    if (release)
        ret += ";NDEBUG";
    else
        ret += ";_DEBUG";
    if (proj.isGuiExecutable())
        ret += ";_WINDOWS";
    else if (proj.isClExecutable())
        ret += ";_CONSOLE";
    else if (proj.isDynamicLib())
        ret += ";_WINDOWS;_USRDLL;TESTDLL_EXPORTS";
    else if (proj.isStaticLib())
        ret += ";_LIB";
    else
        fgThrow("fgConsVsPreprocessorDefs unhandled type",proj.type);
    for (string const & def : defs)
        ret += ";" + def;
    return ret;
}

static
bool
writeVcxproj(
    const FgConsSolution &      sln,            // Transitive lookups
    const FgConsProj &          proj,
    const map<string,string> &  nameToGuid,     // Must contain all project names
    uint                        vsver)          // 13 - VS2013, 15 = VS2015, 17 = VS2017, 19 = VS2019
{
    Strings          includes = sln.getIncludes(proj.name,false),
                    defines = sln.getDefs(proj.name),
                    lnkDeps = sln.getLnkDeps(proj.name);
    string          verStr = fgToStringDigits(vsver,2),
                    projDir = proj.name + "/VisualStudio" + verStr + "/";
    fgCreateDirectory(projDir);
    string          projFile = projDir + proj.name + ".vcxproj";
    ostringstream   ofs;
    string          config[] = {"Debug","Release"};
    string          bits[] = {"Win32","x64"};
    string          toolsVersion = "4";
	if (vsver == 13)
		toolsVersion = "12";
	else if (vsver == 15)
		toolsVersion = "14";
	else if ((vsver == 17) || (vsver == 19))
		toolsVersion = "15";
    else
        fgThrow("writeVcxproj unhandled vsver",vsver);
    ofs << 
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
        "<Project DefaultTargets=\"Build\" ToolsVersion=\""+toolsVersion+".0\" xmlns=\""
            "http://schemas.microsoft.com/developer/msbuild/2003\">\n"
        "  <ItemGroup Label=\"ProjectConfigurations\">\n";
    for (uint cc=0; cc<2; ++cc)
        for (uint bb=0; bb<2; ++bb)
            ofs <<
                "    <ProjectConfiguration Include=\"" << config[cc] << "|" << bits[bb] << "\">\n"
                "      <Configuration>" << config[cc] << "</Configuration>\n"
                "      <Platform>" << bits[bb] << "</Platform>\n"
                "    </ProjectConfiguration>\n";
    ofs <<
        "  </ItemGroup>\n"
        "  <PropertyGroup Label=\"Globals\">\n"
        "    <ProjectGuid>" << fgLookup(nameToGuid,proj.name) << "</ProjectGuid>\n"
        "    <RootNamespace>" << proj.name << "</RootNamespace>\n"
        "    <Keyword>Win32Proj</Keyword>\n";
    if (vsver == 19)
        ofs << 
            "    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>\n";
    ofs <<
        "  </PropertyGroup>\n"
        "  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />\n";
    for (uint bb=0; bb<2; ++bb) {
        for (uint cc=0; cc<2; ++cc) {
            ofs <<
                "  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='" << config[cc]
                    << "|" << bits[bb] << "'\" Label=\"Configuration\">\n"
                "    <ConfigurationType>";
            if (proj.isExecutable())
                ofs << "Application";
            else if (proj.isDynamicLib())
                ofs << "DynamicLibrary";
            else
                ofs << "StaticLibrary";
            ofs <<
                "</ConfigurationType>\n";
            // PlatformToolset:
            if (vsver == 13)
                ofs << "    <PlatformToolset>v120</PlatformToolset>\n";
            else if (vsver == 15)
                ofs << "    <PlatformToolset>v140</PlatformToolset>\n";
			else if (vsver == 17)
				ofs << "    <PlatformToolset>v141</PlatformToolset>\n";
            else if (vsver == 19)
                ofs << "    <PlatformToolset>v142</PlatformToolset>\n";
            else
                fgThrow("writeVcxproj unhandled vsver",vsver);
            ofs <<
                "    <CharacterSet>Unicode</CharacterSet>\n"
                "  </PropertyGroup>\n";
        }
    }
    ofs <<
        "  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.props\" />\n"
        "  <ImportGroup Label=\"ExtensionSettings\">\n"
        "  </ImportGroup>\n";
    for (uint bb=0; bb<2; ++bb)
        for (uint cc=0; cc<2; ++cc)
            ofs <<
                "  <ImportGroup Condition=\"'$(Configuration)|$(Platform)'=='" << config[cc] << "|"
                    << bits[bb] << "'\" Label=\"PropertySheets\">\n"
                "    <Import Project=\"$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\" Condition=\""
                    "exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')\" Label=\""
                    "LocalAppDataPlatform\" />\n"
                "  </ImportGroup>\n";
    ofs <<
        "  <PropertyGroup Label=\"UserMacros\" />\n"
        "  <PropertyGroup>\n"
        "    <_ProjectFileVersion>10.0.40219.1</_ProjectFileVersion>\n";

    string      sourceToBinPath("..\\bin\\win\\");
    for (uint cc=0; cc<2; ++cc) {
        for (uint bb=0; bb<2; ++bb) {
            ofs <<
                "    <OutDir Condition=\"'$(Configuration)|$(Platform)'=='" << config[cc]
                    << "|" << bits[bb] << "'\">";
            if (proj.isLinked())
                ofs << "..\\..\\" << sourceToBinPath
                    << ((bb == 0) ? "x86\\" : "x64\\")
                    << "vs" << verStr << "\\"
                    << ((cc == 0) ? "debug\\" : "release\\");
            else
                ofs << "$(Platform)\\$(Configuration)\\";
            ofs << "</OutDir>\n"
                "    <IntDir Condition=\"'$(Configuration)|$(Platform)'=='" << config[cc]
                    << "|" << bits[bb] << "'\">$(Platform)\\$(Configuration)\\</IntDir>\n";
            if (proj.isLinked())
                ofs <<
                    "    <LinkIncremental Condition=\"'$(Configuration)|$(Platform)'=='" << config[cc]
                    << "|" << bits[bb] << "'\">" << ((cc == 0) ? "true" : "false") << "</LinkIncremental>\n";
        }
    }
    ofs <<
        "  </PropertyGroup>\n";
    bool        pch = false;
    if (!proj.srcGroups.empty() &&
        fileReadable(proj.name+"/"+proj.baseDir+proj.srcGroups[0].dir+"stdafx.cpp"))
        pch = true;
    for (uint cc=0; cc<2; ++cc) {
        for (uint bb=0; bb<2; ++bb) {
            ofs <<
                "  <ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='" << config[cc]
                << "|" << bits[bb] << "'\">\n";
            if (bb == 1)
                ofs <<
                    "    <Midl>\n"
                    "      <TargetEnvironment>X64</TargetEnvironment>\n"
                    "    </Midl>\n";
            ofs <<
                "    <ClCompile>\n"
                "      <AdditionalOptions>/Zm200 /bigobj %(AdditionalOptions)</AdditionalOptions>\n"
                "      <Optimization>" << ((cc == 0) ? "Disabled" : "MaxSpeed") << "</Optimization>\n";
            if (cc == 1)
                ofs <<
                    "      <IntrinsicFunctions>true</IntrinsicFunctions>\n";
            ofs <<
                "      <AdditionalIncludeDirectories>";
            for (string const & relPath : includes) {
                string          dir = fgReplace(relPath,'/','\\');
                ofs << "..\\" << dir << ";";    // .vcxproj file is in a subdir of project dir
            }
            ofs <<
                "%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>\n"
                "      <PreprocessorDefinitions>" << fgConsVsPreprocessorDefs((cc == 1),proj,defines)
                    << ";%(PreprocessorDefinitions)</PreprocessorDefinitions>\n";
            if (cc == 0)
                ofs <<
                    "      <BasicRuntimeChecks>EnableFastChecks</BasicRuntimeChecks>\n";
            ofs <<
                "      <RuntimeLibrary>MultiThreaded" << ((cc == 0) ? "Debug" : "") << "</RuntimeLibrary>\n";
            if (cc == 1)
                ofs <<
                    "      <FunctionLevelLinking>true</FunctionLevelLinking>\n";
            // Tried "EnableAllWarnings" but got tons of warning from boost include files:
            string      warnStr = (proj.warn == 0) ? "TurnOffAllWarnings" : "Level" + toString(proj.warn);
            ofs <<
                "      <PrecompiledHeader>" << (pch ? "Use" : "\n      ") << "</PrecompiledHeader>\n"
                "      <WarningLevel>" << warnStr << "</WarningLevel>\n"
                "      <DebugInformationFormat>" << ((cc == 0) ? "ProgramDatabase" : "\n      ") << "</DebugInformationFormat>\n"
                "      <WholeProgramOptimization>false</WholeProgramOptimization>\n"
                // Note that MSVC fast floating point model is an extension to the standard and thus cannot
                // be used in combination with disabling MSVC language extensions:
                "      <FloatingPointModel>Fast</FloatingPointModel>\n"
                // No downside to supporting OpenMP in Visual Studio:
                "      <OpenMPSupport>true</OpenMPSupport>\n";
            if (vsver >= 17)    // Improved error position indicator:
                ofs <<
                "      <DiagnosticsFormat>Caret</DiagnosticsFormat>\n";
            ofs <<
                "    </ClCompile>\n";
            if (proj.isLinked()) {
                ofs <<
                    "    <Link>\n"
                    "      <GenerateDebugInformation>" << ((cc == 0) ? "true" : "false")
                        << "</GenerateDebugInformation>\n"
                    "      <SubSystem>" << (proj.isGuiExecutable() ? "Windows" : "Console") << "</SubSystem>\n"
                    "      <TargetMachine>MachineX" << ((bb == 0) ? "86" : "64") << "</TargetMachine>\n"
                    "      <AdditionalDependencies>gdiplus.lib;comctl32.lib;user32.lib;gdi32.lib;"
                        "opengl32.lib;advapi32.lib;comdlg32.lib;Shell32.lib;";
                for (string const & dll : lnkDeps)
                    if (!sln.contains(dll))         // Binary-only DLL
                        ofs << dll << ".lib;";
                ofs
                    << "%(AdditionalDependencies)</AdditionalDependencies>\n"
                    // The SDK builder copies the preferred binaries into repo/bin/os/arch/ dirs:
                       "      <AdditionalLibraryDirectories>$(SolutionDir)\\"
                    << sourceToBinPath
                    << ((bb == 0) ? "x86\\" : "x64\\")
                    << "</AdditionalLibraryDirectories>\n"
                       "      <OutputFile>$(SolutionDir)\\" << sourceToBinPath
                        << ((bb == 0) ? "x86\\" : "x64\\")
                        << "vs" << verStr << "\\"
                        << ((cc == 0) ? "debug\\" : "release\\")
                        << "$(ProjectName)." << (proj.isExecutable() ? "exe" : "dll") << "</OutputFile>\n"
                    "    </Link>\n";
            }
            else    // static lib
                ofs <<
                    "    <Lib>\n"
                    "      <AdditionalOptions>/IGNORE:4221 %(AdditionalOptions)</AdditionalOptions>\n"
                    "    </Lib>\n";
            ofs <<
                "  </ItemDefinitionGroup>\n";
        }
    }
    if (proj.isLinked()) {
        ofs << "  <ItemGroup>\n";
        for (string const & name : lnkDeps) {
            if (sln.contains(name))     // Only projects with source are referenced:
                ofs <<
                    "    <ProjectReference Include=\"..\\..\\" << name << "\\VisualStudio"
                        << verStr << "\\" << name << ".vcxproj\">\n"
                    "      <Project>" << fgLookup(nameToGuid,name) << "</Project>\n"
                    "      <CopyLocalSatelliteAssemblies>true</CopyLocalSatelliteAssemblies>\n"
                    "      <ReferenceOutputAssembly>true</ReferenceOutputAssembly>\n"
                    "    </ProjectReference>\n";
        }
        ofs << "  </ItemGroup>\n";
    }
    ofs << "  <ItemGroup>\n";
    for (const FgConsSrcDir & grp : proj.srcGroups) {
        for (size_t ff=0; ff<grp.files.size(); ++ff) {
            string path = fgReplace(proj.baseDir+grp.dir+grp.files[ff],'/','\\');
            if (fgEndsWith(grp.files[ff],".cpp") || fgEndsWith(grp.files[ff],".c"))
                ofs << "    <ClCompile Include=\"..\\" << path << "\"";
            else if (fgEndsWith(grp.files[ff],".hpp") || fgEndsWith(grp.files[ff],".h"))
                ofs << "    <ClInclude Include=\"..\\" << path << "\"";
            else
                fgThrow("Unrecognized source file type",grp.files[ff]);
            // WARNING: MSVC cannot mix pure C files with pre-comiled headers unless you also individually
            // mark them for pre-compilation which we do NOT do here:
            if (grp.files[ff] == "stdafx.cpp") {
                ofs << ">\n";
                for (uint cc=0; cc<2; ++cc)
                    for (uint bb=0; bb<2; ++bb)
                        ofs <<
                            "      <PrecompiledHeader Condition=\"'$(Configuration)|$(Platform)'=='"
                            << config[cc] << "|" << bits[bb] << "'\">"
                            << ((grp.files[ff]=="stdafx.cpp") ? "Create" : "")
                            << "</PrecompiledHeader>\n";
                ofs << "    </ClCompile>\n";
            }
            else
                ofs << " />\n";
        }
    }
    ofs <<
        "  </ItemGroup>\n";
    if (pathExists(proj.name+"/icon.ico")) {
        fileCopy(proj.name+"/icon.ico",projDir+"icon1.ico",true);
        ofs <<
            "  <ItemGroup>\n"
            "    <ClInclude Include=\"resource.h\" />\n"
            "  </ItemGroup>\n"
            "  <ItemGroup>\n"
            "    <ResourceCompile Include=\""+proj.name+".rc\" />\n"
            "  </ItemGroup>\n"
            "  <ItemGroup>\n"
            "    <Image Include=\"icon1.ico\" />\n"
            "  </ItemGroup>\n";
        Ofstream  res(projDir+"resource.h");
        res <<
            "#define IDI_ICON1                       101\n"
            "#ifdef APSTUDIO_INVOKED\n"
            "#ifndef APSTUDIO_READONLY_SYMBOLS\n"
            "#define _APS_NEXT_RESOURCE_VALUE        102\n"
            "#define _APS_NEXT_COMMAND_VALUE         40001\n"
            "#define _APS_NEXT_CONTROL_VALUE         1001\n"
            "#define _APS_NEXT_SYMED_VALUE           101\n"
            "#endif\n"
            "#endif\n";
        Ofstream rd(projDir+proj.name+".rc");
        rd <<
            "#include \"resource.h\"\n"
            "#define APSTUDIO_READONLY_SYMBOLS\n"
            "#include \"afxres.h\"\n"
            "#undef APSTUDIO_READONLY_SYMBOLS\n"
            "#if !defined(AFX_RESOURCE_DLL) || defined(AFX_TARG_ENC)\n"
            "LANGUAGE LANG_ENGLISH, SUBLANG_ENGLISH_CAN\n"
            "#ifdef APSTUDIO_INVOKED\n"
            "1 TEXTINCLUDE \n"
            "BEGIN\n"
            "    \"resource.h\\0\"\n"
            "END\n"
            "2 TEXTINCLUDE \n"
            "BEGIN\n"
            "    \"#include \"\"afxres.h\"\"\\r\\n\"\n"
            "    \"\\0\"\n"
            "END\n"
            "3 TEXTINCLUDE \n"
            "BEGIN\n"
            "    \"\\r\\n\"\n"
            "    \"\\0\"\n"
            "END\n"
            "#endif    // APSTUDIO_INVOKED\n"
            "IDI_ICON1               ICON                    \"icon1.ico\"\n"
            "#endif    // English (Canada) resources\n"
            "#ifndef APSTUDIO_INVOKED\n"
            "#endif    // not APSTUDIO_INVOKED\n";
    }
    ofs <<
        "  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />\n"
        "  <ImportGroup Label=\"ExtensionTargets\">\n"
        "  </ImportGroup>\n"
        "</Project>\n";
    return fgDump(ofs.str(),projFile.c_str());
}

static
bool
writeSln(
    const FgConsSolution &      sln,
    string const &              merkle,         // Concatenation of project GUIDs
    const map<string,string> &  nameToGuid,     // Must contain all project names
    uint                        vsver)
{
    // Create solution file:
    string          rootName("VisualStudio"),
                    verStr = fgToStringDigits(vsver,2),
    // Visual studio completely changes the SLN UUID if any project is added or removed.
    // It does not change either UUID if project properties are modified:
                    slnGuid = fgCreateMicrosoftGuid(merkle+verStr);
    ostringstream   ofs;
    ofs <<
        "\n"
        "Microsoft Visual Studio Solution File, Format Version "
            << "12.00\n"
        "# Visual Studio ";
    if (vsver >= 17)
        ofs << "15\n"       // VS17 writes version number instead of year
            "VisualStudioVersion = 15.0.27703.2000\n"
            "MinimumVisualStudioVersion = 10.0.40219.1\n";
    else
        ofs << "20" << verStr << "\n";
    for (const FgConsProj & proj : sln.projects) {
        if (!proj.srcGroups.empty()) {
            ofs <<
                "Project(\"" + slnGuid + "\") = \""
                << proj.name << "\", \"" << proj.name << "\\VisualStudio" << verStr << "\\"
                << proj.name << ".vcxproj\", \"" << fgLookup(nameToGuid,proj.name) << "\"\n"
                "EndProject\n";
        }
    }
    string      globalEndSections;
    // Add CMake and Unix folders if this is a VS17 dev instance:
    if ((vsver >= 17) && pathExists("../data/_overwrite_baselines.flag")) {
        // This GUID is pre-defined to mean a solution folder:
        string                  uuidFolder = "{2150E333-8FDC-42A3-9474-1A3956D46DE8}";
        if (pathExists("CMakeLists.txt")) {   // Add CMake files for easy viewing / searching
            string                  uuidCmakeFolder = "{06685DCC-912A-409E-BAF1-580272DB71CD}";
            DirectoryContents     dc = directoryContents("");
            ofs <<
                "Project(\"" + uuidFolder + "\") = \"cmake\", \"cmake\", \"" + uuidCmakeFolder + "\"\n"
                "	ProjectSection(SolutionItems) = preProject\n"
                "		CMakeLists.txt = CMakeLists.txt\n";
            ofs <<
                "	EndProjectSection\n"
                "EndProject\n";
            for (Ustring const & dir : dc.dirnames) {
                Ustring            cmf = dir + "\\CMakeLists.txt";
                if (pathExists(cmf)) {
                    string          uuid = fgCreateMicrosoftGuid("FaceGenCmakeFolder"+dir.m_str);
                    ofs <<
                        "Project(\"" + uuidFolder + "\") = \"" + dir + "\", \"" + dir + "\", \"" + uuid + "\"\n"
                        "	ProjectSection(SolutionItems) = preProject\n"
                        "		" + cmf + " = " + cmf + "\n"
                        "	EndProjectSection\n"
                        "EndProject\n";
                    globalEndSections += "		" + uuid + " = " + uuidCmakeFolder + "\n";
                }
            }
        }
        if (pathExists("LibFgBase/src/nix")) {    // Add unix files for easy viewing / searching
            string                  nixPath = "LibFgBase\\src\\nix\\";
            string                  uuidNixFolder = "{AE0BD3A3-EF58-4762-9266-81AF0C5A05A8}";
            DirectoryContents     dc = directoryContents(nixPath);
            ofs <<
                // These GUIDs are specific to generic folders with text files in them:
                "Project(\"" + uuidFolder + "\") = \"unix\", \"unix\", \"" + uuidNixFolder + "\"\n"
                "	ProjectSection(SolutionItems) = preProject\n";
            for (Ustring const & fname : dc.filenames)
                ofs << "		" + nixPath+fname + " = " + nixPath+fname + "\n";
            ofs <<
                "	EndProjectSection\n"
                "EndProject\n";
        }
    }
    ofs <<
        "Global\n"
        "	GlobalSection(SolutionConfigurationPlatforms) = preSolution\n"
        "		Debug|Win32 = Debug|Win32\n"
        "		Debug|x64 = Debug|x64\n"
        "		Release|Win32 = Release|Win32\n"
        "		Release|x64 = Release|x64\n"
        "	EndGlobalSection\n"
        "	GlobalSection(ProjectConfigurationPlatforms) = postSolution\n";
    for (const FgConsProj & proj : sln.projects) {
        if (!proj.srcGroups.empty()) {
            string          guid = fgLookup(nameToGuid,proj.name);
            ofs <<
                "		" << guid << ".Debug|Win32.ActiveCfg = Debug|Win32\n"
                "		" << guid << ".Debug|Win32.Build.0 = Debug|Win32\n"
                "		" << guid << ".Debug|x64.ActiveCfg = Debug|x64\n"
                "		" << guid << ".Debug|x64.Build.0 = Debug|x64\n"
                "		" << guid << ".Release|Win32.ActiveCfg = Release|Win32\n"
                "		" << guid << ".Release|Win32.Build.0 = Release|Win32\n"
                "		" << guid << ".Release|x64.ActiveCfg = Release|x64\n"
                "		" << guid << ".Release|x64.Build.0 = Release|x64\n";
        }
    }
    ofs <<
        "	EndGlobalSection\n"
        "	GlobalSection(SolutionProperties) = preSolution\n"
        "		HideSolutionNode = FALSE\n"
        "	EndGlobalSection\n"
        "	GlobalSection(NestedProjects) = preSolution\n"
        << globalEndSections <<
        "	EndGlobalSection\n"
        "	GlobalSection(ExtensibilityGlobals) = postSolution\n"
        "		SolutionGuid = {39B2BC3E-9FF2-4906-92AE-80DBB3FF9746}\n"
        "	EndGlobalSection\n"
        "EndGlobal\n";
    return fgDump(ofs.str(),rootName+verStr+".sln");
}

bool
fgConsVs201x(const FgConsSolution & sln)
{
    FGASSERT(sln.type == FgConsType::win);
    map<string,string>  nameToGuid;
    string              hashPrepend = "FaceGenVisualStudio:",    // Ensure descriptor string >= 16 chars
                        merkle;
    for (const FgConsProj & proj : sln.projects) {
        if (!proj.srcGroups.empty()) {
            string          projGuid = fgCreateMicrosoftGuid(hashPrepend+proj.descriptor());
            auto            it = nameToGuid.find(proj.name);
            if (it != nameToGuid.end())
                fgThrow("fgConsVs201x duplicate project name",proj.name);
            nameToGuid[proj.name] = projGuid;
            merkle += projGuid;
        }
    }
    bool                changed = false;
    for (const FgConsProj & proj : sln.projects) {
        if (!proj.srcGroups.empty()) {
            changed = writeVcxproj(sln,proj,nameToGuid,15) || changed;
            changed = writeVcxproj(sln,proj,nameToGuid,17) || changed;
            changed = writeVcxproj(sln,proj,nameToGuid,19) || changed;
        }
    }
    // This codebase uses C++11 which is only supported by VS 2013 and later:
    changed = writeSln(sln,merkle,nameToGuid,15) || changed;
    changed = writeSln(sln,merkle,nameToGuid,17) || changed;
    changed = writeSln(sln,merkle,nameToGuid,19) || changed;
    return changed;
}

}

// */
