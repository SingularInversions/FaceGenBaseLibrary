//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// NOTES:
//  * Target Collada 1.4 (1.5 has nothing we need and is not as well supported)
//  * Used the 'skin_and_morph.dae' sample in github 'threejs' to get started
//  * Blender is able to read in this file with bones & morph
//  * Unity can read it with the bones but doesn't get the morph
//  * Blender is able to DAE round-trip teeth w/ map after deselecting 'triangulate' on export.
//  * Not a single example DAE in assimp tests contains the 'morph' keyword
//  * MeshLab triangulates on DAE export but can read in Blender's DAE with quads & color map.
//  * Only other morph DAE I could find is 'glTF-Sample-Models' 'AnimatedMorphCube' but neither app read this morph (and blender crashed)
//  * It seems Unity will not read morphs from DAE
//
//  * TODO: preserve shiny & transparent texture material properties.
//  * TODO: save only a single controller for shared morph names.
//  * TODO: save only a single instance of the topology for all morph targets.
//  * TODO: save only a single instance of shared albedo maps.
// 

#include "stdafx.h"
#include "FgStdStream.hpp"
#include "FgStdMap.hpp"
#include "FgImage.hpp"
#include "FgFileSystem.hpp"
#include "Fg3dMeshIo.hpp"
#include "Fg3dMesh.hpp"
#include "FgParse.hpp"
#include "FgTestUtils.hpp"

using namespace std;

