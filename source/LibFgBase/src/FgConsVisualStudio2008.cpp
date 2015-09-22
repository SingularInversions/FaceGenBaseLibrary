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
#include "FgRandom.hpp"
#include "FgHex.hpp"
#include "FgBuild.hpp"

using namespace std;

string
fgConsVsPreprocessorDefs(
    bool                release,
    const FgConsProj &  proj)
{
    string  ret = "WIN32";
    if (release)
        ret += ";NDEBUG";
    else
        ret += ";_DEBUG";
    if (proj.app) {
        if (proj.pureGui)
            ret += ";_WINDOWS";
        else
            ret += ";_CONSOLE";
    }
    else {
        if (proj.dll)
            ret += ";_WINDOWS;_USRDLL;TESTDLL_EXPORTS";
        else
            ret += ";_LIB";
    }
    ret +=
        ";_CRT_SECURE_NO_DEPRECATE=1"
        ";_SCL_SECURE_NO_DEPRECATE=1"
        ";_CRT_SECURE_NO_WARNINGS";
    for (size_t ii=0; ii<proj.defs.size(); ++ii)
        ret += ";" + proj.defs[ii];
    return ret;
}

static
void
writeVcprojFiles(
    ofstream &          ofs,
    string              srcBaseDir,
    FgConsSrcGroup      group)
{
    // Note that VS by default puts all .obj files into the same directory and silently
    // overwrites any duplicate names resulting from identially-named source files in
    // different source directories (within the same project). The only such duplicate
    // file is utf8_codecvt_facet.cpp in boost which compiles to the same thing in both
    // instances (filesystem and serialization) so we just leave it for now.
    if (!group.dir.empty())
        ofs <<
            " 		<Filter\n"
            "			Name=\"" << group.dir << "\"\n"
            "			>\n";
    srcBaseDir = fgReplace(srcBaseDir,'/','\\');
    group.dir = fgReplace(group.dir,'/','\\');
    for (size_t ii=0; ii<group.files.size(); ++ii) {
        FgPath      p(group.files[ii]);
        string      ext = p.ext.ascii();
        ofs <<
            "        <File RelativePath=\"..\\"
            << srcBaseDir << group.dir << group.files[ii] << "\">\n";
        if ((p.base.ascii() == "stdafx") && (ext == "cpp")) {
            ofs <<
                "			<FileConfiguration\n"
                "			    Name=\"Debug|Win32\">\n"
                "				<Tool\n"
                "				    Name=\"VCCLCompilerTool\"\n"
                "				    UsePrecompiledHeader=\"1\"\n"
                "				/>\n"
                "			</FileConfiguration>\n"
                "			<FileConfiguration\n"
                "			    Name=\"Debug|x64\">\n"
                "				<Tool\n"
                "				    Name=\"VCCLCompilerTool\"\n"
                "				    UsePrecompiledHeader=\"1\"\n"
                "				/>\n"
                "			</FileConfiguration>\n"
                "			<FileConfiguration\n"
                "			    Name=\"Release|Win32\">\n"
                "				<Tool\n"
                "				    Name=\"VCCLCompilerTool\"\n"
                "				    UsePrecompiledHeader=\"1\"\n"
                "				/>\n"
                "			</FileConfiguration>\n"
                "			<FileConfiguration\n"
                "			    Name=\"Release|x64\">\n"
                "				<Tool\n"
                "				    Name=\"VCCLCompilerTool\"\n"
                "				    UsePrecompiledHeader=\"1\"\n"
                "				/>\n"
                "			</FileConfiguration>\n"
                "		</File>\n";
        }
        else {
            // C files must be excluded from PCH:
            if (ext == "c")
                ofs << 
                    "			<FileConfiguration\n"
                    "			    Name=\"Release|Win32\">\n"
                    "				<Tool\n"
                    "				    Name=\"VCCLCompilerTool\"\n"
                    "				    UsePrecompiledHeader=\"0\"\n"
                    "				/>\n"
                    "			</FileConfiguration>\n"
                    "			<FileConfiguration\n"
                    "			    Name=\"Release|x64\">\n"
                    "				<Tool\n"
                    "				    Name=\"VCCLCompilerTool\"\n"
                    "				    UsePrecompiledHeader=\"0\"\n"
                    "				/>\n"
                    "			</FileConfiguration>\n"
                    "			<FileConfiguration\n"
                    "			    Name=\"Debug|Win32\">\n"
                    "				<Tool\n"
                    "				    Name=\"VCCLCompilerTool\"\n"
                    "				    UsePrecompiledHeader=\"0\"\n"
                    "				/>\n"
                    "			</FileConfiguration>\n"
                    "			<FileConfiguration\n"
                    "			    Name=\"Debug|x64\">\n"
                    "				<Tool\n"
                    "				    Name=\"VCCLCompilerTool\"\n"
                    "				    UsePrecompiledHeader=\"0\"\n"
                    "				/>\n"
                    "			</FileConfiguration>\n";
            ofs <<  "        </File>\n";
        }
    }
    if (!group.dir.empty())
        ofs <<
            " 		</Filter>\n";
}

