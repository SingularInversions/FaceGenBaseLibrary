//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgCommand.hpp"
#include "FgFileSystem.hpp"

#include "Fg3dMeshIo.hpp"
#include "FgTestUtils.hpp"
#include "FgKdTree.hpp"
#include "FgParse.hpp"

using namespace std;

namespace Fg {

namespace {

void                cmdMorphAnim(CLArgs const & args)
{
    Syntax              syn {args,R"(<suffix> (<mesh>.(tri|fgmesh) )+ (<morphName> <morphValue>)
    <suffix>     - Will be added to the mesh name for the respective output file.
    <mesh>       - The meshes with morphs
    <morphName>  - Exact name of morph to be applied. Need not exist on all meshes.
    <morphValue> - Any floating point number (0 no application, 1 full application).
OUTPUT:
    <mesh><suffix>.(tri|fgmesh) will be created for each specified <mesh>
COMMENTS:
    * The output mesh(es) will have no morphs defined)"
    };
    String              suffix = syn.next();
    Meshes              meshes;
    while (endsWith(syn.peekNext(),".tri") || endsWith(syn.peekNext(),".fgmesh")) {
        meshes.push_back(loadMesh(syn.next()));
        meshes.back().name = syn.curr();
    }
    Svec<pair<String,float>>    morphs;
    while (syn.more()) {
        String                      name = syn.next();
        morphs.push_back(make_pair(name,syn.nextAs<float>()));
    }
    for (auto & mesh : meshes) {
        Floats              coord (mesh.numMorphs(),0);
        for (auto const & nv : morphs) {
            Valid<size_t>       idx = mesh.findMorph(nv.first);
            if (idx.valid())
                coord[idx.val()] = nv.second;
        }
        Vec3Fs          out;
        mesh.morph(coord,out);
        mesh.verts = out;
        // The morphs are invalidated once the base verts are changed:
        mesh.deltaMorphs.clear();
        mesh.targetMorphs.clear();
        Path          path (mesh.name);
        path.base += suffix;
        saveMesh(mesh,path.str());
    }
}

void                cmdMorphApply(CLArgs const & args)
{
    Syntax              syn {args,
        R"(<meshIn>.<exti> <meshOut>.<exto> (<specifier> <value>)+
    <exti>      - )" + getClOptionsString(getMeshNativeFormats()) +  R"(
    <ext>       - )" + getMeshSaveExtsCLDescription() + R"(
    <specifier> - (<index> | <name>)
    <index>     - (d | t) <number>
    d           - delta morph 
    t           - target morph
    <number>    - respective morph index number from 'morph list' command
    <name>      - the name of the morph (use quotes if it contains spaces)
    <value>     - Any floating point number (0: no application, 1: full application)
COMMENTS:
    - The output mesh will have no morphs defined since application of the morphs to the vertex
      list necessarily invalidates the morph data.
    - If using the <index> specifier, note that <number> of the same morph will be different for
      different meshes.)"
    };
    string              inFile = syn.next(),
                        outFile = syn.next();
    Mesh                mesh = loadMesh(inFile);
    Floats              deltas (mesh.deltaMorphs.size(),0.0f),
                        targets (mesh.targetMorphs.size(),0.0f);
    while (syn.more()) {
        String              spec = syn.next();
        if (spec == "d") {
            uint                idx = syn.nextAs<uint>();
            if (idx >= deltas.size())
                fgThrow("Delta morph index out of bounds",toStr(idx));
            deltas[idx] = syn.nextAs<float>();
        }
        else if (spec == "t") {
            uint                idx = syn.nextAs<uint>();
            if (idx >= targets.size())
                fgThrow("Target morph index out of bounds",toStr(idx));
            targets[idx] = syn.nextAs<float>();
        }
        else {
            String8             name {spec};
            size_t              idx = findFirstIdx(mesh.deltaMorphs,name);
            if (idx < mesh.deltaMorphs.size()) {
                deltas[idx] = syn.nextAs<float>();
                continue;
            }
            idx = findFirstIdx(mesh.targetMorphs,name);
            if (idx < mesh.targetMorphs.size())
                targets[idx] = syn.nextAs<float>();
            else
                syn.error("Morph name not found",spec);
        }
    }
    mesh.verts = mesh.morph(deltas,targets);
    // The morphs are invalidated once the base verts are changed:
    mesh.deltaMorphs.clear();
    mesh.targetMorphs.clear();
    saveMesh(mesh,outFile);
}

