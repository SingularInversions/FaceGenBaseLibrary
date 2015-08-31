//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: Jan 17, 2014
//
// Very basic FBX export

#include "stdafx.h"
#include "FgStdStream.hpp"
#include "FgImage.hpp"
#include "FgFileSystem.hpp"
#include "Fg3dMeshOps.hpp"
#include "Fg3dMeshIo.hpp"
#include "Fg3dNormals.hpp"

using namespace std;

struct  Sects
{
    string      objects;
    string      connections;
};

Sects
getSects(const Fg3dMesh & mesh,size_t & objCnt)
{
    std::ostringstream      o,c;

    // MODEL:
    size_t  id_model = ++objCnt,
            id_material = ++objCnt;
    o <<
        "    Model: " << id_model << ", \"Model::" << mesh.name.as_ascii() << "\", \"Mesh\" {\n"
        "        Version: 232\n"
        "        Properties70:  {\n"
        "            P: \"ScalingMax\", \"Vector3D\", \"Vector\", \"\",0,0,0\n"
        "            P: \"DefaultAttributeIndex\", \"int\", \"Integer\", \"\",0\n"
        "            P: \"Lcl Translation\", \"Lcl Translation\", \"\", \"A\",0,0,0\n"
        "        }\n"
        "        Shading: T\n"
        "        Culling: \"CullingOff\"\n"
        "    }\n"
        "    Material: " << id_material << ", \"Material::Skin\", \"\" {\n"
        "        Version: 102\n"
        "        ShadingModel: \"phong\"\n"
        "        MultiLayer: 0\n"
        "        Properties70:  {\n"
        "            P: \"ShadingModel\", \"KString\", \"\", \"\", \"phong\"\n"
        "            P: \"AmbientColor\", \"Color\", \"\", \"A\",1,1,1\n"
        "            P: \"DiffuseColor\", \"Color\", \"\", \"A\",1,1,1\n"
        "            P: \"SpecularColor\", \"Color\", \"\", \"A\",0,0,0\n"
        "            P: \"ShininessExponent\", \"Number\", \"\", \"A\",0.5\n"
        "            P: \"Emissive\", \"Vector3D\", \"Vector\", \"\",0,0,0\n"
        "            P: \"Ambient\", \"Vector3D\", \"Vector\", \"\",1,1,1\n"
        "            P: \"Diffuse\", \"Vector3D\", \"Vector\", \"\",1,1,1\n"
        "            P: \"Specular\", \"Vector3D\", \"Vector\", \"\",0,0,0\n"
        "            P: \"Shininess\", \"double\", \"Number\", \"\",0.5\n"
        "            P: \"Opacity\", \"double\", \"Number\", \"\",1\n"
        "            P: \"Reflectivity\", \"double\", \"Number\", \"\",0\n"
        "        }\n"
        "    }\n";
    c <<
        "    C: \"OO\"," << id_model << ",0\n"
        "    C: \"OO\"," << id_material << "," << id_model << "\n";

    // BASE GEOMETRY
    size_t  id_geometry_base = ++objCnt;
    o <<
        "    Geometry: " << id_geometry_base << ", \"Geometry::" << mesh.name.as_ascii() << "\" , \"Mesh\" {\n"
        "        Vertices: *" << mesh.verts.size() * 3 << " {\n"
        "            a: ";
    for (size_t ii=0; ii<mesh.verts.size(); ++ii) {
        FgVect3F        v = mesh.verts[ii];
        o << v[0] << "," << v[1] << "," << v[2];
        if (ii != mesh.verts.size() - 1)
            o << ",";
    }
    if (mesh.surfaces.size() > 1)
        fgThrow("FBX export for multi-surface meshes not yet implemented");
    const Fg3dSurface &     surf = mesh.surfaces[0];
    size_t                  numInds = surf.tris.vertInds.size() * 3 + surf.quads.vertInds.size() * 4;
    o << "\n"
        "        }\n"
        "        PolygonVertexIndex: *" << numInds << " {\n"
        "            a: ";
    for (size_t ii=0; ii<surf.tris.vertInds.size(); ++ii) {
        FgVect3UI       i = surf.tris.vertInds[ii];
        o << i[0] << "," << i[1] << "," << -1 - int(i[2]);
        if ((ii != surf.tris.vertInds.size()-1) || !surf.quads.vertInds.empty())
            o << ",";
    }
    for (size_t ii=0; ii<surf.quads.vertInds.size(); ++ii) {
        FgVect4UI       i = surf.quads.vertInds[ii];
        o << i[0] << "," << i[1] << "," << i[2] << "," << -1 - int(i[3]);
        if (ii != surf.quads.vertInds.size()-1)
            o << ",";
    }
    Fg3dNormals     norms = fgNormals(mesh.surfaces,mesh.verts);
    o << "\n"
        "        }\n"
        "        GeometryVersion: 124\n"
        "        LayerElementNormal: 0 {\n"
        "            Version: 102\n"
        "            Name: \"\"\n"
        "            MappingInformationType: \"ByVertice\"\n"
        "            ReferenceInformationType: \"Direct\"\n"
        "            Normals: *" << norms.vert.size() * 3 << " {\n"
        "                a: ";
    for (size_t ii=0; ii<norms.vert.size(); ++ii) {
        FgVect3F    n = norms.vert[ii];
        o << n[0] << "," << n[1] << "," << n[2];
        if (ii != norms.vert.size()-1)
            o << ",";
    }
    o << "\n"
        "            } \n"
        "        }\n";
    if (!mesh.uvs.empty()) {
        o <<
            "        LayerElementUV: 0 {\n"
            "            Version: 101\n"
            "            Name: \"UVs\"\n"
            "            MappingInformationType: \"ByPolygonVertex\"\n"
            "            ReferenceInformationType: \"IndexToDirect\"\n"
            "            UV: *" << mesh.uvs.size() * 2 << " {\n"
            "                a: ";
        for (size_t ii=0; ii<mesh.uvs.size(); ++ii) {
            FgVect2F    uv = mesh.uvs[ii];
            o << uv[0] << "," << uv[1];
            if (ii != mesh.uvs.size()-1)
                o << ",";
        }
        numInds = surf.tris.uvInds.size() * 3 + surf.quads.uvInds.size() * 4;
        o << "\n"
            "            }\n"
            "            UVIndex: *" << numInds << " {\n"
            "                a: ";
        for (size_t ii=0; ii<surf.tris.uvInds.size(); ++ii) {
            FgVect3UI       i = surf.tris.uvInds[ii];
            o << i[0] << "," << i[1] << "," << i[2];
            if ((ii != surf.tris.uvInds.size()-1) || !surf.quads.uvInds.empty())
                o << ",";
        }
        for (size_t ii=0; ii<surf.quads.uvInds.size(); ++ii) {
            FgVect4UI       i = surf.quads.uvInds[ii];
            o << i[0] << "," << i[1] << "," << i[2] << "," << i[3];
            if (ii != surf.quads.uvInds.size()-1)
                o << ",";
        }
        o << "\n"
            "            }\n"
            "        }\n";
    }
    o <<
        "        LayerElementMaterial: 0 {\n"
        "            Version: 101\n"
        "            Name: \"\"\n"
        "            MappingInformationType: \"AllSame\"\n"
        "        }\n"
        "        Layer: 0 {\n"
        "            Version: 100\n"
        "            LayerElement:  {\n"
        "                Type: \"LayerElementNormal\"\n"
        "                TypedIndex: 0\n"
        "            }\n"
        "            LayerElement:  {\n"
        "                Type: \"LayerElementMaterial\"\n"
        "                TypedIndex: 0\n"
        "            }\n"
        "            LayerElement:  {\n"
        "                Type: \"LayerElementUV\"\n"
        "                TypedIndex: 0\n"
        "            }\n"
        "        }\n"
        "    }\n";
    c << 
        "    C: \"OO\"," << id_geometry_base << "," << id_model << "\n";

    // MORPHS:
    vector<float>       morphCoord(mesh.numMorphs(),0.0f);
    for (size_t mm=0; mm<mesh.numMorphs(); ++mm) {
        size_t      id_morph_geometry = ++objCnt,
                    id_morph_deformer = ++objCnt,
                    id_morph_channel = ++objCnt;
        FgIndexedMorph  m = mesh.getMorphAsIndexedDelta(mm);
        o <<
            "    Geometry: " << id_morph_geometry << ",  \"Geometry::" << m.name << "\", \"Shape\" {\n"
            "        Version: 100\n"
            "        Indexes: *" << m.baseInds.size() << " {\n"
            "            a: ";
        for (size_t ii=0; ii<m.baseInds.size(); ++ii) {
            o << m.baseInds[ii];
            if (ii != m.baseInds.size()-1)
                o << ",";
        }
        o << "\n"
            "        }\n"
            "        Vertices: *" << m.baseInds.size() * 3 << " {\n"
            "            a: ";
        for (size_t ii=0; ii<m.verts.size(); ++ii) {
            FgVect3F    v = m.verts[ii];
            o << v[0] << "," << v[1] << "," << v[2];
            if (ii != m.verts.size()-1)
                o << ",";
        }
        morphCoord[mm] = 1.0f;
        FgVerts         morphVerts;
        mesh.morph(morphCoord,morphVerts);
        Fg3dNormals     mn = fgNormals(mesh.surfaces,morphVerts);
        morphCoord[mm] = 0.0f;
        o << "\n"
            "        }\n"
            "        Normals: *" << m.baseInds.size() * 3 << " {\n"
            "            a: ";
        for (size_t ii=0; ii<m.baseInds.size(); ++ii) {
            FgVect3F    n = mn.vert[m.baseInds[ii]];
            o << n[0] << "," << n[1] << "," << n[2];
            if (ii != m.baseInds.size()-1)
                o << ",";
        }
        o << "\n"
            "        }\n"
            "    }\n"
            "    Deformer: " << id_morph_deformer << ", \"Deformer::" << m.name << "\", \"BlendShape\" {\n"
            "        Version: 100\n"
            "    }\n"
            "    Deformer: " << id_morph_channel << ", \"SubDeformer::" << m.name << "\", \"BlendShapeChannel\" {\n"
            "        Version: 100\n"
            "        DeformPercent: 0\n"
            "        FullWeights: *1 {\n"
            "            a: 10\n"
            "        }\n"
            "    }\n";
        c <<
            "    C: \"OO\"," << id_morph_deformer << "," << id_geometry_base << "\n"
            "    C: \"OO\"," << id_morph_channel << "," << id_morph_deformer << "\n"
            "    C: \"OO\"," << id_morph_geometry << "," << id_morph_channel << "\n";
    }

    Sects   ret;
    ret.objects = o.str();
    ret.connections = c.str();
    return ret;
}

