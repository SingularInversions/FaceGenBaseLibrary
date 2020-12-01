//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgCommand.hpp"
#include "FgFileSystem.hpp"
#include "FgSyntax.hpp"
#include "Fg3dMeshIo.hpp"
#include "FgTestUtils.hpp"
#include "FgKdTree.hpp"

using namespace std;

namespace Fg {

/**
   \ingroup Base_Commands
   Apply morphs by name to meshes.
 */
static
void
anim(CLArgs const & args)
{
    Syntax    syn(args,
        "<fileSuffix> (<mesh>.tri)+ (<morphName> <morphValue>)+\n"
        "    <fileSuffix> - Will be added to the mesh name for the respective output file.\n"
        "    <morphName>  - Exact name of morph to be applied. Need not exist on all meshes.\n"
        "    <morphValue> - Any floating point number (0 no application, 1 full application).\n"
        "COMMENTS:\n"
        "    - The output mesh will have no morphs defined as application of the morphs\n"
        "      to the vertex list necessarily invalidates the morph data."
        );
    string                          suffix = syn.next();
    vector<pair<string,Mesh> >  meshes;
    while (endsWith(syn.peekNext(),".tri")) {
        string                      name = syn.next();
        meshes.push_back(make_pair(name,loadTri(name)));
    }
    vector<pair<string,float> >     morphs;
    while (syn.more()) {
        string                      name = syn.next();
        morphs.push_back(make_pair(name,syn.nextAs<float>()));
    }
    for (auto & mp : meshes) {
        Floats              coord(mp.second.numMorphs(),0);
        for (auto const & rp : morphs) {
            Valid<size_t>     idx = mp.second.findMorph(rp.first);
            if (idx.valid())
                coord[idx.val()] = rp.second;
        }
        Vec3Fs         out;
        mp.second.morph(coord,out);
        mp.second.verts = out;
        // The morphs are invalidated once the base verts are changed:
        mp.second.deltaMorphs.clear();
        mp.second.targetMorphs.clear();
        Path          path(mp.first);
        path.base += suffix;
        saveTri(path.str(),mp.second);
    }
}

/**
   \ingroup Base_Commands
   Apply morphs by index number to a mesh.
 */
static
void
apply(CLArgs const & args)
{
    Syntax    syn(args,
        "<meshIn>.tri <meshOut>.<ext> ((d | t) <index> <value>)+\n"
        "    <ext>      - " + meshSaveFormatsCLDescription() + "\n"
        "    d          - Delta morph\n"
        "    t          - Target morph\n"
        "    <index>    - Morph index number (see 'morph list' command)\n"
        "    <value>    - Any floating point number (0: no application, 1: full application)\n"
        "COMMENTS:\n"
        "    - The output mesh will have no morphs defined as application of the morphs\n"
        "      to the vertex list necessarily invalidates the morph data.\n"
        "    - Note that the index number of the same morph will be different for different meshes."
        );
    string      inFile = syn.next(),
                outFile = syn.next();
    if (!checkExt(inFile,"tri"))
        syn.error("Not a TRI file",inFile);
    Mesh        mesh = loadTri(inFile);
    vector<float>   deltas(mesh.deltaMorphs.size(),0.0f),
                    targets(mesh.targetMorphs.size(),0.0f);
    while (syn.more()) {
        Ustring    arg = syn.nextLower();
        uint        idx = syn.nextAs<uint>();
        float       val = syn.nextAs<float>();

        //! Apply the morph:
        if (arg == "d") {
            if (idx >= deltas.size())
                fgThrow("Delta morph index out of bounds",toStr(idx));
            deltas[idx] = val;
        }
        else if (arg == "t") {
            if (idx >= targets.size())
                fgThrow("Target morph index out of bounds",toStr(idx));
            targets[idx] = val;
        }
        else
            syn.error("Invalid morph type",arg);
    }
    mesh.verts = mesh.morph(deltas,targets);
    // The morphs are invalidated once the base verts are changed:
    mesh.deltaMorphs.clear();
    mesh.targetMorphs.clear();
    saveMesh(mesh,outFile);
}

/**
   \ingroup Base_Commands
   Clamp delta morph vertex deltas to zero for seam vertices
 */
static
void
clamp(CLArgs const & args)
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
    for (Morph & morph : mesh.deltaMorphs)
        for (uint ii : clampVertInds)
            morph.verts[ii] = Vec3F(0);
    saveTri(syn.next(),mesh);
}

/**
   \ingroup Base_Commands
   Remove all animation morphs in a mesh
 */
static
void
clear(CLArgs const & args)
{
    Syntax    syn(args,"<mesh>.tri");
    string      name = syn.next();
    Mesh    mesh = loadTri(name);
    mesh.deltaMorphs.clear();
    mesh.targetMorphs.clear();
    saveTri(name,mesh);
}

