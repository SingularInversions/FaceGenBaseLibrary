//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Generate Visual Studio SLN and XPROJ files.
//
// NOTES:
// * Use of the exact number of tab characters is required for correct VS parsing behaviour.

#include "stdafx.h"

#include "FgBuild.hpp"
#include "FgFileSystem.hpp"
#include "FgBuild.hpp"
#include "FgTestUtils.hpp"

using namespace std;

namespace Fg {

namespace {

struct      VsInfo {
    Compiler            compiler;
    String              vsYearDigits2;
    uint                vsMajorVersion;
    String              targetPlatformVersion;
    String              platformToolset;
    String              vsFullVersion;      // with latest updates as of 2022.05
    String              vcProjectVersion;

    bool                operator==(Compiler c) const {return (compiler==c); }
};

VsInfo                  getCompilerInfo(Compiler compiler)
{
    static vector<VsInfo>   ret {
        {Compiler::vs19,"19",16,"10.0",        "v142","16.0.32510.428", "16.0",},
        {Compiler::vs22,"22",17,"10.0",        "v143","17.2.32519.379", "16.0",},
    };
    return findFirst(ret,compiler);
}

string              getVsPreprocessorDefs(
    bool                release,
    ConsProj const &    proj,
    Strings const &     defs)
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
        fgThrow("getVsPreprocessorDefs unhandled type",proj.type);
    for (string const & def : defs)
        ret += ";" + def;
    return ret;
}

bool                writeVcxproj(
    ConsSolution const &        sln,            // Transitive lookups
    ConsProj const &            proj,
    const map<string,string> &  nameToGuid,     // Must contain all project names
    Compiler                    compiler)
{
    VsInfo                  vsinfo = getCompilerInfo(compiler);
    Strings                 includes = sln.getIncludes(proj.name,false),
                            defines = sln.getDefs(proj.name),
                            lnkDeps = sln.getLnkDeps(proj.name);
    string                  projDir = proj.name + "/VisualStudio" + vsinfo.vsYearDigits2 + "/";
    createDirectory(projDir);
    string                  projFile = projDir + proj.name + ".vcxproj";
    ostringstream           ofs;
    string                  config[] = {"Debug","Release"};
    string                  bits[] = {"Win32","x64"};
    string                  toolsVersion = "15";
    ofs << 
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
        "<Project DefaultTargets=\"Build\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n"
        "  <ItemGroup Label=\"ProjectConfigurations\">\n";
    struct              DRSel           // debrel-selected attributes
    {
        String          nameLower;
        String          nameUpper;
        String          useDebugLibs;
        String          linkIncremental;
    };
    Arr<DRSel,2>        drSels {{
        {"debug",  "Debug",  "true", "true",},
        {"release","Release","false","false",},
        }};
    for (uint bb=0; bb<2; ++bb)
        for (DRSel const & drSel : drSels)
            ofs <<
                "    <ProjectConfiguration Include=\"" << drSel.nameUpper << "|" << bits[bb] << "\">\n"
                "      <Configuration>" << drSel.nameUpper << "</Configuration>\n"
                "      <Platform>" << bits[bb] << "</Platform>\n"
                "    </ProjectConfiguration>\n";
    ofs <<
        "  </ItemGroup>\n"
        "  <PropertyGroup Label=\"Globals\">\n"
        "    <VCProjectVersion>" << vsinfo.vcProjectVersion << "</VCProjectVersion>\n"
        "    <Keyword>Win32Proj</Keyword>\n"
        "    <ProjectGuid>" << nameToGuid.find(proj.name)->second << "</ProjectGuid>\n"
        "    <RootNamespace>" << proj.name << "</RootNamespace>\n"
        "    <WindowsTargetPlatformVersion>" + vsinfo.targetPlatformVersion + "</WindowsTargetPlatformVersion>\n"
        "  </PropertyGroup>\n"
        "  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />\n";
    for (uint bb=0; bb<2; ++bb) {
        for (DRSel const & drSel : drSels) {
            ofs <<
                "  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='" << drSel.nameUpper
                    << "|" << bits[bb] << "'\" Label=\"Configuration\">\n"
                "    <ConfigurationType>";
            if (proj.isExecutable())
                ofs << "Application";
            else if (proj.isDynamicLib())
                ofs << "DynamicLibrary";
            else
                ofs << "StaticLibrary";
            ofs <<
                "</ConfigurationType>\n"
                "    <UseDebugLibraries>" << drSel.useDebugLibs << "</UseDebugLibraries>\n"
				"    <PlatformToolset>" << vsinfo.platformToolset << "</PlatformToolset>\n"
                "    <CharacterSet>Unicode</CharacterSet>\n"
                "  </PropertyGroup>\n";
        }
    }
    ofs <<
        "  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.props\" />\n"
        "  <ImportGroup Label=\"ExtensionSettings\">\n"
        "  </ImportGroup>\n";
    for (uint bb=0; bb<2; ++bb)
        for (DRSel const & drSel : drSels)
            ofs <<
                "  <ImportGroup Label=\"PropertySheets\" Condition=\"'$(Configuration)|$(Platform)'=='" << drSel.nameUpper << "|"
                    << bits[bb] << "'\">\n"
                "    <Import Project=\"$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\" Condition=\""
                    "exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')\" Label=\""
                    "LocalAppDataPlatform\" />\n"
                "  </ImportGroup>\n";
    ofs <<
        "  <PropertyGroup Label=\"UserMacros\" />\n"
        "  <PropertyGroup>\n"
        "    <_ProjectFileVersion>10.0.40219.1</_ProjectFileVersion>\n";

    string      sourceToBinPath("..\\bin\\win\\");
    for (DRSel const & drSel : drSels) {
        for (uint bb=0; bb<2; ++bb) {
            ofs <<
                "    <OutDir Condition=\"'$(Configuration)|$(Platform)'=='" << drSel.nameUpper
                    << "|" << bits[bb] << "'\">";
            if (proj.isLinked())
                ofs << "..\\..\\" << sourceToBinPath
                    << ((bb == 0) ? "x86\\" : "x64\\")
                    << "vs" << vsinfo.vsYearDigits2 << "\\" << drSel.nameLower << "\\";
            else
                ofs << "$(Platform)\\$(Configuration)\\";
            ofs << "</OutDir>\n"
                "    <IntDir Condition=\"'$(Configuration)|$(Platform)'=='" << drSel.nameUpper
                    << "|" << bits[bb] << "'\">$(Platform)\\$(Configuration)\\</IntDir>\n";
            if (proj.isLinked())
                ofs <<
                    "    <LinkIncremental Condition=\"'$(Configuration)|$(Platform)'=='" << drSel.nameUpper
                    << "|" << bits[bb] << "'\">" << drSel.linkIncremental << "</LinkIncremental>\n";
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
                string          dir = replaceAll(relPath,'/','\\');
                ofs << "..\\" << dir << ";";    // .vcxproj file is in a subdir of project dir
            }
            ofs <<
                "%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>\n"
                "      <PreprocessorDefinitions>" << getVsPreprocessorDefs((cc == 1),proj,defines)
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
            string      warnStr = (proj.warn == 0) ? "TurnOffAllWarnings" : "Level" + toStr(proj.warn);
            ofs <<
                "      <PrecompiledHeader>" << (pch ? "Use" : "\n      ") << "</PrecompiledHeader>\n"
                "      <WarningLevel>" << warnStr << "</WarningLevel>\n"
                "      <DebugInformationFormat>" << ((cc == 0) ? "ProgramDatabase" : "\n      ") << "</DebugInformationFormat>\n"
                "      <WholeProgramOptimization>false</WholeProgramOptimization>\n"
                // Note that MSVC fast floating point model is an extension to the standard and thus cannot
                // be used in combination with: Disable Language Extensions (/Za)
                "      <FloatingPointModel>Fast</FloatingPointModel>\n"
                // Don't support OpenMP in Visual Studio since:
                // * It's a pain to get working cross platform
                // * If used it requires VCOMP140.DLL which is not always present with Win7
                // * direct use of threads is not hard
                //"      <OpenMPSupport>true</OpenMPSupport>\n";
                // Show the user the column of the error and put a caret under it:
                "      <DiagnosticsFormat>Caret</DiagnosticsFormat>\n"
                // ensure source code conforms to the language specification; this does not
                // preclude non-conforming code generation (see /fpFast above)
                "      <ConformanceMode>true</ConformanceMode>\n"
                // standard C++17
                "      <LanguageStandard>stdcpp17</LanguageStandard>\n"
                "    </ClCompile>\n";
            if (proj.isLinked()) {
                ofs <<
                    "    <Link>\n"
                    "      <GenerateDebugInformation>" << ((cc == 0) ? "true" : "false")
                        << "</GenerateDebugInformation>\n"
                    "      <SubSystem>" << (proj.isGuiExecutable() ? "Windows" : "Console") << "</SubSystem>\n"
                    "      <TargetMachine>MachineX" << ((bb == 0) ? "86" : "64") << "</TargetMachine>\n"
                    // As far as I can tell, the DLLs associated with these libs are always included with
                    // windows so do not need redistribution:
                    "      <AdditionalDependencies>gdiplus.lib;comctl32.lib;user32.lib;gdi32.lib;"
                        "advapi32.lib;comdlg32.lib;Shell32.lib;";
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
                        << "vs" << vsinfo.vsYearDigits2 << "\\"
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
                        << vsinfo.vsYearDigits2 << "\\" << name << ".vcxproj\">\n"
                    "      <Project>" << nameToGuid.find(name)->second << "</Project>\n"
                    "      <CopyLocalSatelliteAssemblies>true</CopyLocalSatelliteAssemblies>\n"
                    "      <ReferenceOutputAssembly>true</ReferenceOutputAssembly>\n"
                    "    </ProjectReference>\n";
        }
        ofs << "  </ItemGroup>\n";
    }
    ofs << "  <ItemGroup>\n";
    for (const ConsSrcDir & grp : proj.srcGroups) {
        for (String const & fname : grp.files) {
            String              path = replaceAll(proj.baseDir+grp.dir+fname,'/','\\');
            if (endsWith(fname,".cpp") || endsWith(fname,".c")) {
                ofs << "    <ClCompile Include=\"..\\" << path << "\"";
                ofs << ">\n";
                // WARNING: MSVC cannot mix pure C files with pre-compiled headers unless you also individually
                // mark them for pre-compilation which we do NOT do here:
                if (fname == "stdafx.cpp") {
                    for (uint cc=0; cc<2; ++cc)
                        for (uint bb=0; bb<2; ++bb)
                            ofs <<
                                "      <PrecompiledHeader Condition=\"'$(Configuration)|$(Platform)'=='"
                                << config[cc] << "|" << bits[bb] << "'\">"
                                << ((fname=="stdafx.cpp") ? "Create" : "")
                                << "</PrecompiledHeader>\n";
                }
                else {      // cannot use output tree structure for stdafx.obj:
                    // ensure no obj filename collision by mirroring tree structure:
                    Path                fpath {fname};
                    // only specify output tree when necessary since it breaks parallel builds with Incredibuild:
                    if (!fpath.dirs.empty()) {
                        String8             objPath = grp.dir + fpath.dirBase() + ".obj";
                        ofs << "      <ObjectFileName>$(IntDir)/" << objPath + ".obj</ObjectFileName>\n";
                    }
                }
                ofs << "    </ClCompile>\n";
            }
            else if (endsWith(fname,".hpp") || endsWith(fname,".h")) {
                ofs << "    <ClInclude Include=\"..\\" << path << "\"  />\n";
            }
            else
                fgThrow("Unrecognized source file type",fname);
        }
    }
    ofs <<
        "  </ItemGroup>\n";
    if (pathExists(proj.name+"/icon.ico")) {
        copyFile(proj.name+"/icon.ico",projDir+"icon1.ico",true);
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
    return saveRaw(ofs.str(),projFile.c_str());
}

bool                writeSln(
    ConsSolution const &        sln,
    string const &              merkle,         // Concatenation of project GUIDs
    map<string,string> const &  nameToGuid,     // Must contain all project names
    Compiler                    compiler)
{
    // Create solution file:
    VsInfo                  vsinfo = getCompilerInfo(compiler);
    String                  rootName("VisualStudio"),
    // Visual studio completely changes the SLN UUID if any project is added or removed.
    // It does not change either UUID if project properties are modified:
                            slnGuid = createMicrosoftGuid(merkle+vsinfo.vsYearDigits2);
    ostringstream           ofs;
    ofs <<
        "\n"
        "Microsoft Visual Studio Solution File, Format Version 12.00\n"
        "# Visual Studio Version " << vsinfo.vsMajorVersion << "\n"
        "VisualStudioVersion = " << vsinfo.vsFullVersion << "\n"
        "MinimumVisualStudioVersion = 10.0.40219.1\n";
    for (ConsProj const & proj : sln.projects) {
        if (!proj.srcGroups.empty()) {
            ofs <<
                "Project(\"" + slnGuid + "\") = \""
                << proj.name << "\", \"" << proj.name << "\\VisualStudio" << vsinfo.vsYearDigits2 << "\\"
                << proj.name << ".vcxproj\", \"" << nameToGuid.find(proj.name)->second << "\"\n"
                "EndProject\n";
        }
    }
    string      globalEndSections;
    // Add CMake and Unix folders if this is a VS dev instance:
    if (pathExists("../data/_overwrite_baselines.flag")) {
        // This GUID is pre-defined to mean a solution folder:
        string                  uuidFolder = "{2150E333-8FDC-42A3-9474-1A3956D46DE8}";
        if (pathExists("CMakeLists.txt")) {   // Add CMake files for easy viewing / searching
            string                  uuidCmakeFolder = "{06685DCC-912A-409E-BAF1-580272DB71CD}";
            DirContents     dc = getDirContents("");
            ofs <<
                "Project(\"" + uuidFolder + "\") = \"cmake\", \"cmake\", \"" + uuidCmakeFolder + "\"\n"
                "	ProjectSection(SolutionItems) = preProject\n"
                "		CMakeLists.txt = CMakeLists.txt\n";
            ofs <<
                "	EndProjectSection\n"
                "EndProject\n";
            for (String8 const & dir : dc.dirnames) {
                String8            cmf = dir + "\\CMakeLists.txt";
                if (pathExists(cmf)) {
                    string          uuid = createMicrosoftGuid("FaceGenCmakeFolder"+dir.m_str);
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
            DirContents     dc = getDirContents(nixPath);
            ofs <<
                // These GUIDs are specific to generic folders with text files in them:
                "Project(\"" + uuidFolder + "\") = \"unix\", \"unix\", \"" + uuidNixFolder + "\"\n"
                "	ProjectSection(SolutionItems) = preProject\n";
            for (String8 const & fname : dc.filenames)
                ofs << "		" + nixPath+fname + " = " + nixPath+fname + "\n";
            ofs <<
                "	EndProjectSection\n"
                "EndProject\n";
        }
        if (pathExists("../data/base/shaders")) {
            string                  shaderPath = "..\\data\\base\\shaders\\";
            ofs <<
                // These GUIDs are specific to generic folders with text files in them:
                "Project(\"" + uuidFolder + "\") = \"shaders\", \"shaders\", \"{6A42D238-C546-44D6-B1DB-25366960F044}\"\n"
                "	ProjectSection(SolutionItems) = preProject\n";
            for (String8 const & fname : getDirContents(shaderPath).filenames)
                if (pathToExt(fname) == "hlsl")
                    ofs << "		" + shaderPath+fname + " = " + shaderPath+fname + "\n";
            ofs <<
                "	EndProjectSection\n"
                "EndProject\n";
        }
        if (pathExists("../design")) {
            string                  path = "..\\design\\";
            ofs <<
                "Project(\"" + uuidFolder + "\") = \"design\", \"design\", \"{56DCB0CB-CA3C-4FD8-9799-2DAB9E0724A7}\"\n"
                "	ProjectSection(SolutionItems) = preProject\n";
            for (String8 const & fname : getDirContents(path).filenames)
                if ((pathToExt(fname) == "html") || (pathToExt(fname) == "txt"))
                    ofs << "		" + path+fname + " = " + path+fname + "\n";
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
    for (ConsProj const & proj : sln.projects) {
        if (!proj.srcGroups.empty()) {
            string          guid = nameToGuid.find(proj.name)->second;
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
    return saveRaw(ofs.str(),rootName+vsinfo.vsYearDigits2+".sln");
}

}

bool                writeVisualStudioSolutionFiles(ConsSolution const & sln)
{
    FGASSERT(sln.type == ConsType::win);
    map<string,string>  nameToGuid;
    string              hashPrepend = "FaceGenVisualStudio:",    // Ensure descriptor string >= 16 chars
                        merkle;
    for (ConsProj const & proj : sln.projects) {
        if (!proj.srcGroups.empty()) {
            string          projGuid = createMicrosoftGuid(hashPrepend+proj.descriptor());
            auto            it = nameToGuid.find(proj.name);
            if (it != nameToGuid.end())
                fgThrow("writeVisualStudioSolutionFiles duplicate project name",proj.name);
            nameToGuid[proj.name] = projGuid;
            merkle += projGuid;
        }
    }
    // This codebase uses C++17 which is only supported by VS 2019 and later:
    Compilers           compilers {Compiler::vs19,Compiler::vs22,};
    bool                changed = false;
    for (ConsProj const & proj : sln.projects) {
        if (!proj.srcGroups.empty()) {
            for (Compiler compiler : compilers)
                changed = writeVcxproj(sln,proj,nameToGuid,compiler) || changed;
        }
    }
    for (Compiler compiler : compilers)
        changed = writeSln(sln,merkle,nameToGuid,compiler) || changed;
    return changed;
}

}

// */