static
void
writeVcprojConfig(
    ofstream &              ofs,
    const FgConsProj &      proj,
    bool                    release,
    bool                    x64,
    bool                    pch)
{
    string  configPath;
    if (x64)
        configPath = "64\\";
    else
        configPath = "32\\";
    if (release)
        configPath += "release\\";
    else
        configPath += "debug\\";
    ofs <<
        "        <Configuration\n"
        "            Name=\"" << (release ? "Release|" : "Debug|") << (x64 ? "x64" : "Win32") << "\"\n"
        "            OutputDirectory=\"";
    if (proj.app || proj.dll)
        ofs << (x64 ? "64\\" : "32\\") << (release ? "release" : "debug");
    else
        ofs << "$(PlatformName)\\$(ConfigurationName)";
    ofs << "\"\n"
        "            IntermediateDirectory=\"$(PlatformName)\\$(ConfigurationName)\"\n";
    char    configType;
    if (proj.app)
        configType = '1';
    else {
        if (proj.dll)
            configType = '2';
        else
            configType = '4';
    }
    ofs <<
        "            ConfigurationType=\"" << configType << "\"\n"
        // Value of 2 is the old MBCS used by Fg3PlatformWin:
        "            CharacterSet=\"" << (proj.unicode ? '1' : '2') << "\"\n";
    if (release)
        ofs << "            WholeProgramOptimization=\"1\"\n";
    ofs <<
        "            >\n"
        "            <Tool\n"
        "                Name=\"VCPreBuildEventTool\"\n"
        "            />\n"
        "            <Tool\n"
        "                Name=\"VCCustomBuildTool\"\n"
        "            />\n"
        "            <Tool\n"
        "                Name=\"VCMIDLTool\"\n";
    // Microsoft Interface Definition Language (MIDL) tool is COM related:
    if (x64)
        ofs <<
        "                TargetEnvironment=\"3\"\n";
    ofs <<
        "            />\n"
        "            <Tool\n"
        "                Name=\"VCCLCompilerTool\"\n"
        // zm200: use twice (200%) of the default memory allocation for PCH. Required due to boost.
        // bigobj: boost serialization templates exceeded a limitation of the old COFF format 
        // when compiling certain files, so use the newer format (not link-compatible with
        // vs2005).
        "			    AdditionalOptions=\"/Zm200 /bigobj\"\n"
        "                Optimization=\"" << (release ? '2' : '0') << "\"\n";
    if (release)
        ofs << "                EnableIntrinsicFunctions=\"true\"\n";
    ofs <<
        "                AdditionalIncludeDirectories=\"";
    for (size_t ii=0; ii<proj.incDirs.size(); ++ii) {
        string  dir = fgReplace(proj.incDirs[ii],'/','\\');
        ofs << "..\\" << dir;       // .vcproj file is in a subdir of project dir
        if (ii+1 < proj.incDirs.size())
            ofs << ";";
    }
    ofs <<
        "\"\n"
        "                PreprocessorDefinitions=\"" << fgConsVsPreprocessorDefs(release,proj) << "\"\n";
        // Disable minimal rebuilds for safety (issues crop up if multiple
        // definitions of a global appear). VS2008 default is only enable for
        // debug builds:
        //"                MinimalRebuild=\"true\"\n"
    if (!release)
        ofs <<
        "                BasicRuntimeChecks=\"3\"\n";
    ofs <<
        // These are the CRT static bound ones:
        "                RuntimeLibrary=\"" << (release ? '0' : '1') << "\"\n";
    if (release)
        ofs <<
        "                EnableFunctionLevelLinking=\"true\"\n";
    ofs <<
        // Enable pre-compiled headers:
        "                UsePrecompiledHeader=\"" << (pch ? '2' : '0') << "\"\n"
        "                WarningLevel=\"" << proj.warn << "\"\n"
        // 0: none.
        // 3: Zi - default
        // 4: ZI - debug info for edit & continue
        "                DebugInformationFormat=\"" << (release ? '0' : '3') << "\"\n"
        "            />\n"
        "            <Tool\n"
        "                Name=\"VCManagedResourceCompilerTool\"\n"
        "            />\n"
        "            <Tool\n"
        "                Name=\"VCResourceCompilerTool\"\n"
        "            />\n"
        "            <Tool\n"
        "                Name=\"VCPreLinkEventTool\"\n"
        "            />\n";
    if (proj.app || proj.dll) {
        ofs <<
            "			<Tool\n"
            "				Name=\"VCLinkerTool\"\n"
            // Incremental builds allowed for debug not release:
            "				LinkIncremental=\"" << (release ? '1' : '2') << "\"\n"
            "				GenerateDebugInformation=\"" << (release ? "false" : "true") << "\"\n";
        char    subsystem;
        if (proj.pureGui || proj.dll)
            subsystem = '2';    // Windows app / dll
        else
            subsystem = '1';    // Console app
        ofs <<
            "				SubSystem=\"" << subsystem << "\"\n";
        if (release)
            ofs <<
                "				OptimizeReferences=\"2\"\n"
                "				EnableCOMDATFolding=\"2\"\n";
        ofs <<
            "				TargetMachine=\"" << (x64 ? "17" : "1") << "\"\n"
            "				AdditionalDependencies=\""
                "gdiplus.lib comctl32.lib user32.lib gdi32.lib opengl32.lib "
                "advapi32.lib comdlg32.lib Shell32.lib";
        for (size_t ii=0; ii<proj.dllDeps.size(); ++ii)
            ofs << " " << proj.dllDeps[ii] << ".lib";
        ofs << "\"\n";
        if (!proj.dllDeps.empty())
            // The SDK builder copies the preferred binaries into all fully differentiated bins:
            ofs << 
                "				AdditionalLibraryDirectories=\"$(SolutionDir)\\..\\bin\\win\\vs08\\"
                << configPath << "\"\n";
        ofs <<
            "				OutputFile=\"$(SolutionDir)\\..\\bin\\win\\vs08\\" << configPath <<
                "$(ProjectName)." << (proj.app ? "exe" : "dll") << "\"\n"
            "			/>\n";
    }
    else {
        ofs <<
            "            <Tool\n"
            "                Name=\"VCLibrarianTool\"\n"
                             // Don't warn about .obj files with nothing in them:
            "                AdditionalOptions=\"/IGNORE:4221\"\n"
            "            />\n";
    }
    ofs <<
        "            <Tool\n"
        "                Name=\"VCALinkTool\"\n"
        "            />\n"
        "            <Tool\n"
        "                Name=\"VCXDCMakeTool\"\n"
        "            />\n"
        "            <Tool\n"
        "                Name=\"VCBscMakeTool\"\n"
        "            />\n"
        "            <Tool\n"
        "                Name=\"VCFxCopTool\"\n"
        "            />\n"
        "            <Tool\n"
        "                Name=\"VCPostBuildEventTool\"\n"
        "            />\n"
        "        </Configuration>\n";
}