void
fgSaveFbx(
    const FgString &            filename,
    const vector<Fg3dMesh> &    meshes)
{
    FGASSERT(!meshes.empty());
    FgOfstream  ofs(filename);
    ofs.precision(7);
    ofs <<
        "; FBX 7.4.0 project file\n"
        "FBXHeaderExtension:  {\n"
        "    FBXHeaderVersion: 1003\n"
        "    FBXVersion: 7400\n"
        "    Creator: \"FaceGen\"\n"
        "}\n"
        "Definitions:  {\n"
        "    Version: 100\n"
        "    Count: 7\n"
        "    ObjectType: \"GlobalSettings\" {\n"
        "        Count: 1\n"
        "    }\n"
        "    ObjectType: \"Geometry\" {\n"
        "        Count: 2\n"
        "        PropertyTemplate: \"FbxMesh\" {\n"
        "            Properties70:  {\n"
        "                P: \"Color\", \"ColorRGB\", \"Color\", \"\",0.8,0.8,0.8\n"
        "                P: \"BBoxMin\", \"Vector3D\", \"Vector\", \"\",0,0,0\n"
        "                P: \"BBoxMax\", \"Vector3D\", \"Vector\", \"\",0,0,0\n"
        "                P: \"Primary Visibility\", \"bool\", \"\", \"\",1\n"
        "                P: \"Casts Shadows\", \"bool\", \"\", \"\",1\n"
        "                P: \"Receive Shadows\", \"bool\", \"\", \"\",1\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "    ObjectType: \"Model\" {\n"
        "        Count: 1\n"
        "        PropertyTemplate: \"FbxNode\" {\n"
        "        }\n"
        "    }\n"
        "    ObjectType: \"Material\" {\n"
        "        Count: 1\n"
        "        PropertyTemplate: \"FbxSurfacePhong\" {\n"
        "            Properties70:  {\n"
        "                P: \"ShadingModel\", \"KString\", \"\", \"\", \"Phong\"\n"
        "                P: \"MultiLayer\", \"bool\", \"\", \"\",0\n"
        "                P: \"EmissiveColor\", \"Color\", \"\", \"A\",0,0,0\n"
        "                P: \"EmissiveFactor\", \"Number\", \"\", \"A\",1\n"
        "                P: \"AmbientColor\", \"Color\", \"\", \"A\",0.2,0.2,0.2\n"
        "                P: \"AmbientFactor\", \"Number\", \"\", \"A\",1\n"
        "                P: \"DiffuseColor\", \"Color\", \"\", \"A\",0.8,0.8,0.8\n"
        "                P: \"DiffuseFactor\", \"Number\", \"\", \"A\",1\n"
        "                P: \"Bump\", \"Vector3D\", \"Vector\", \"\",0,0,0\n"
        "                P: \"NormalMap\", \"Vector3D\", \"Vector\", \"\",0,0,0\n"
        "                P: \"BumpFactor\", \"double\", \"Number\", \"\",1\n"
        "                P: \"TransparentColor\", \"Color\", \"\", \"A\",0,0,0\n"
        "                P: \"TransparencyFactor\", \"Number\", \"\", \"A\",0\n"
        "                P: \"DisplacementColor\", \"ColorRGB\", \"Color\", \"\",0,0,0\n"
        "                P: \"DisplacementFactor\", \"double\", \"Number\", \"\",1\n"
        "                P: \"VectorDisplacementColor\", \"ColorRGB\", \"Color\", \"\",0,0,0\n"
        "                P: \"VectorDisplacementFactor\", \"double\", \"Number\", \"\",1\n"
        "                P: \"SpecularColor\", \"Color\", \"\", \"A\",0.2,0.2,0.2\n"
        "                P: \"SpecularFactor\", \"Number\", \"\", \"A\",1\n"
        "                P: \"ShininessExponent\", \"Number\", \"\", \"A\",20\n"
        "                P: \"ReflectionColor\", \"Color\", \"\", \"A\",0,0,0\n"
        "                P: \"ReflectionFactor\", \"Number\", \"\", \"A\",1\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "    ObjectType: \"Texture\" {\n"
        "        Count: 3\n"
        "        PropertyTemplate: \"FbxFileTexture\" {\n"
        "            Properties70:  {\n"
        "                P: \"TextureTypeUse\", \"enum\", \"\", \"\",0\n"
        "                P: \"Texture alpha\", \"Number\", \"\", \"A\",1\n"
        "                P: \"CurrentMappingType\", \"enum\", \"\", \"\",0\n"
        "                P: \"WrapModeU\", \"enum\", \"\", \"\",0\n"
        "                P: \"WrapModeV\", \"enum\", \"\", \"\",0\n"
        "                P: \"UVSwap\", \"bool\", \"\", \"\",0\n"
        "                P: \"PremultiplyAlpha\", \"bool\", \"\", \"\",1\n"
        "                P: \"Translation\", \"Vector\", \"\", \"A\",0,0,0\n"
        "                P: \"Rotation\", \"Vector\", \"\", \"A\",0,0,0\n"
        "                P: \"Scaling\", \"Vector\", \"\", \"A\",1,1,1\n"
        "                P: \"TextureRotationPivot\", \"Vector3D\", \"Vector\", \"\",0,0,0\n"
        "                P: \"TextureScalingPivot\", \"Vector3D\", \"Vector\", \"\",0,0,0\n"
        "                P: \"CurrentTextureBlendMode\", \"enum\", \"\", \"\",1\n"
        "                P: \"UVSet\", \"KString\", \"\", \"\", \"default\"\n"
        "                P: \"UseMaterial\", \"bool\", \"\", \"\",0\n"
        "                P: \"UseMipMap\", \"bool\", \"\", \"\",0\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "    ObjectType: \"LayeredTexture\" {\n"
        "        Count: 2\n"
        "        PropertyTemplate: \"FbxLayeredTexture\" {\n"
        "            Properties70:  {\n"
        "                P: \"TextureTypeUse\", \"enum\", \"\", \"\",0\n"
        "                P: \"Texture alpha\", \"Number\", \"\", \"A\",1\n"
        "                P: \"CurrentMappingType\", \"enum\", \"\", \"\",0\n"
        "                P: \"WrapModeU\", \"enum\", \"\", \"\",0\n"
        "                P: \"WrapModeV\", \"enum\", \"\", \"\",0\n"
        "                P: \"UVSwap\", \"bool\", \"\", \"\",0\n"
        "                P: \"PremultiplyAlpha\", \"bool\", \"\", \"\",1\n"
        "                P: \"Translation\", \"Vector\", \"\", \"A\",0,0,0\n"
        "                P: \"Rotation\", \"Vector\", \"\", \"A\",0,0,0\n"
        "                P: \"Scaling\", \"Vector\", \"\", \"A\",1,1,1\n"
        "                P: \"TextureRotationPivot\", \"Vector3D\", \"Vector\", \"\",0,0,0\n"
        "                P: \"TextureScalingPivot\", \"Vector3D\", \"Vector\", \"\",0,0,0\n"
        "                P: \"CurrentTextureBlendMode\", \"enum\", \"\", \"\",1\n"
        "                P: \"UVSet\", \"KString\", \"\", \"\", \"default\"\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "    ObjectType: \"Video\" {\n"
        "        Count: 3\n"
        "        PropertyTemplate: \"FbxVideo\" {\n"
        "            Properties70:  {\n"
        "                P: \"ImageSequence\", \"bool\", \"\", \"\",0\n"
        "                P: \"ImageSequenceOffset\", \"int\", \"Integer\", \"\",0\n"
        "                P: \"FrameRate\", \"double\", \"Number\", \"\",0\n"
        "                P: \"LastFrame\", \"int\", \"Integer\", \"\",0\n"
        "                P: \"Width\", \"int\", \"Integer\", \"\",0\n"
        "                P: \"Height\", \"int\", \"Integer\", \"\",0\n"
        "                P: \"Path\", \"KString\", \"XRefUrl\", \"\", \"\"\n"
        "                P: \"StartFrame\", \"int\", \"Integer\", \"\",0\n"
        "                P: \"StopFrame\", \"int\", \"Integer\", \"\",0\n"
        "                P: \"PlaySpeed\", \"double\", \"Number\", \"\",0\n"
        "                P: \"Offset\", \"KTime\", \"Time\", \"\",0\n"
        "                P: \"InterlaceMode\", \"enum\", \"\", \"\",0\n"
        "                P: \"FreeRunning\", \"bool\", \"\", \"\",0\n"
        "                P: \"Loop\", \"bool\", \"\", \"\",0\n"
        "                P: \"AccessMode\", \"enum\", \"\", \"\",0\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "    ObjectType: \"Deformer\" {\n"
        "        Count: 2\n"
        "    }\n"
        "}\n"
        "Objects:  {\n";
    size_t          objCnt = 2740000;
    vector<Sects>   sects;
    for (size_t ii=0; ii<meshes.size(); ++ii) {
        sects.push_back(getSects(meshes[ii],objCnt));
        ofs << sects.back().objects;
    }
    ofs <<
        "}\n"
        "Connections:  {\n";
    for (size_t ii=0; ii<sects.size(); ++ii)
        ofs << sects[ii].connections;
    ofs <<
        "}\n"
        "Takes:  {\n"
        "    Current: \"\"\n"
        "}\n";
}

// */