void                cmdMorphClampSeam(CLArgs const & args)
{
    Syntax    syn(args,"<in>.tri (v | m) <seam>.tri <out>.tri\n"
        "    v - All vertices in <seam>.tri will be used to define the seam.\n"
        "    m - Only marked vertices in <seam>.tri will defin the seam.\n"
        "NOTES:\n"
        "    * Only delta morphs are affected.\n"
        "    * Vertex matching tolerance is 1/10,000 of the mesh max dimension."
    );
    Mesh        mesh = loadTri(syn.next());
    string          vm = syn.next();
    Vec3Fs         seam;
    if (vm == "v")
        seam = loadTri(syn.next()).verts;
    else if (vm == "m")
        seam = loadTri(syn.next()).markedVertPositions();
    else
        syn.error("Not a valid option",vm);
    if (seam.size() < 3)
        syn.error("Too few vertices to be a seam",toStr(seam.size()));
    KdTree        kd(seam);
    float           scale = cMaxElem(cDims(mesh.verts)),
                    closeSqr = sqr(scale / 10000.0f);
    set<uint>       clampVertInds;
    for (size_t ii=0; ii<mesh.verts.size(); ++ii)
        if (kd.findClosest(mesh.verts[ii]).distMag < closeSqr)
            clampVertInds.insert(uint(ii));
    for (DirectMorph & morph : mesh.deltaMorphs)
        for (uint ii : clampVertInds)
            morph.verts[ii] = Vec3F(0);
    saveTri(syn.next(),mesh);
}

void                cmdMorphClear(CLArgs const & args)
{
    Syntax              syn {args,"<in>.(fgmesh|tri) <out>.<ext>"};
    Mesh                mesh = loadMesh(syn.next());
    mesh.deltaMorphs.clear();
    mesh.targetMorphs.clear();
    saveMesh(mesh,syn.next());
}

void                cmdMorphCopy(CLArgs const & args)
{
    Syntax    syn(args,
        "<meshIn>.tri <meshOut>.tri ((d | t) <index>)*\n"
        "    d          - Delta morph\n"
        "    t          - Target morph\n"
        "    <index>    - morph index number (see 'morph list' command)\n"
        "COMMENTS:\n"
        "    If no morphs are specified, all morphs are copied *and destination morphs are overwritten*.\n"
        "    The meshes must have the same number of vertices and those vertices should correspond\n"
        "    to the same semantic locations."
        );

    string      nameIn = syn.next(),
                nameOut = syn.next();
    if (nameIn == nameOut)
        syn.error("Input and output meshes must be different");
    Mesh    meshIn = loadTri(nameIn);
    Mesh    meshOut = loadTri(nameOut);
    if (meshIn.verts.size() != meshOut.verts.size())
        syn.error("Meshes have different vertex counts");
    if (!syn.more()) {           // All morphs
        meshOut.deltaMorphs = meshIn.deltaMorphs;
        meshOut.targetMorphs = meshIn.targetMorphs;
    }
    while (syn.more()) {
        bool        delta = true;
        if (syn.next() == "t")
            delta = false;
        else if (syn.curr() != "d")
            syn.error("Invalid morph type flag",syn.curr());
        size_t      idx = syn.nextAs<size_t>();
        if (delta) {
            if (idx >= meshIn.deltaMorphs.size())
                syn.error("Invalid delta morph index",toStr(idx));
            meshOut.addDeltaMorph(meshIn.deltaMorphs[idx]);
        }
        else {
            if (idx >= meshIn.targetMorphs.size())
                syn.error("Invalid target morph index",toStr(idx));
            meshOut.addTargMorph(meshIn.targetMorphs[idx]);
        }
    }
    saveTri(nameOut,meshOut);
}

