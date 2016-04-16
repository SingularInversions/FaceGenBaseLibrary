//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: August 17, 2013
//

#include "stdafx.h"

#include "FgCommand.hpp"
#include "FgFileSystem.hpp"
#include "FgSyntax.hpp"
#include "Fg3dMeshIo.hpp"
#include "FgTestUtils.hpp"

using namespace std;

/**
   \ingroup Base_Commands
   Command to apply animation morphs to a mesh.
 */
static
void
apply(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<meshIn>.tri <meshOut>.<ext> ((d | t) <index> <value>)+\n"
        "    <ext>      - " + fgSaveMeshFormatsDescription() + "\n"
        "    d          - Delta morph\n"
        "    t          - Target morph\n"
        "    <index>    - Morph index number (see 'morph list' command)\n"
        "    <value>    - Any floating point number (0: no application, 1: full application)\n"
        "COMMENTS:\n"
        "    The output mesh will have no morphs defined as application of the morphs\n"
        "    to the vertex list necessarily invalidates the morph data"
        );

    string      inFile = syntax.next(),
                outFile = syntax.next();
    if (!fgCheckSetExtension(inFile,"tri"))
        syntax.error("Not a TRI file",inFile);
    Fg3dMesh        mesh = fgLoadTri(inFile);
    vector<float>   deltas(mesh.deltaMorphs.size(),0.0f),
                    targets(mesh.targetMorphs.size(),0.0f);
    while (syntax.more()) {
        string      arg = syntax.nextLower();
        uint        idx = fgFromString<uint>(syntax.next());
        float       val = fgFromString<float>(syntax.next());

        //! Apply the morph:
        if (arg == "d") {
            if (idx >= deltas.size())
                fgThrow("Delta morph index out of bounds",fgToString(idx));
            deltas[idx] = val;
        }
        else if (arg == "t") {
            if (idx >= targets.size())
                fgThrow("Target morph index out of bounds",fgToString(idx));
            targets[idx] = val;
        }
        else
            syntax.error("Invalid morph type",arg);
    }
    mesh.verts = mesh.morph(deltas,targets);
    // The morphs are invalidated once the base verts are changed:
    mesh.deltaMorphs.clear();
    mesh.targetMorphs.clear();
    fgSaveMeshAnyFormat(mesh,outFile);
}

/**
   \ingroup Base_Commands
   Command to remove all animation morphs in a mesh
 */
static
void
clear(const FgArgs & args)
{
    FgSyntax    syntax(args,"<mesh>.tri");
    string      name = syntax.next();
    Fg3dMesh    mesh = fgLoadTri(name);
    mesh.deltaMorphs.clear();
    mesh.targetMorphs.clear();
    fgSaveTri(name,mesh);
}

/**
   \ingroup Base_Commands
   Command to copy animation morphs between meshes with corresponding vertex lists
 */
static
void
copymorphs(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<meshIn>.tri <meshOut>.tri ((d | t) <index>)*\n"
        "    d          - Delta morph\n"
        "    t          - Target morph\n"
        "    <index>    - Morph index number (see 'morph list' command)\n"
        "COMMENTS:\n"
        "    If no morphs are specified, all morphs are copied *and destination morphs are overwritten*.\n"
        "    The meshes must have the same number of vertices and those vertices should correspond\n"
        "    to the same semantic locations."
        );

    string      nameIn = syntax.next(),
                nameOut = syntax.next();
    if (nameIn == nameOut)
        syntax.error("Input and output meshes must be different");
    Fg3dMesh    meshIn = fgLoadTri(nameIn);
    Fg3dMesh    meshOut = fgLoadTri(nameOut);
    if (meshIn.verts.size() != meshOut.verts.size())
        syntax.error("Meshes have different vertex counts");
    if (!syntax.more()) {           // All morphs
        meshOut.deltaMorphs = meshIn.deltaMorphs;
        meshOut.targetMorphs = meshIn.targetMorphs;
    }
    while (syntax.more()) {
        bool        delta = true;
        if (syntax.next() == "t")
            delta = false;
        else if (syntax.curr() != "d")
            syntax.error("Invalid morph type flag",syntax.curr());
        size_t      idx = fgFromString<size_t>(syntax.next());
        if (delta) {
            if (idx >= meshIn.deltaMorphs.size())
                syntax.error("Invalid delta morph index",fgToString(idx));
            meshOut.addDeltaMorph(meshIn.deltaMorphs[idx]);
        }
        else {
            if (idx >= meshIn.targetMorphs.size())
                syntax.error("Invalid target morph index",fgToString(idx));
            meshOut.addTargMorph(meshIn.targetMorphs[idx]);
        }
    }
    fgSaveTri(nameOut,meshOut);
}