namespace Fg {

namespace {

string
cEffect(string const & id)
{
    // <index_of_refraction> option is included to avoid Blender logging an invalid value for IOR
    // sid references cannot include spaces (<source>Name</source> NOT <source> Name </source>
    return R"(
    <effect id="Effect)" + id + R"(">
      <profile_COMMON>
        <technique sid="Technique">
          <newparam sid="Surface">
            <surface type="2D">
            <init_from>Image)" + id + R"(</init_from>
            </surface>
          </newparam>
          <newparam sid="Sampler">
            <sampler2D>
            <source>Surface</source>
            </sampler2D>
          </newparam>
          <phong>
            <ambient>
              <texture texture="Sampler" texcoord="CHANNEL0" />
            </ambient>
            <diffuse>
              <texture texture="Sampler" texcoord="CHANNEL0" />
            </diffuse>
            <index_of_refraction>
              <float> 1.0 </float>
            </index_of_refraction>
          </phong>
        </technique>
      </profile_COMMON>
    </effect>)";
}

string
cGeometryVerts(string const & id,Vec3Fs const & verts,Vec3Fs const & norms,Vec2Fs const & uvs)
{
    ostringstream       ofs;
    ofs.precision(7);
    ofs << "\n"
        "        <source id=\"" << id << "Coords\">\n"
        "          <float_array id=\"" << id << "CoordsArray\" count=\"" << verts.size()*3 << "\">";
    for (Vec3F const & v : verts)
        ofs << " " << v[0] << " " << v[1] << " " << v[2];
    ofs << " </float_array>\n"
        "          <technique_common>\n"
        "            <accessor count=\"" << verts.size() << "\" source=\"#" << id << "CoordsArray\" stride=\"3\">\n"
        "              <param name=\"X\" type=\"float\" />\n"
        "              <param name=\"Y\" type=\"float\" />\n"
        "              <param name=\"Z\" type=\"float\" />\n"
        "            </accessor>\n"
        "          </technique_common>\n"
        "        </source>\n"
        "        <source id=\"" << id << "Norms\">\n"
        "          <float_array id=\"" << id << "NormsArray\" count=\"" << norms.size()*3 << "\">";
    for (Vec3F const & v : norms)
        ofs << " " << v[0] << " " << v[1] << " " << v[2];
    ofs << " </float_array>\n"
        "          <technique_common>\n"
        "            <accessor count=\"" << norms.size() << "\" source=\"#" << id << "NormsArray\" stride=\"3\">\n"
        "              <param name=\"X\" type=\"float\" />\n"
        "              <param name=\"Y\" type=\"float\" />\n"
        "              <param name=\"Z\" type=\"float\" />\n"
        "            </accessor>\n"
        "          </technique_common>\n"
        "        </source>\n";
    if (!uvs.empty()) {
        ofs <<
            "        <source id=\"" << id << "Uvs\">\n"
            "          <float_array id=\"" << id << "UvsArray\" count=\"" << uvs.size()*2 << "\">";
        for (const Vec2F & v : uvs)
            ofs << " " << v[0] << " " << v[1];
        ofs << " </float_array>\n"
            "          <technique_common>\n"
            "            <accessor count=\"" << uvs.size() << "\" source=\"#" << id << "UvsArray\" stride=\"2\">\n"
            "              <param name=\"S\" type=\"float\" />\n"
            "              <param name=\"T\" type=\"float\" />\n"
            "            </accessor>\n"
            "          </technique_common>\n"
            "        </source>\n";
    }
    ofs <<
        "        <vertices id=\"" << id << "Verts\">\n"
        "          <input semantic=\"POSITION\" source=\"#" << id << "Coords\" />\n"
        "        </vertices>";
    return ofs.str();
}

string
cGeometrySurfs(Surfs const & surfs,string const & id,size_t mm)
{
    ostringstream       ofs;
    for (size_t ss=0; ss<surfs.size(); ++ss) {
        Surf const &     surf = surfs[ss];
        bool        hasUVs = (surf.tris.hasUvs() && surf.quads.hasUvs());
        ofs << "\n"
            "        <polylist count=\"" << surf.tris.size() + surf.quads.size() << "\" material=\"MaterialRef" << mm << "_" << ss << "\">\n"
            "          <input offset=\"0\" semantic=\"VERTEX\" source=\"#" << id << "Verts\" />\n"
            "          <input offset=\"1\" semantic=\"NORMAL\" source=\"#" << id << "Norms\" />\n";
        if (hasUVs)
            ofs <<
            "          <input offset=\"2\" semantic=\"TEXCOORD\" source=\"#" << id << "Uvs\" set=\"0\" />\n";
        ofs <<
            "          <vcount>";
        for (size_t ii=0; ii<surf.tris.size(); ++ii)
            ofs << " 3";
        for (size_t ii=0; ii<surf.quads.size(); ++ii)
            ofs << " 4";
        ofs << " </vcount>\n"
            "          <p>";
        for (size_t ii=0; ii<surf.tris.posInds.size(); ++ii) {
            Vec3UI       tri = surf.tris.posInds[ii];
            for (size_t jj=0; jj<3; ++jj) {
                ofs << " " << tri[jj];      // vertex index
                ofs << " " << tri[jj];      // normal index
                if (hasUVs)
                    ofs << " " << surf.tris.uvInds[ii][jj];
            }
        }
        for (size_t ii=0; ii<surf.quads.posInds.size(); ++ii) {
            Vec4UI       quad = surf.quads.posInds[ii];
            for (size_t jj=0; jj<4; ++jj) {
                ofs << " " << quad[jj];     // vertex index
                ofs << " " << quad[jj];     // normal index
                if (hasUVs)
                    ofs << " " << surf.quads.uvInds[ii][jj];
            }
        }
        ofs << " </p>\n"
            "        </polylist>";
    }
    return ofs.str();
}

string  cMeshId(size_t mm)  {return "Mesh" + toStr(mm); }
string  cMorphId(size_t mm,size_t rr) {return "Mesh" + toStr(mm) + "Morph" + toStr(rr); }

String8
cGeometry(Mesh const & mesh,size_t mm)
{
    string              id = cMeshId(mm);
    String8             name = mesh.name.empty() ? id : mesh.name;
    String8             ret = R"(
    <geometry id=")" + id + R"(" name=")" + name + R"(">
      <mesh>)" +
        cGeometryVerts(id,mesh.verts,cNormals(mesh).vert,mesh.uvs) +
        cGeometrySurfs(mesh.surfaces,id,mm) + R"(
      </mesh>
    </geometry>)";
    for (size_t rr=0; rr<mesh.numMorphs(); ++rr) {
        string              morphId = cMorphId(mm,rr);
        String8             morphName = mesh.morphName(rr);
        Vec3Fs              morphVerts = mesh.morphSingle(rr);
        ret += R"(
    <geometry id=")" + morphId + R"(" name=")" + morphName + R"(">
      <mesh>)" +
            cGeometryVerts(morphId,morphVerts,cNormals(mesh.surfaces,morphVerts).vert,mesh.uvs) +
            // Blender crashes without repeating surface definition for each morph.
            // Maya supposedly has the same problem (Collada docs).
            cGeometrySurfs(mesh.surfaces,morphId,mm) + R"(
      </mesh>
    </geometry>)";
    }
    return ret;
}

String8
cController(Mesh const & mesh,size_t mm)
{
    size_t              num = mesh.numMorphs();
    string              meshId = cMeshId(mm),
                        numStr = toStr(num);
    String8             ret = R"(
    <controller id=")" + meshId + R"(Controller" name="Morph Controls">
      <morph method="NORMALIZED" source="#)" + meshId + R"(">
        <source id=")" + meshId + R"(Targs">
          <IDREF_array id=")" + meshId + R"(TargsArray" count=")" + numStr + R"(">)";
    for (size_t rr=0; rr<num; ++rr)
        ret += cMorphId(mm,rr) + " ";
    ret += R"(</IDREF_array>
          <technique_common>
            <accessor source="#)" + meshId + R"(TargsArray" count=")" + numStr + R"(" stride="1">
              <param name="MORPH_TARGET" type="IDREF"/>
            </accessor>
          </technique_common>
        </source>
        <source id=")" + meshId + R"(Weights">
          <float_array id=")" + meshId + R"(WeightsArray" count=")" + numStr + R"(">)";
    for (size_t rr=0; rr<num; ++rr)
        ret += "0 ";
    ret += R"(</float_array>
          <technique_common>
            <accessor source="#)" + meshId + R"(WeightsArray" count=")" + numStr + R"(" stride="1">
              <param name="MORPH_WEIGHT" type="float"/>
            </accessor>
          </technique_common>
        </source>
        <targets>
          <input semantic="MORPH_TARGET" source="#)" + meshId + R"(Targs"/>
          <input semantic="MORPH_WEIGHT" source="#)" + meshId + R"(Weights"/>
        </targets>
      </morph>
    </controller>)";
    return ret;
}

