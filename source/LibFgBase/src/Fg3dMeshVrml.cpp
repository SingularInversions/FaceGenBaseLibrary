//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "Fg3dMeshIo.hpp"
#include "FgFile.hpp"
#include "FgFileSystem.hpp"
#include "FgParse.hpp"
#include "FgMain.hpp"
#include "FgTestUtils.hpp"

using namespace std;

namespace Fg {

template<size_t dim>
void
writePoint(Ofstream & ofs,Mat<float,dim,1> const & pnt)
{
    ofs << "               ";
    for (uint kk=0; kk<dim; ++kk)
        ofs << " " << pnt[kk];
}

template<size_t dim>
void
writePoints(Ofstream & ofs,vector<Mat<float,dim,1> > const & pts)
{
    if (pts.empty())
        return;
    ofs <<
        "            point\n"
        "            [\n";
    writePoint(ofs,pts[0]);
    for (size_t jj=1; jj<pts.size(); ++jj) {
        ofs << ",\n";
        writePoint(ofs,pts[jj]);
    }
    ofs << 
        "\n            ]\n";
}

template<size_t dim>
void
writeIdx(Ofstream & ofs,Arr<uint,dim> const & idx)
{
    ofs << "            ";
    for (uint ii=0; ii<dim; ++ii)
        ofs << idx[ii] << ", ";
    ofs << "-1";
}

template<size_t dim>
void
writeIndices(Ofstream & ofs,vector<Arr<uint,dim> > const &  inds)
{
    if (inds.size() == 0)
        return;
    writeIdx(ofs,inds[0]);
    for (size_t ii=1; ii<inds.size(); ++ii) {
        ofs << ",\n";
        writeIdx(ofs,inds[ii]);
    }
}

void
saveVrml(
    String8 const &         filename,
    Meshes const &          meshes,
    String                  imgFormat)
{
    FGASSERT(meshes.size() > 0);
    Ofstream            ofs(filename);
    ofs.precision(7);
    ofs <<
        "#VRML V2.0 utf8\n"
        "# Copyright 2015 Singular Inversions Inc. (facegen.com)\n"
        "# For more information, please visit https://facegen.com\n";
    Path                fpath(filename);
    for (size_t ii=0; ii<meshes.size(); ++ii) {
        Mesh const &        mesh = meshes[ii];
        String8             nameUtf;
        if (mesh.name.empty())
            nameUtf = fpath.base + toStr(ii);
        else
            nameUtf = mesh.name;
        // Some VRML parsers (Meshlab) require DEF variable names to be strictly alphanumeric starting
        // with a letter:
        string              name = fgToVariableName(nameUtf);
        ofs << "\n"
            "DEF " << name << " Shape\n"
            "{\n"
            "    appearance Appearance\n"
            "    {\n"
            "        material Material\n"
            "        {\n"
            "            ambientIntensity    1.0\n"
            "            diffuseColor        0.8 0.8 0.8\n"
            "            specularColor       0 0 0\n"
            "        }\n";
        if (mesh.numValidAlbedoMaps() > 0) {
            if (mesh.numValidAlbedoMaps() > 1)
                fgThrow("VRML export with multiple texture images not yet implemented");
            // Some software (Meshlab:) can't deal with spaces in the image filename:
            String8         imgFile = fpath.base.replace(' ','_') + toStr(ii);
            imgFile += "." + imgFormat;
            saveImage(*mesh.surfaces[0].material.albedoMap,fpath.dir()+imgFile);
            ofs <<
                "        texture ImageTexture\n"
                "        {\n"
                "            url \"" << imgFile << "\"\n"
                "        }\n";
        }
        ofs <<
            "    }\n";
        ofs <<
            "    geometry IndexedFaceSet\n"
            "    {\n"
            "        creaseAngle 1\n"
            "        coord Coordinate\n"
            "        {\n";
        writePoints(ofs,mesh.verts);
        ofs << 
            "        }\n"
            "        coordIndex\n"
            "        [\n";
        for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
            Surf const & surf = mesh.surfaces[ss];
            if (surf.tris.size() > 0) {
                writeIndices(ofs,surf.tris.vertInds);
                if (surf.quads.size() > 0)
                    ofs << ",\n";
                else
                    ofs << "\n";
            }
            if (surf.quads.size() > 0) {
                writeIndices(ofs,surf.quads.vertInds);
                ofs << "\n";
            }
        }
        ofs <<
            "        ]\n";
        if (mesh.uvs.size() > 0) {
            ofs <<
                "        texCoord TextureCoordinate\n"
                "        {\n";
            writePoints(ofs,mesh.uvs);
            ofs <<
                "        }\n"
                "        texCoordIndex\n"
                "        [\n";
            for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
                Surf const & surf = mesh.surfaces[ss];
                if (surf.tris.uvInds.size() > 0) {
                    writeIndices(ofs,surf.tris.uvInds);
                    if (surf.quads.uvInds.size() > 0)
                        ofs << ",\n";
                    else
                        ofs << "\n";
                }
                if (surf.quads.uvInds.size() > 0) {
                    writeIndices(ofs,surf.quads.uvInds);
                    ofs << "\n";
                }
            }
            ofs <<
                "        ]\n";
        }
        ofs <<
            "    }\n"
            "}\n";
    }
}

void
testSaveVrml(CLArgs const & args)
{
    FGTESTDIR
    String8         dd = dataDir();
    string          rd = "base/";
    Mesh            mouth = loadTri(dd+rd+"Mouth.tri");
    mouth.surfaces[0].setAlbedoMap(loadImage(dd+rd+"MouthSmall.png"));
    Mesh            glasses = loadTri(dd+rd+"Glasses.tri");
    glasses.surfaces[0].setAlbedoMap(loadImage(dd+rd+"Glasses.tga"));
    saveVrml("meshExportVrml.wrl",{mouth,glasses});
    regressFileRel("meshExportVrml.wrl","base/test/");
    regressFileRel("meshExportVrml0.png","base/test/");
    regressFileRel("meshExportVrml1.png","base/test/");
}

}

// */