/**
   \ingroup Base_Commands
   Command to create animation morphs for a mesh.
 */
static
void
create(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<base>.tri <target>.<extIn> [-i] (d | t) <morphName>\n"
        "    <extIn> = " + fgLoadMeshFormatsDescription() + "\n"
        "    <extOut> = " + fgSaveMeshFormatsDescription() + "\n"
        "    -i         - Ignore very small morphs (ie. do not create)\n"
        "    d          - Delta morph\n"
        "    t          - Target morph\n"
        );
    string      baseName = syntax.next();
    Fg3dMesh    base = fgLoadTri(baseName);
    Fg3dMesh    target = fgLoadMeshAnyFormat(syntax.next());
    if (base.verts.size() != target.verts.size())
        fgThrow("Different number of vertices between base and target");
    bool        ignore = false;
    if (syntax.peekNext() == "-i") {
        ignore = true;
        syntax.next();
    }
    float       baseSz = fgMaxElem(fgDims(base.verts)),
                delSz = fgMaxElem(fgDims(target.verts-base.verts));
    if ((delSz / baseSz) < 0.00001) {
        if (ignore) {
            fgout << "Very small or zero morph ignored";
            return;
        }
        else
            fgout << "WARNING: Very small or zero morph";
    }
    string      type = syntax.next();
    if (type == "d") {
        FgMorph    m;
        m.name = syntax.next();
        m.verts = target.verts - base.verts;
        base.deltaMorphs.push_back(m);
    }
    else if (type == "t")
        base.addTargMorph(syntax.next(),target.verts);
    else
        syntax.error("Unrecognized morph type",type);
    fgSaveTri(baseName,base);
}

/**
   \ingroup Base_Commands
   Command to list animation morphs in a mesh.
 */
static
void
morphList(const FgArgs & args)
{
    FgSyntax    syntax(args,
            "<mesh>.tri\n"
            "    Show available delta and target morphs\n");
    if (args.size() != 2)
        syntax.incorrectNumArgs();
    string      inFile = syntax.next();
    if (!fgCheckSetExtension(inFile,"tri"))
        syntax.error("Not a TRI file",inFile);
    Fg3dMesh    mesh = fgLoadTri(inFile);
    const vector<FgMorph> & dmorphs = mesh.deltaMorphs;
    fgout << fgnl << dmorphs.size() << " delta morphs:" << fgpush;
    for (size_t ii=0; ii<dmorphs.size(); ++ii)
        fgout << fgnl << fgToStringDigits(ii,2) << " " << dmorphs[ii].name;
    const vector<FgIndexedMorph> &   tmorphs = mesh.targetMorphs;
    fgout << fgpop << fgnl << tmorphs.size() << " target morphs:" << fgpush;
    for (size_t ii=0; ii<tmorphs.size(); ++ii)
        fgout << fgnl << fgToStringDigits(ii,2) << " " << tmorphs[ii].name;
    fgout << fgpop << fgnl;
}

/**
   \ingroup Base_Commands
   Command to remove animation morphs from a mesh.
 */
static
void
removebrackets(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<meshIn>.tri <meshOut>.tri\n"
        );
    Fg3dMesh            mesh = fgLoadTri(syntax.next());
    vector<FgMorph> &   dms = mesh.deltaMorphs;
    for (size_t ii=0; ii<dms.size(); ++ii)
        dms[ii].name = fgRemoveChars(fgRemoveChars(dms[ii].name,'('),')');
    vector<FgIndexedMorph> &    tms = mesh.targetMorphs;
    for (size_t ii=0; ii<tms.size(); ++ii)
        tms[ii].name = fgRemoveChars(fgRemoveChars(tms[ii].name,'('),')');
    fgSaveMeshAnyFormat(mesh,syntax.next());
}

/**
   \ingroup Base_Commands
   Command to remove animation morphs from a mesh.
 */