string
cSceneNode(Mesh const & mesh,size_t mm)
{
    ostringstream       ofs;
    string              id = "Node" + toStr(mm);
    String8             name = mesh.name.empty() ? id : mesh.name;
    ofs << "\n"
        "      <node id=\"" << id << "\" name=\"" << name << "\">\n"
        "        <instance_geometry url=\"#Mesh" << mm << "\">\n";
    for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
        ofs <<
            "          <bind_material>\n"
            "            <technique_common>\n"
            "              <instance_material symbol=\"MaterialRef" << mm << "_" << ss << "\" target=\"#Material" << mm << "_" << ss << "\">\n"
            "                <bind_vertex_input semantic=\"CHANNEL0\" input_semantic=\"TEXCOORD\" input_set=\"0\" />\n"
            "              </instance_material>\n"
            "            </technique_common>\n"
            "          </bind_material>\n";
    }
    ofs <<
        "        </instance_geometry>\n"
        "      </node>";
    return ofs.str();
}

}

void
saveDae(String8 const & filename,Meshes const & meshes,string imgFormat,SpatialUnit unit)
{
    Path                fpath {filename};
    String8             dirBase = fpath.dirBase();
    String8             images,
                        effects,
                        materials,
                        geometries,
                        controllers,
                        sceneNodes;
    map<ImgRgba8*,String8>  imagesSaved;
    for (size_t mm=0; mm<meshes.size(); ++mm) {
        Mesh const &        mesh = meshes[mm];
        geometries += cGeometry(mesh,mm);
        controllers += cController(mesh,mm);
        sceneNodes += cSceneNode(mesh,mm);
        for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
            Surf const &        surf = mesh.surfaces[ss];
            if (surf.hasUvIndices() && surf.material.albedoMap) {
                string              id = toStr(mm) + "_" + toStr(ss);
                String8             imgFile = fpath.base + id + "." + imgFormat;
                ImgRgba8             *imgPtr = surf.material.albedoMap.get();
                if (contains(imagesSaved,imgPtr))
                    imgFile = imagesSaved[imgPtr];
                else {
                    saveImage(fpath.dir()+imgFile,*surf.material.albedoMap);
                    imagesSaved[imgPtr] = imgFile;
                }
                images += R"(
    <image id="Image)" + id + R"(">
      <init_from>)" + imgFile + R"(</init_from>
    </image>)";
                effects += cEffect(id);
                materials += R"(
    <material id="Material)" + id + R"(" name="Material)" + id + R"(">
      <instance_effect url="#Effect)" + id + R"(" />
    </material>)";
            }
        }
    }
    Ofstream            ofs {dirBase+".dae"};
    ofs << R"(<?xml version="1.0" encoding="UTF-8"?>
<COLLADA xmlns="http://www.collada.org/2005/11/COLLADASchema" version="1.4.1">
  <asset>
    <contributor>
      <authoring_tool>FaceGen</authoring_tool>
    </contributor>
    <unit name=")" + toStr(unit) + R"(" meter=")" + inMetresStr(unit) + R"("/>
    <up_axis>Y_UP</up_axis>
  </asset>
  <library_images>)" << images << R"(
  </library_images>
  <library_effects>)" << effects << R"(
  </library_effects>
  <library_materials>)" << materials << R"(
  </library_materials>
  <library_geometries>)" << geometries << R"(
  </library_geometries>
  <library_controllers>)" << controllers << R"(
  </library_controllers>
  <library_visual_scenes>
    <visual_scene id="Scene" name="Scene">)" << sceneNodes << R"(
    </visual_scene>
  </library_visual_scenes>
  <scene>
    <instance_visual_scene url="#Scene" />
  </scene>
</COLLADA>
)";
}

void
testSaveDae(CLArgs const & args)
{
    FGTESTDIR
    String8         dd = dataDir() + "base/";
    Mesh            face = loadTri(dd+"JaneLoresFace.tri"),
                    mouth = loadTri(dd+"MouthSmall.tri");
    face.surfaces[0].material.albedoMap = make_shared<ImgRgba8>(loadImage(dd+"JaneLoresFace.jpg"));
    mouth.surfaces[0].material.albedoMap = make_shared<ImgRgba8>(loadImage(dd+"MouthSmall.png"));
    saveDae("meshExportDae",{face,mouth});
    regressFileRel("meshExportDae.dae","base/test/");
    regressFileRel("meshExportDae0_0.png","base/test/");
    regressFileRel("meshExportDae1_0.png","base/test/");
}

}

// */
