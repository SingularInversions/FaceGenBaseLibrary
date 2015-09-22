//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Nov 30, 2011
//

#include "stdafx.h"

#include "FgCons.hpp"
#include "FgOut.hpp"
#include "FgException.hpp"
#include "FgFileSystem.hpp"
#include "FgBuild.hpp"

using namespace std;

string
fgConsVsPreprocessorDefs(bool,const FgConsProj &);

static
void
writeVcxproj(
    const FgConsProj &          proj,
    const map<string,string> &  guidMap,
    uint                        vsver)      // 10 - VS2010, 12 - VS2012, 13 - VS2013
{
    if (proj.srcGroups.empty())
        fgThrow("Project has no source files",proj.name);
    string      verStr = fgToStringDigits(vsver,2),
                projDir = proj.name + "/VisualStudio" + verStr + "/";
    fgCreateDirectory(projDir);
    string      projFile = projDir + proj.name + ".vcxproj";
    ofstream    ofs(projFile.c_str());      // Text mode so newline = CR LF
    string      config[] = {"Debug","Release"};
    string      bits[] = {"Win32","x64"};
    string      toolsVersion = "4";
    if (vsver == 13)
        toolsVersion = "12";
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
        "    <ProjectGuid>" << proj.guid << "</ProjectGuid>\n"
        "    <RootNamespace>" << proj.name << "</RootNamespace>\n"
        "    <Keyword>Win32Proj</Keyword>\n"
        "  </PropertyGroup>\n"
        "  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />\n";
    for (uint bb=0; bb<2; ++bb) {
        for (uint cc=0; cc<2; ++cc) {
            ofs <<
                "  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='" << config[cc]
                    << "|" << bits[bb] << "'\" Label=\"Configuration\">\n"
                "    <ConfigurationType>";
            if (proj.app)
                ofs << "Application";
            else if (proj.dll)
                ofs << "DynamicLibrary";
            else
                ofs << "StaticLibrary";
            ofs <<
                "</ConfigurationType>\n";
            // PlatformToolset:
            // v100: use VS2010 (VC++10) compiler & libs (from VS2012+ IDE)
            // v110: use VS2012 (VC++11) compiler & libs, etc.
            if (vsver == 12)
                ofs << "    <PlatformToolset>v110</PlatformToolset>\n";
            else if (vsver == 13)
                ofs << "    <PlatformToolset>v120</PlatformToolset>\n";
            ofs <<
                "    <CharacterSet>" << (proj.unicode ? "Unicode" : "MultiByte") << "</CharacterSet>\n";
            if (cc == 1)
                ofs <<
                    "    <WholeProgramOptimization>true</WholeProgramOptimization>\n";
            ofs <<
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
    sourceToBinPath += "vs"+verStr+"\\";
    for (uint cc=0; cc<2; ++cc) {
        for (uint bb=0; bb<2; ++bb) {
            ofs <<
                "    <OutDir Condition=\"'$(Configuration)|$(Platform)'=='" << config[cc]
                    << "|" << bits[bb] << "'\">";
            if (proj.app || proj.dll)
                ofs << "..\\..\\" << sourceToBinPath
                    << ((bb == 0) ? "32\\" : "64\\")
                    << ((cc == 0) ? "debug\\" : "release\\");
            else
                ofs << "$(Platform)\\$(Configuration)\\";
            ofs << "</OutDir>\n"
                "    <IntDir Condition=\"'$(Configuration)|$(Platform)'=='" << config[cc]
                    << "|" << bits[bb] << "'\">$(Platform)\\$(Configuration)\\</IntDir>\n";
            if (proj.app || proj.dll)
                ofs <<
                    "    <LinkIncremental Condition=\"'$(Configuration)|$(Platform)'=='" << config[cc]
                    << "|" << bits[bb] << "'\">" << ((cc == 0) ? "true" : "false") << "</LinkIncremental>\n";
        }
    }
    ofs <<
        "  </PropertyGroup>\n";
    bool        pch = false;
    if (fgFileReadable(proj.name+"/"+proj.srcBaseDir+proj.srcGroups[0].dir+"stdafx.cpp"))
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
            for (size_t ii=0; ii<proj.incDirs.size(); ++ii)
            {
                string  dir = fgReplace(proj.incDirs[ii],'/','\\');
                ofs << "..\\" << dir << ";";    // .vcxproj file is in a subdir of project dir
            }
            ofs <<
                "%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>\n"
                "      <PreprocessorDefinitions>" << fgConsVsPreprocessorDefs((cc == 1),proj)
                    << ";%(PreprocessorDefinitions)</PreprocessorDefinitions>\n";
            if (cc == 0)
                ofs <<
                    "      <BasicRuntimeChecks>EnableFastChecks</BasicRuntimeChecks>\n";
            ofs <<
                "      <RuntimeLibrary>MultiThreaded" << ((cc == 0) ? "Debug" : "") << "</RuntimeLibrary>\n";
            if (cc == 1)
                ofs <<
                    "      <FunctionLevelLinking>true</FunctionLevelLinking>\n";
            ofs <<
                "      <PrecompiledHeader>" << (pch ? "Use" : "") << "</PrecompiledHeader>\n"
                "      <WarningLevel>"
                    << ((proj.warn == 0) ? "TurnOffAllWarnings" : "Level" + fgToString(proj.warn))
                    << "</WarningLevel>\n"
                "      <DebugInformationFormat>" << ((cc == 0) ? "ProgramDatabase" : "")
                    << "</DebugInformationFormat>\n"
                "    </ClCompile>\n";
            if (proj.app || proj.dll) {
                ofs <<
                    "    <Link>\n"
                    "      <GenerateDebugInformation>" << ((cc == 0) ? "true" : "false")
                        << "</GenerateDebugInformation>\n"
                    "      <SubSystem>" << (proj.pureGui ? "Windows" : "Console") << "</SubSystem>\n"
                    "      <TargetMachine>MachineX" << ((bb == 0) ? "86" : "64") << "</TargetMachine>\n"
                    "      <AdditionalDependencies>gdiplus.lib;comctl32.lib;user32.lib;gdi32.lib;"
                        "opengl32.lib;advapi32.lib;comdlg32.lib;Shell32.lib;";
                for (size_t ii=0; ii<proj.dllDeps.size(); ++ii)
                    ofs << proj.dllDeps[ii] << ".lib;";
                ofs <<
                    "%(AdditionalDependencies)</AdditionalDependencies>\n";
                if (!proj.dllDeps.empty())
                    // The SDK builder copies the preferred binaries into all fully
                    // differentiated bins:
                    ofs <<
                        "      <AdditionalLibraryDirectories>$(SolutionDir)\\"
                        << sourceToBinPath
                        << ((bb == 0) ? "32\\" : "64\\")
                        << ((cc == 0) ? "debug\\" : "release\\")
                        << "</AdditionalLibraryDirectories>\n";
                ofs <<
                    "      <OutputFile>$(SolutionDir)\\" << sourceToBinPath
                        << ((bb == 0) ? "32\\" : "64\\")
                        << ((cc == 0) ? "debug\\" : "release\\")
                        << "$(ProjectName)." << (proj.app ? "exe" : "dll") << "</OutputFile>\n"
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
    if (proj.app || proj.dll) {
        ofs << "  <ItemGroup>\n";
        for (size_t ll=0; ll<proj.lnkDeps.size(); ++ll) {
            const string &  name = proj.lnkDeps[ll];
            map<string,string>::const_iterator  it = guidMap.find(name);
            if (it == guidMap.end())
                fgThrow("Link dependency to unknown library",name);
            const string &  guid = it->second;
            ofs <<
                "    <ProjectReference Include=\"..\\..\\" << name << "\\VisualStudio"
                    << verStr << "\\" << name << ".vcxproj\">\n"
                "      <Project>" << guid << "</Project>\n"
                "      <CopyLocalSatelliteAssemblies>true</CopyLocalSatelliteAssemblies>\n"
                "      <ReferenceOutputAssembly>true</ReferenceOutputAssembly>\n"
                "    </ProjectReference>\n";
        }
        ofs << "  </ItemGroup>\n";
    }
    ofs << "  <ItemGroup>\n";
    for (size_t gg=0; gg<proj.srcGroups.size(); ++gg) {
        const FgConsSrcGroup &  grp = proj.srcGroups[gg];
        for (size_t ff=0; ff<grp.files.size(); ++ff) {
            string path = fgReplace(proj.srcBaseDir+grp.dir+grp.files[ff],'/','\\');
            if (fgEndsWith(grp.files[ff],".cpp") || fgEndsWith(grp.files[ff],".c"))
                ofs << "    <ClCompile Include=\"..\\" << path << "\"";
            else if (fgEndsWith(grp.files[ff],".hpp") || fgEndsWith(grp.files[ff],".h"))
                ofs << "    <ClInclude Include=\"..\\" << path << "\"";
            else
                fgThrow("Unrecognized source file type",grp.files[ff]);
            if ((grp.files[ff] == "stdafx.cpp") || (fgEndsWith(grp.files[ff],".c"))) {
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
                ofs << "  />\n";
        }
    }
    ofs <<
        "  </ItemGroup>\n";
    if ((vsver >= 12) && fgExists(proj.name+"/icon.ico")) {
        fgCopyFile(proj.name+"/icon.ico",projDir+"icon1.ico",true);
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
        FgOfstream  res(projDir+"resource.h");
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
        FgOfstream rd(projDir+proj.name+".rc");
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
}

static
void
writeSln(
    const FgConsSolution &  sln,
    uint                    vsver)  // 10, 12, 13
{
    // Create solution file:
    string          rootName("VisualStudio"),
                    verStr = fgToStringDigits(vsver,2);
    FgOfstream      ofs(rootName+verStr+".sln");
    ofs <<
        "\n"
        "Microsoft Visual Studio Solution File, Format Version "
            << ((vsver == 10) ? "11" : "12") << ".00\n"
        "# Visual Studio 20" << verStr << endl;
    for (size_t ii=0; ii<sln.projects.size(); ++ii) {
        const FgConsProj & proj = sln.projects[ii];
        ofs <<
            "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \""
            << proj.name << "\", \"" << proj.name << "\\VisualStudio" << verStr << "\\"
            << proj.name << ".vcxproj\", \"" << proj.guid << "\"\n"
            "EndProject\n";
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
    for (size_t ii=0; ii<sln.projects.size(); ++ii) {
        const string & guid = sln.projects[ii].guid;
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
    ofs <<
        "	EndGlobalSection\n"
        "	GlobalSection(SolutionProperties) = preSolution\n"
        "		HideSolutionNode = FALSE\n"
        "	EndGlobalSection\n"
        "EndGlobal\n";
}

void
fgConsVs201x(FgConsSolution sln)
{
    // Create project files:
    map<string,string>  guidMap;
    for (size_t ii=0; ii<sln.projects.size(); ++ii) {
        FgConsProj &    proj = sln.projects[ii];
        proj.guid = fgCreateMicrosoftGuid(proj.name);
        guidMap.insert(make_pair(proj.name,proj.guid));
    }
    for (size_t ii=0; ii<sln.projects.size(); ++ii) {
        writeVcxproj(sln.projects[ii],guidMap,10);
        writeVcxproj(sln.projects[ii],guidMap,12);
        writeVcxproj(sln.projects[ii],guidMap,13);
    }
    writeSln(sln,10);
    writeSln(sln,12);
    writeSln(sln,13);
}

// */
