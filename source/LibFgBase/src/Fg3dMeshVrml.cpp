//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "Fg3dMeshIo.hpp"
#include "FgStdStream.hpp"
#include "FgFileSystem.hpp"
#include "FgParse.hpp"

using namespace std;

namespace Fg {

template<uint dim>
void
writePoint(
    Ofstream &            ofs,
    Mat<float,dim,1>  pnt)
{
    ofs << "               ";
    for (uint kk=0; kk<dim; ++kk)
        ofs << " " << pnt[kk];
}

template<uint dim>
void
writePoints(
    Ofstream &                            ofs,
    const vector<Mat<float,dim,1> > & pts)
{
    ofs <<
        "            point\n"
        "            [\n";
    writePoint(ofs,pts[0]);
    for (size_t jj=1; jj<pts.size(); ++jj)
    {
        ofs << ",\n";
        writePoint(ofs,pts[jj]);
    }
    ofs << 
        "\n            ]\n";
}

template<uint dim>
void
writeIdx(
    Ofstream &          ofs,
    Mat<uint,dim,1> idx)
{
    ofs << "            ";
    for (uint ii=0; ii<dim; ++ii)
        ofs << idx[ii] << ", ";
    ofs << "-1";
}

template<uint dim>
void
writeIndices(
    Ofstream &                            ofs,
    const vector<Mat<uint,dim,1> > &  inds)
{
    if (inds.size() == 0) return;
    writeIdx(ofs,inds[0]);
    for (size_t ii=1; ii<inds.size(); ++ii)
    {
        ofs << ",\n";
        writeIdx(ofs,inds[ii]);
    }
}

void
saveVrml(
    Ustring const &            filename,
    const vector<Mesh> &    meshes,
    string                      imgFormat)
{
    FGASSERT(meshes.size() > 0);
    Ofstream  ofs(filename);
    ofs.precision(7);
    ofs <<
        "#VRML V2.0 utf8\n"
        "# Copyright 2015 Singular Inversions Inc. (facegen.com)\n"
        "# For more information, please visit https://facegen.com\n";
    Path          fpath(filename);
    for (size_t ii=0; ii<meshes.size(); ++ii) {
        const Mesh &    mesh = meshes[ii];
        Ustring            nameUtf;
        if (mesh.name.empty())
            nameUtf = fpath.base + toString(ii);
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
            Ustring    imgFile = fpath.base.replace(' ','_') + toString(ii);
            imgFile += "." + imgFormat;
            imgSaveAnyFormat(fpath.dir()+imgFile,*mesh.surfaces[0].material.albedoMap);
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
            const Surf & surf = mesh.surfaces[ss];
            if (surf.numTris() > 0) {
                writeIndices(ofs,surf.tris.vertInds);
                if (surf.numQuads() > 0)
                    ofs << ",\n";
                else
                    ofs << "\n";
            }
            if (surf.numQuads() > 0) {
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
                const Surf & surf = mesh.surfaces[ss];
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

}

// */