void                cmdMorphAdd(CLArgs const & args)
{
    Syntax              syn {args,R"([-i] <base>.<ext0> (d | t) (<targets> | <targsFile>.txt)
    -i              - ignore very small morphs (ie. do not create)
    <base>          - add morph to this mesh
    <ext0>          - (fgmesh | tri)
    (d | t)         - specify delta (aka blendshape) or target morph (target shape follows shape changes)
    <targets>       - (<targ>.<ext1> <morphName>)+
    <targ>          - target shape mesh containing the same number of vertices as <base>
    <ext1>          - )" + getMeshLoadExtsCLDescription() + R"(
    <targsFile>.txt - each line must be of the form <targ>.<ext1>, where the morph will be named <targ>
OUTPUT:
    <base>.<ext0> is saved with the new morph(s) added)"
    };
    bool                ignoreSmall = false;
    if (syn.peekNext() == "-i") {
        ignoreSmall = true;
        syn.next();
    }
    String              baseName = syn.next();
    Mesh                base = loadMesh(baseName);
    bool                asDeltaMorph = true;            // false == target morph
    if (syn.next() == "d")
        asDeltaMorph = true;
    else if (syn.curr() == "t")
        asDeltaMorph = false;
    else
        syn.error("Unknown morph type selection",syn.curr());
    Strings             morphFiles,
                        morphNames;
    if (endsWith(syn.peekNext(),".txt")) {
        morphFiles = splitWhitespace(loadRawString(syn.next()));
        morphNames = mapCall(morphFiles,[](String const & s){return pathToBase(String8{s}).m_str;});
    }
    else {
        while (syn.more()) {
            morphFiles.push_back(syn.next());
            morphNames.push_back(syn.next());
        }
    }
    float               baseSz = cMaxElem(cDims(base.verts)),
                        epsilon = baseSz * epsBits(17);         // less than 1 in 100,000 relative to max dim
    for (size_t mm=0; mm<morphFiles.size(); ++mm) {
        PushIndent          pind {morphFiles[mm]};
        Mesh                target = loadMesh(morphFiles[mm]);
        if (base.verts.size() != target.verts.size())
            fgThrow("Different number of vertices between base and target");
        size_t              epsDiffs {0},
                            sigDiffs {0};
        for (size_t vv=0; vv<base.verts.size(); ++vv) {
            float               del = cMaxElem(mapAbs(target.verts[vv] - base.verts[vv]));
            if (del > epsilon)
                ++sigDiffs;
            else
                ++epsDiffs;
        }
        fgout << fgnl << "Vertices that differed signficantly: " << sigDiffs
            << fgnl << "Vertices that differed insignificantly: " << epsDiffs
            << fgnl << "Total number of vertices: " << base.verts.size();
        if (!ignoreSmall)
            sigDiffs += epsDiffs;
        if (sigDiffs == 0) {
            fgout << fgnl << "No morph created";
            continue;
        }
        if (asDeltaMorph)
            base.deltaMorphs.emplace_back(morphNames[mm],target.verts-base.verts);
        else
            base.addTargMorph(morphNames[mm],target.verts);
    }
    saveMesh(base,baseName);
}

void                cmdMorphExport(CLArgs const & args)
{
    Syntax              syn {args,R"(<mesh>.(fgmesh | tri) <ext> [<base>] [-t]
    <mesh>          - mesh from which to export all morphs
    <ext>           - output format )" + getMeshSaveExtsCLDescription() + R"("
    <base>          - base name for saved meshes. Defaults to <mesh> if not specified
    -t              - add a trailing underscore to target morph names
OUTPUTS:
    <base>_<name>.<ext> is saved for each morph, where <name> is the name of the morph.
NOTES:
    ':', '(' and ')' characters are stripped from the morph names.)"
    };
    Mesh                mesh = loadMesh(syn.next());
    String8             base = pathToBase(syn.curr());
    String              ext = "." + syn.next();
    bool                markTarget = false;
    while (syn.more()) {
        String              opt = syn.next();
        if (opt == "-t")
            markTarget = true;
        else
            base = opt;
    }
    Mesh                out = mesh;
    out.deltaMorphs.clear();
    out.targetMorphs.clear();
    for (size_t ii=0; ii<mesh.numMorphs(); ++ii) {
        out.verts = mesh.morphSingle(ii);
        String8             morphName = removeChars(mesh.morphName(ii),":()");
        if (markTarget && ii >= mesh.deltaMorphs.size())
            morphName += "_";
        saveMesh(out,base+"_"+morphName+ext);
    }
}