static
void
removemorphs(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<meshIn>.tri <meshOut>.tri ((d | t) <index>)+\n"
        "    d          - Delta morph\n"
        "    t          - Target morph\n"
        "    <index>    - Morph index number (see 'morph list' command)"
        );

    string      inFile = syntax.next(),
                outFile = syntax.next();
    if (!fgCheckSetExtension(inFile,"tri"))
        syntax.error("Not a TRI file",inFile);
    if (!fgCheckSetExtension(outFile,"tri"))
        syntax.error("Not a TRI file",outFile);
    Fg3dMesh    mesh = fgLoadTri(inFile);
    vector<FgBool>  deltas(mesh.deltaMorphs.size(),true),
                    targets(mesh.targetMorphs.size(),true);
    while (syntax.more()) {
        string      arg = syntax.nextLower();
        uint        idx = fgFromString<uint>(syntax.next());

        if (arg == "d") {
            if (idx >= deltas.size())
                fgThrow("Delta morph index out of bounds",fgToString(idx));
            deltas[idx] = false;
        }
        else if (arg == "t") {
            if (idx >= targets.size())
                fgThrow("Target morph index out of bounds",fgToString(idx));
            targets[idx] = false;
        }
        else
            syntax.error("Invalid morph type",arg);
    }
    Fg3dMesh                meshOut = mesh;
    meshOut.deltaMorphs.clear();
    meshOut.targetMorphs.clear();
    for (size_t ii=0; ii<deltas.size(); ++ii)
        if (deltas[ii])
            meshOut.deltaMorphs.push_back(mesh.deltaMorphs[ii]);
    for (size_t ii=0; ii<targets.size(); ++ii)
        if (targets[ii])
            meshOut.targetMorphs.push_back(mesh.targetMorphs[ii]);
    fgSaveMeshAnyFormat(meshOut,outFile);
}

/**
   \ingroup Base_Commands
   Command to rename an animation morph in a mesh.
 */
static
void
renameMorph(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<mesh>.tri (d | t) <index> <name>\n"
        "    d          - Delta morph\n"
        "    t          - Target morph\n"
        "    <index>    - Morph index number (see 'morph list' command)"
        );
    string      fname = syntax.next();
    if (!fgCheckSetExtension(fname,"tri"))
        syntax.error("Not a TRI file",fname);
    Fg3dMesh    mesh = fgLoadTri(fname);
    string      arg = syntax.nextLower();
    uint        idx = fgFromString<uint>(syntax.next());
    if (arg == "d") {
        if (idx >= mesh.deltaMorphs.size())
            fgThrow("Delta morph index out of bounds",fgToString(idx));
        mesh.deltaMorphs[idx].name = syntax.next();
    }
    else if (arg == "t") {
        if (idx >= mesh.targetMorphs.size())
            fgThrow("Target morph index out of bounds",fgToString(idx));
        mesh.targetMorphs[idx].name = syntax.next();
    }
    else
        syntax.error("Invalid morph type",arg);
    fgSaveTri(fname,mesh);
}

static
void
morph(const FgArgs & args)
{
    vector<FgCmd>   cmds;
    cmds.push_back(FgCmd(apply,"apply","Apply morphs in a mesh with morphs"));
    cmds.push_back(FgCmd(clear,"clear","Clear all morphs from a mesh"));
    cmds.push_back(FgCmd(copymorphs,"copy","Copy a morph between meshes with corresponding vertex lists"));
    cmds.push_back(FgCmd(create,"create","Create morphs for a mesh"));
    cmds.push_back(FgCmd(morphList,"list","List available morphs in a mesh"));
    cmds.push_back(FgCmd(removemorphs,"remove","Remove morphs from a mesh"));
    cmds.push_back(FgCmd(removebrackets,"removebrackets","Removes brackets from morphs names (for Maya)"));
    cmds.push_back(FgCmd(renameMorph,"rename","Rename a morph in a mesh"));
    fgMenu(args,cmds);
}

FgCmd
fgCmdMorphInfo()
{return FgCmd(morph,"morph","List, apply or create morphs for 3D meshes"); }

void
fgMorphTest(const FgArgs & args)
{
    FGTESTDIR
    fgTestCopy("base/Jane.tri");
    fgTestCmd(apply,"apply Jane.tri tmp.tri d 0 1 t 0 1");
    FgString        baseline = fgDataDir()+"base/test/JaneMorphBaseline.tri";
    if (!fgExists(baseline) || !fgBinaryFileCompare("tmp.tri",baseline)) {
        if (fgRegressOverwrite())
            fgCopyFile("tmp.tri",baseline,true);
        fgThrow("Morph test failed");
    }
}
