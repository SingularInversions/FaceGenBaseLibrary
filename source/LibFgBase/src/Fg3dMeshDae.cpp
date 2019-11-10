//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"
#include "FgStdStream.hpp"
#include "FgStdMap.hpp"
#include "FgImage.hpp"
#include "FgFileSystem.hpp"
#include "Fg3dMeshIo.hpp"
#include "Fg3dMeshOps.hpp"
#include "FgParse.hpp"
#include "Fg3dNormals.hpp"
#include "FgTestUtils.hpp"

using namespace std;

namespace Fg {

void
saveDae(
    const Ustring &            filename,
    const vector<Mesh> &    meshes,
    string                      imgFormat)
{
    Path          fpath(filename);
    Ustring        dirBase = fpath.dirBase();
    Ofstream      ofs(dirBase+".dae");
    ofs.precision(7);
    ofs <<
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<COLLADA xmlns=\"http://www.collada.org/2005/11/COLLADASchema\" version=\"1.4.1\">\n"
        "  <asset>\n"
        "    <contributor>\n"
        "      <authoring_tool>FaceGen</authoring_tool>\n"
        "    </contributor>\n"
        "    <up_axis>Y_UP</up_axis>\n"
        "  </asset>\n";
    ostringstream       lib_img,
                        lib_shd,
                        lib_mat;
    map<ImgC4UC*,Ustring>  imagesSaved;
    for (size_t mm=0; mm<meshes.size(); ++mm) {
        const Mesh &        mesh = meshes[mm];
        for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
            const Surf &     surf = mesh.surfaces[ss];
            if (surf.hasUvIndices() && surf.material.albedoMap) {
                string              id = toString(mm) + "_" + toString(ss);
                Ustring            imgFile = fpath.base + id + "." + imgFormat;
                ImgC4UC         *imgPtr = surf.material.albedoMap.get();
                if (fgContains(imagesSaved,imgPtr))
                    imgFile = imagesSaved[imgPtr];
                else {
                    imgSaveAnyFormat(fpath.dir()+imgFile,*surf.material.albedoMap);
                    imagesSaved[imgPtr] = imgFile;
                }
                lib_img <<
                    "    <image id=\"image" << id << "\">\n"
                    "      <init_from>" << imgFile << "</init_from>\n"
                    "    </image>\n";
                lib_shd <<
                    "    <effect id=\"shader" << id << "\" name=\"shader" << id << "\">\n"
                    "      <profile_COMMON>\n"
                    "        <newparam sid=\"surface" << id << "\">\n"
                    "          <surface type=\"2D\">\n"
                    "            <init_from>image" << id << "</init_from>\n"
                    "          </surface>\n"
                    "        </newparam>\n"
                    "        <newparam sid=\"sampler" << id << "\">\n"
                    "          <sampler2D>\n"
                    "            <source>surface" << id << "</source>\n"
                    "          </sampler2D>\n"
                    "        </newparam>\n"
                    "        <technique sid=\"standard\">\n"
                    "          <phong>\n"
                    "            <ambient>\n"
                    "              <texture texture=\"sampler" << id << "\" texcoord=\"CHANNEL0\" />\n"
                    "            </ambient>\n"
                    "            <diffuse>\n"
                    "              <texture texture=\"sampler" << id << "\" texcoord=\"CHANNEL0\" />\n"
                    "            </diffuse>\n"
                    "          </phong>\n"
                    "        </technique>\n"
                    "      </profile_COMMON>\n"
                    "    </effect>\n";
                lib_mat <<
                    "    <material id=\"material" << id << "\" name=\"material" << id << "\">\n"
                    "      <instance_effect url=\"#shader" << id << "\" />\n"
                    "    </material>\n";
            }
        }
    }
    ofs <<
        "  <library_images>\n" << lib_img.str() << "  </library_images>\n"
        "  <library_effects>\n" << lib_shd.str() << "  </library_effects>\n"
        "  <library_materials>\n" << lib_mat.str() << "  </library_materials>\n";
    ofs <<
        "  <library_geometries>\n";
    for (size_t mm=0; mm<meshes.size(); ++mm) {
        const Mesh &    mesh = meshes[mm];
        Normals         norms = cNormals(mesh);
        string              id = "mesh" + toString(mm);
        Ustring            name = mesh.name.empty() ? id : mesh.name;
        ofs <<
            "    <geometry id=\"" << id << "\" name=\"" << name << "\">\n"
            "      <mesh>\n"
            "        <source id=\"" << id << "-positions\" name=\"positions\">\n"
            "          <float_array id=\"" << id << "-positions-array\" count=\"" << mesh.verts.size()*3 << "\">";
        for (const Vec3F & v : mesh.verts)
            ofs << " " << v[0] << " " << v[1] << " " << v[2];
        ofs << " </float_array>\n"
            "          <technique_common>\n"
            "            <accessor count=\"" << mesh.verts.size() << "\" source=\"#" << id << "-positions-array\" stride=\"3\">\n"
            "              <param name=\"X\" type=\"float\" />\n"
            "              <param name=\"Y\" type=\"float\" />\n"
            "              <param name=\"Z\" type=\"float\" />\n"
            "            </accessor>\n"
            "          </technique_common>\n"
            "        </source>\n"
            "        <source id=\"" << id << "-normals\" name=\"normals\">\n"
            "          <float_array id=\"" << id << "-normals-array\" count=\"" << norms.vert.size()*3 << "\">";
        for (const Vec3F & v : norms.vert)
            ofs << " " << v[0] << " " << v[1] << " " << v[2];
        ofs << " </float_array>\n"
            "          <technique_common>\n"
            "            <accessor count=\"" << norms.vert.size() << "\" source=\"#" << id << "-normals-array\" stride=\"3\">\n"
            "              <param name=\"X\" type=\"float\" />\n"
            "              <param name=\"Y\" type=\"float\" />\n"
            "              <param name=\"Z\" type=\"float\" />\n"
            "            </accessor>\n"
            "          </technique_common>\n"
            "        </source>\n";
        if (!mesh.uvs.empty()) {
            ofs <<
                "        <source id=\"" << id << "-uvs\" name=\"uvs\">\n"
                "          <float_array id=\"" << id << "-uvs-array\" count=\"" << mesh.uvs.size()*2 << "\">";
            for (const Vec2F & v : mesh.uvs)
                ofs << " " << v[0] << " " << v[1];
        ofs << " </float_array>\n"
                "          <technique_common>\n"
                "            <accessor count=\"" << mesh.uvs.size() << "\" source=\"#" << id << "-uvs-array\" stride=\"2\">\n"
                "              <param name=\"S\" type=\"float\" />\n"
                "              <param name=\"T\" type=\"float\" />\n"
                "            </accessor>\n"
                "          </technique_common>\n"
                "        </source>\n";
        }
        ofs <<
            "        <vertices id=\"" << id << "-vertices\">\n"
            "          <input semantic=\"POSITION\" source=\"#" << id << "-positions\" />\n"
            "        </vertices>\n";
        for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
            const Surf &     surf = mesh.surfaces[ss];
            bool        hasUVs = (surf.tris.hasUvs() && surf.quads.hasUvs());
            ofs <<
                "        <polylist count=\"" << surf.tris.size() + surf.quads.size() << "\" material=\"materialRef" << mm << "_" << ss << "\">\n"
                "          <input offset=\"0\" semantic=\"VERTEX\" source=\"#" << id << "-vertices\" />\n"
                "          <input offset=\"0\" semantic=\"NORMAL\" source=\"#" << id << "-normals\" />\n";
            if (hasUVs)
                ofs <<
                "          <input offset=\"1\" semantic=\"TEXCOORD\" source=\"#" << id << "-uvs\" set=\"0\" />\n";
            ofs <<
                "          <vcount>";
            for (size_t ii=0; ii<surf.tris.size(); ++ii)
                ofs << " 3";
            for (size_t ii=0; ii<surf.quads.size(); ++ii)
                ofs << " 4";
            ofs << " </vcount>\n"
                "          <p>";
            for (size_t ii=0; ii<surf.tris.vertInds.size(); ++ii) {
                Vec3UI       tri = surf.tris.vertInds[ii];
                for (size_t jj=0; jj<3; ++jj) {
                    ofs << " " << tri[jj];
                    if (hasUVs)
                        ofs << " " << surf.tris.uvInds[ii][jj];
                }
            }
            for (size_t ii=0; ii<surf.quads.vertInds.size(); ++ii) {
                Vec4UI       quad = surf.quads.vertInds[ii];
                for (size_t jj=0; jj<4; ++jj) {
                    ofs << " " << quad[jj];
                    if (hasUVs)
                        ofs << " " << surf.quads.uvInds[ii][jj];
                }
            }
            ofs << " </p>\n"
                "        </polylist>\n";
        }
        ofs <<
            "      </mesh>\n"
            "    </geometry>\n";
    }
    ofs <<
        "  </library_geometries>\n"
        "  <library_visual_scenes>\n"
        "    <visual_scene id=\"RootNode\" name=\"RootNode\">\n";
    for (size_t mm=0; mm<meshes.size(); ++mm) {
        ofs <<
            "      <node id=\"node" << mm << "\" name=\"node" << mm << "\">\n"
            "        <instance_geometry url=\"#mesh" << mm << "\">\n";
        for (size_t ss=0; ss<meshes[mm].surfaces.size(); ++ss) {
            ofs <<
                "          <bind_material>\n"
                "            <technique_common>\n"
                "              <instance_material symbol=\"materialRef" << mm << "_" << ss << "\" target=\"#material" << mm << "_" << ss << "\">\n"
                "                <bind_vertex_input semantic=\"CHANNEL0\" input_semantic=\"TEXCOORD\" input_set=\"0\" />\n"
                "              </instance_material>\n"
                "            </technique_common>\n"
                "          </bind_material>\n";
        }
        ofs <<
            "        </instance_geometry>\n"
            "      </node>\n";
    }
    ofs <<
        "    </visual_scene>\n"
        "  </library_visual_scenes>\n"
        "  <scene>\n"
        "    <instance_visual_scene url=\"#RootNode\" />\n"
        "  </scene>\n"
        "</COLLADA>\n";
}

void
fgSaveDaeTest(const CLArgs & args)
{
    FGTESTDIR
    Ustring        dd = dataDir() + "base/test/";
    Mesh        teethU = loadTri(dd+"teethUpper.tri"),
                    teethL = loadTri(dd+"teethLower.tri");
    auto            teethImg = make_shared<ImgC4UC>(imgLoadAnyFormat(dd+"teeth.png"));
    teethU.surfaces[0].material.albedoMap = teethImg;
    teethL.surfaces[0].material.albedoMap = teethImg;
    saveDae("teethExportDae",fgSvec(teethU,teethL));
    regressFileRel("teethExportDae.dae","base/test/");
    regressFileRel("teethExportDae0_0.png","base/test/");
}

}

// */