/**
   \ingroup Base_Commands
   Copy animation morphs between meshes with corresponding vertex lists
 */
static
void
copymorphs(CLArgs const & args)
{
    Syntax    syn(args,
        "<meshIn>.tri <meshOut>.tri ((d | t) <index>)*\n"
        "    d          - Delta morph\n"
        "    t          - Target morph\n"
        "    <index>    - Morph index number (see 'morph list' command)\n"
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

/**
   \ingroup Base_Commands
   Create animation morphs for a mesh.
 */
static
void
create(CLArgs const & args)
{
    Syntax    syn(args,
        "<base>.tri <target>.<extIn> [-i] (d | t) <morphName>\n"
        "    <extIn> = " + meshLoadFormatsCLDescription() + "\n"
        "    <extOut> = " + meshSaveFormatsCLDescription() + "\n"
        "    -i         - Ignore very small morphs (ie. do not create)\n"
        "    d          - Delta morph\n"
        "    t          - Target morph\n"
        );
    string      baseName = syn.next();
    Mesh    base = loadTri(baseName);
    Mesh    target = loadMesh(syn.next());
    if (base.verts.size() != target.verts.size())
        fgThrow("Different number of vertices between base and target");
    bool        ignoreSmall = false;
    if (syn.peekNext() == "-i") {
        ignoreSmall = true;
        syn.next();
    }
    float       baseSz = cMaxElem(cDims(base.verts)),
                delSz = cMaxElem(cDims(target.verts-base.verts));
    if ((delSz / baseSz) < 0.00001) {
        if (ignoreSmall) {
            fgout << "Very small or zero morph ignored";
            return;
        }
        else
            fgout << "WARNING: Very small or zero morph";
    }
    string      type = syn.next();
    if (type == "d") {
        Morph    m;
        m.name = syn.next();
        m.verts = target.verts - base.verts;
        base.deltaMorphs.push_back(m);
    }
    else if (type == "t")
        base.addTargMorph(syn.next(),target.verts);
    else
        syn.error("Unrecognized morph type",type);
    saveTri(baseName,base);
}

/**
   \ingroup Base_Commands
   Extract all morphs to named OBJ files
 */
static
void
extract(CLArgs const & args)
{
    Syntax    syn(args,"<mesh>.tri <ext> [<base>]\n"
        "    <ext> - Output format " + meshSaveFormatsCLDescription() + "\n"
        "OUTPUTS:\n"
        "    <mesh>_<name>.<ext> for each morph target, or\n"
        "    <base>_<name>.<ext> if <base> is specified.\n"
        "NOTES:\n"
        "    ':', '(' and ')' characters are stripped from the morph names."
    );
    string      meshName = syn.next(),
                ext = syn.next();
    Ustring    baseName = pathToBase(meshName);
    if (syn.more())
        baseName = syn.next();
    Mesh    base = loadTri(meshName),
                out = base;
    out.deltaMorphs.clear();
    out.targetMorphs.clear();
    for (size_t ii=0; ii<base.numMorphs(); ++ii) {
        out.verts = base.morphSingle(ii);
        Ustring    morphName = removeChars(base.morphName(ii),":()");
        saveMesh(out,baseName+"_"+morphName+"."+ext);
    }
}

/**
   \ingroup Base_Commands
   Filter out morphs that do nothing
 */
static
void
filter(CLArgs const & args)
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
    for (Morph const & morph : in.deltaMorphs)
        if (cMaxElem(cMax(morph.verts)) > minDelta)
            out.deltaMorphs.push_back(morph);
    saveTri(syn.next(),out);
}

/**
   \ingroup Base_Commands
   List animation morphs in a mesh.
 */
static
void
morphList(CLArgs const & args)
{
    Syntax                  syn(args,
        "<mesh>.(tri|fgmesh)\n"
        "    Show available delta and target morphs\n");
    if (args.size() != 2)
        syn.incorrectNumArgs();
    string                  inFile = syn.next();
    Mesh                    mesh = loadMesh(inFile);
    Morphs const &          dmorphs = mesh.deltaMorphs;
    fgout << fgnl << dmorphs.size() << " delta morphs:" << fgpush;
    for (size_t ii=0; ii<dmorphs.size(); ++ii)
        fgout << fgnl << toStrDigits(ii,2) << " " << dmorphs[ii].name;
    IndexedMorphs const &   tmorphs = mesh.targetMorphs;
    fgout << fgpop << fgnl << tmorphs.size() << " target morphs:" << fgpush;
    for (size_t ii=0; ii<tmorphs.size(); ++ii)
        fgout << fgnl << toStrDigits(ii,2) << " " << tmorphs[ii].name;
    fgout << fgpop << fgnl;
}

/**
   \ingroup Base_Commands
   Remove brackets from animation morph names (some mesh formats do not support them).
 */
static
void
removebrackets(CLArgs const & args)
{
    Syntax    syn(args,
        "<meshIn>.tri <meshOut>.tri\n"
        );
    Mesh            mesh = loadTri(syn.next());
    Morphs &   dms = mesh.deltaMorphs;
    for (size_t ii=0; ii<dms.size(); ++ii)
        dms[ii].name = removeChars(removeChars(dms[ii].name,'('),')');
    IndexedMorphs &    tms = mesh.targetMorphs;
    for (size_t ii=0; ii<tms.size(); ++ii)
        tms[ii].name = removeChars(removeChars(tms[ii].name,'('),')');
    saveMesh(mesh,syn.next());
}

/**
   \ingroup Base_Commands
   Remove animation morphs from a mesh.
 */
static
void
removemorphs(CLArgs const & args)
{
    Syntax    syn(args,
        "<meshIn>.tri <meshOut>.tri ((d | t) <index>)+\n"
        "    d          - Delta morph\n"
        "    t          - Target morph\n"
        "    <index>    - Morph index number (see 'morph list' command)"
        );

    string      inFile = syn.next(),
                outFile = syn.next();
    if (!checkExt(inFile,"tri"))
        syn.error("Not a TRI file",inFile);
    if (!checkExt(outFile,"tri"))
        syn.error("Not a TRI file",outFile);
    Mesh    mesh = loadTri(inFile);
    vector<FgBool>  deltas(mesh.deltaMorphs.size(),true),
                    targets(mesh.targetMorphs.size(),true);
    while (syn.more()) {
        Ustring    arg = syn.nextLower();
        uint        idx = syn.nextAs<uint>();
        if (arg == "d") {
            if (idx >= deltas.size())
                fgThrow("Delta morph index out of bounds",toStr(idx));
            deltas[idx] = false;
        }
        else if (arg == "t") {
            if (idx >= targets.size())
                fgThrow("Target morph index out of bounds",toStr(idx));
            targets[idx] = false;
        }
        else
            syn.error("Invalid morph type",arg);
    }
    Mesh                meshOut = mesh;
    meshOut.deltaMorphs.clear();
    meshOut.targetMorphs.clear();
    for (size_t ii=0; ii<deltas.size(); ++ii)
        if (deltas[ii])
            meshOut.deltaMorphs.push_back(mesh.deltaMorphs[ii]);
    for (size_t ii=0; ii<targets.size(); ++ii)
        if (targets[ii])
            meshOut.targetMorphs.push_back(mesh.targetMorphs[ii]);
    saveMesh(meshOut,outFile);
}

/**
   \ingroup Base_Commands
   Rename an animation morph in a mesh.
 */
static
void
renameMorph(CLArgs const & args)
{
    Syntax    syn(args,
        "<mesh>.tri (d | t) <index> <name>\n"
        "    d          - Delta morph\n"
        "    t          - Target morph\n"
        "    <index>    - Morph index number (see 'morph list' command)"
        );
    string      fname = syn.next();
    if (!checkExt(fname,"tri"))
        syn.error("Not a TRI file",fname);
    Mesh    mesh = loadTri(fname);
    Ustring    arg = syn.nextLower();
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

static
void
morph(CLArgs const & args)
{
    Cmds   cmds {
        {anim,"anim","Apply morphs by name to multiple meshes"},
        {apply,"apply","Apply morphs by index number in a single mesh"},
        {clamp,"clamp","Clamp delta morphs to zero on seam vertices"},
        {clear,"clear","Clear all morphs from a mesh"},
        {copymorphs,"copy","Copy a morph between meshes with corresponding vertex lists"},
        {create,"create","Create morphs for a mesh"},
        {extract,"extract","Extract all morphs to named files"},
        {filter,"filter","Filter out delta morphs that do nothing"},
        {morphList,"list","List available morphs in a mesh"},
        {removemorphs,"remove","Remove morphs from a mesh"},
        {removebrackets,"removebrackets","Removes brackets from morphs names (for Maya)"},
        {renameMorph,"rename","Rename a morph in a mesh"}
    };
    doMenu(args,cmds);
}

Cmd
getMorphCmd()
{return Cmd(morph,"morph","List, apply or create animation morphs for 3D meshes"); }

void
fgMorphTest(CLArgs const & args)
{
    FGTESTDIR
    fgTestCopy("base/Jane.tri");
    runCmd(apply,"apply Jane.tri tmp.tri d 0 1 t 0 1");
    regressFile("base/test/JaneMorphBaseline.tri","tmp.tri");
}

}