static
void
writeVcproj(
    const FgConsProj &          proj,
    const map<string,string> &  guidMap)
{
    if (proj.srcGroups.size() == 0)
        fgThrow("Project has no source files",proj.name);
    string      projDir = proj.name + "/VisualStudio08/";
    fgCreateDirectory(projDir);
    string      projFile = projDir + proj.name + ".vcproj";
    ofstream    ofs(projFile.c_str());      // Text mode so newline = CR LF
    ofs << 
        "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>\n"
        "<VisualStudioProject\n"
        "    ProjectType=\"Visual C++\"\n"
        "    Version=\"9.00\"\n"
        "    Name=\"" << proj.name << "\"\n"
        "    ProjectGUID=\"" << proj.guid << "\"\n"
        "    RootNamespace=\"" << proj.name << "\"\n"
        "    Keyword=\"Win32Proj\"\n"
        "    TargetFrameworkVersion=\"196613\"\n"
        "    >\n"
        "    <Platforms>\n"
        "        <Platform\n"
        "            Name=\"Win32\"\n"
        "        />\n"
        "        <Platform\n"
        "            Name=\"x64\"\n"
        "        />\n"
        "    </Platforms>\n"
        "    <ToolFiles>\n"
        "    </ToolFiles>\n"
        "    <Configurations>\n";
    bool        pch = false;
    if (fgFileReadable(proj.name+"/"+proj.srcBaseDir+proj.srcGroups[0].dir+"stdafx.cpp"))
        pch = true;
    writeVcprojConfig(ofs,proj,false,false,pch);
    writeVcprojConfig(ofs,proj,false,true,pch);
    writeVcprojConfig(ofs,proj,true,false,pch);
    writeVcprojConfig(ofs,proj,true,true,pch);
    ofs <<
        "    </Configurations>\n"
        "    <References>\n";
    for (size_t ii=0; ii<proj.lnkDeps.size(); ++ii) {
        string  name = proj.lnkDeps[ii];
        map<string,string>::const_iterator  it = guidMap.find(name);
        if (it == guidMap.end())
            fgThrow("Link dependency to unknown library",name);
        string  guid = it->second;
        ofs <<
            "       <ProjectReference\n"
            "			ReferencedProjectIdentifier=\"" << guid << "\"\n"
            "			RelativePathToProject=\".\\" << name << "\\VisualStudio08\\" << name << ".vcproj\"\n"
            "		/>\n";
    }
    ofs <<
        "    </References>\n"
        "    <Files>\n";
    for (size_t ii=0; ii<proj.srcGroups.size(); ++ii)
        writeVcprojFiles(
            ofs,
            proj.srcBaseDir,
            proj.srcGroups[ii]);
    ofs <<
        "    </Files>\n"
        "    <Globals>\n"
        "    </Globals>\n"
        "</VisualStudioProject>\n";
}

void
fgConsVs2008(FgConsSolution sln)
{
    map<string,string>  guidMap;
    for (size_t ii=0; ii<sln.projects.size(); ++ii) {
        FgConsProj &    proj = sln.projects[ii];
        proj.guid = fgCreateMicrosoftGuid(proj.name);
        guidMap.insert(make_pair(proj.name,proj.guid));
    }
    for (size_t ii=0; ii<sln.projects.size(); ++ii)
        writeVcproj(sln.projects[ii],guidMap);
    ofstream        ofs("VisualStudio08.sln");  // Text mode so newline = CR LF
    ofs <<
        "\n"
        "Microsoft Visual Studio Solution File, Format Version 10.00\n"
        "# Visual Studio 2008\n";
    for (size_t ii=0; ii<sln.projects.size(); ++ii) {
        const FgConsProj & proj = sln.projects[ii];
        ofs <<
            // This GUID is just copied from a vs2008-generated .sln. It remains the same
            // for all projects even when new ones of different types are added in vs:
            "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \""
            << proj.name << "\", \"" << proj.name << "\\VisualStudio08\\"
            << proj.name << ".vcproj\", \"" << proj.guid << "\"\n"
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