void                cmdMorphFilter(CLArgs const & args)
{
    Syntax    syn(args,"<in>.tri <out>.tri\n"
        "NOTES:\n"
        "    Removes all delta morphs that have little or no effect."
    );
    Mesh            in = loadTri(syn.next()),
                    out = in;
    out.deltaMorphs.clear();
    float           dim = cMaxElem(cDims(in.verts)),
                    minDelta = dim * 0.001;
    for (DirectMorph const & morph : in.deltaMorphs)
        if (cMaxElem(mapLen(morph.verts)) > minDelta)
            out.deltaMorphs.push_back(morph);
    saveTri(syn.next(),out);
}

void                cmdMorphList(CLArgs const & args)
{
    Syntax                  syn(args,
        "<mesh>.(tri|fgmesh)\n"
        "    Show available delta and target morphs\n");
    if (args.size() != 2)
        syn.errorNumArgs();
    string                  inFile = syn.next();
    Mesh                    mesh = loadMesh(inFile);
    DirectMorphs const &    dmorphs = mesh.deltaMorphs;
    fgout << fgnl << dmorphs.size() << " delta morphs:" << fgpush;
    for (size_t ii=0; ii<dmorphs.size(); ++ii)
        fgout << fgnl << toStrDigits(ii,2) << " " << dmorphs[ii].name;
    IndexedMorphs const &   tmorphs = mesh.targetMorphs;
    fgout << fgpop << fgnl << tmorphs.size() << " target morphs:" << fgpush;
    for (size_t ii=0; ii<tmorphs.size(); ++ii)
        fgout << fgnl << toStrDigits(ii,2) << " " << tmorphs[ii].name;
    fgout << fgpop << fgnl;
}

void                removebrackets(CLArgs const & args)
{
    Syntax    syn(args,
        "<meshIn>.tri <meshOut>.tri\n"
        );
    Mesh            mesh = loadTri(syn.next());
    DirectMorphs &  dms = mesh.deltaMorphs;
    for (size_t ii=0; ii<dms.size(); ++ii)
        dms[ii].name = removeChars(removeChars(dms[ii].name,'('),')');
    IndexedMorphs &    tms = mesh.targetMorphs;
    for (size_t ii=0; ii<tms.size(); ++ii)
        tms[ii].name = removeChars(removeChars(tms[ii].name,'('),')');
    saveMesh(mesh,syn.next());
}

void                cmdMorphRemove(CLArgs const & args)
{
    Syntax              syn {args,R"(<in>.<ext> <out>.<ext> ((d | t) <index>)+
    <ext>       - (fgmesh | tri)
    d           - delta morph
    t           - target morph
    <index>     - morph index number (see 'morph list' command)
OUTPUT:
    <out>.<ext> - identical to <in>.<ext> except with the specified morph removed)"
    };
    Mesh                in = loadMesh(syn.next());
    String              outName = syn.next();
    Bools               delts (in.deltaMorphs.size(),true),
                        targs (in.targetMorphs.size(),true);
    do {
        String              type = syn.next();
        uint                idx = syn.nextAs<uint>();
        if (type == "d") {
            if (idx < delts.size())
                delts[idx] = false;
            else
                fgThrow("Delta morph index out of bounds",toStr(idx));
        }
        else if (type == "t") {
            if (idx < targs.size())
                targs[idx] = false;
            else
                fgThrow("Target morph index out of bounds",toStr(idx));
        }
        else
            syn.error("Invalid morph type",type);
    } while (syn.more());
    Mesh                out = in;
    out.deltaMorphs = selectIf(in.deltaMorphs,delts);
    out.targetMorphs = selectIf(in.targetMorphs,targs);
    saveMesh(out,outName);
}

void                cmdMorphRename(CLArgs const & args)
{
    Syntax    syn(args,
        "<mesh>.tri (d | t) <index> <name>\n"
        "    d          - Delta morph\n"
        "    t          - Target morph\n"
        "    <index>    - morph index number (see 'morph list' command)"
        );
    string      fname = syn.next();
    if (!checkExt(fname,"tri"))
        syn.error("Not a TRI file",fname);
    Mesh    mesh = loadTri(fname);
    String8    arg = syn.nextLower();
    uint        idx = syn.nextAs<uint>();
    if (arg == "d") {
        if (idx >= mesh.deltaMorphs.size())
            fgThrow("Delta morph index out of bounds",toStr(idx));
        mesh.deltaMorphs[idx].name = syn.next();
    }
    else if (arg == "t") {
        if (idx >= mesh.targetMorphs.size())
            fgThrow("Target morph index out of bounds",toStr(idx));
        mesh.targetMorphs[idx].name = syn.next();
    }
    else
        syn.error("Invalid morph type",arg);
    saveTri(fname,mesh);
}

void                cmdMorphSquelch(CLArgs const & args)
{
    Syntax              syn {args,R"(<in>.<ext> <out>.<ext> [<bitDepth>]
    <ext>           - ( fgmesh | tri )
    <bitDepth>      - [10,20] bits of precision to consider as significant. Defaults to 16.
OUTPUT:
    <out>.<ext>     - identical mesh with morphs squelched at given <bitDepth>
NOTES:
    removes all vertex changes which are smaller than the given relative threshold with respect to
    the maximum dimension of the vertex list boundaries. With 16 bits precision, vertices that move less
    than 1/2^16 ~ 1/65536 of the max boundary size (at morph coefficient 1) will be set to zero change.
    All morphs are affected.)"
    };
    Mesh                mesh = loadMesh(syn.next());
    String              outName = syn.next();
    uint                bitDepth = 16;
    if (syn.more())
        bitDepth = syn.nextAs<uint>();
    if ((bitDepth<10)||(bitDepth>20))
        syn.error("invalid value for <bitDepth>",syn.curr());
    float               maxDim = cMaxElem(cDims(mesh.verts)),
                        maxDel = maxDim * epsBits(bitDepth);
    for (DirectMorph & dm : mesh.deltaMorphs) {
        size_t              cntDel {0};
        for (Vec3F & delta : dm.verts) {
            float           delmax = cMaxElem(mapAbs(delta));
            if ((delmax > 0) && (delmax < maxDel)) {
                delta = Vec3F{0};
                ++cntDel;
            }
        }
        if (cntDel > 0)
            fgout << fgnl << dm.name << " : " << cntDel << " deltas set to zero.";
    }
    for (IndexedMorph & im : mesh.targetMorphs) {
        IdxVec3Fs           ivs;
        for (IdxVec3F const & iv : im.ivs) {
            Vec3F               delta = iv.vec - mesh.verts[iv.idx];
            float               delmax = cMaxElem(mapAbs(delta));
            if (delmax >= maxDel)
                ivs.push_back(iv);
        }
        if (ivs.size() < im.ivs.size()) {
            fgout << fgnl << im.name << " : " << im.ivs.size()-ivs.size() << " deltas set to zero.";
            im.ivs = ivs;
        }
    }
    saveMesh(mesh,outName);
}

}

void                cmdMorph(CLArgs const & args)
{
    Cmds                cmds {
        {cmdMorphAdd,"add","Add morph target(s) to a mesh"},
        {cmdMorphAnim,"anim","Apply morphs by name to multiple meshes"},
        {cmdMorphApply,"apply","Apply morphs by index number in a single mesh"},
        {cmdMorphClampSeam,"clampseam","Clamp delta morphs to zero on seam vertices"},
        {cmdMorphClear,"clear","Clear all morphs from a mesh"},
        {cmdMorphCopy,"copy","Copy a morph between meshes with corresponding vertex lists"},
        {cmdMorphExport,"export","Export all morphs to mesh files"},
        {cmdMorphFilter,"filter","Filter out delta morphs that do nothing"},
        {cmdMorphList,"list","List available morphs in a mesh"},
        {cmdMorphRemove,"remove","Remove morphs from a mesh"},
        {removebrackets,"removebrackets","Removes brackets from morphs names (for Maya)"},
        {cmdMorphRename,"rename","Rename a morph in a mesh"},
        {cmdMorphSquelch,"squelch","Remove insignificant deltas from morphs"},
    };
    doMenu(args,cmds);
}

void                testMorph(CLArgs const & args)
{
    FGTESTDIR
    copyDataFileToCurr("base/Jane.tri");
    runCmd(cmdMorphApply,"apply Jane.tri tmp.tri d 0 1 t 0 1");
    testRegressApprox<Mesh>(
        loadTri("tmp.tri"),
        "base/test/JaneMorphBaseline.tri",
        [](Mesh const & l,Mesh const & r){return isApproxEqual(l.verts,r.verts,epsBits(12)); },
        [](String8 const & fn){return loadMesh(fn); },
        [](Mesh const & m,String8 const & fn){saveMesh(m,fn); });

}

}
