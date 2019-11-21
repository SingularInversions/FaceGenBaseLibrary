//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"
#include "Fg3dMeshLegacy.hpp"
#include "FgStdStream.hpp"
#include "FgImage.hpp"
#include "FgFileSystem.hpp"
#include "Fg3dMeshOps.hpp"
#include "Fg3dMeshIo.hpp"
#include "Fg3dNormals.hpp"
#include "FgCommand.hpp"
#include "FgTestUtils.hpp"
#include "FgImageIo.hpp"

using namespace std;

namespace Fg {

//****************************************************************************
// Local function prototypes
//****************************************************************************
static bool saveXsiFile(
        Ustring const                    &fname,
        const FffMultiObjectC           &model,
        const vector<FffMultiObjectC>   *morphTargets,
        string const                    &appName);
static Vec3F calTriNormals(
        const vector<Vec3F>        &vtxList,
        const vector<Vec3UI>        &triList,
        unsigned long                   tri);
static Vec3F calQuadNormals(
        const vector<Vec3F>        &vtxList,
        const vector<Vec4UI>        &quadList,
        unsigned long                   quad);
static string floatToString(float val);


//****************************************************************************
//                              fffSaveXsiFile
//****************************************************************************
static bool    fffSaveXsiFile(

    Ustring const          &fname,
    const FffMultiObjectC   &model,
    string                  appName)
{
    return saveXsiFile(fname,model,0,appName);
}

static bool    fffSaveXsiFile(

    Ustring const                  &fname,
    const FffMultiObjectC           &model,
    const vector<FffMultiObjectC>   &morphTargets,
    string                          appName)
{
    return saveXsiFile(fname,model,&morphTargets,appName);
}


//****************************************************************************
//                              saveXsiFile
//****************************************************************************
static bool saveXsiFile(

    Ustring const                  &fname,
    const FffMultiObjectC           &model,
    const vector<FffMultiObjectC>   *morphTargets,
    string const                    &appName)
{
    size_t numTargets=0;
    if (morphTargets)
    {
        numTargets = morphTargets->size();
    }

    Path      path(fname);
    path.ext = "xsi";
    Ofstream ofs(path.str());
    if (!ofs)
    {
        return false;
    }

    //
    // Get object's bounding box (for camera and lighting info)
    //
    Vec3F centroid(0.0f,0.0f,0.0f);
    Vec3F lowBound(0.0f,0.0f,0.0f);
    Vec3F hiBound(0.0f,0.0f,0.0f);
    unsigned long numTotalVtx=0;
    for (unsigned long yy=0; yy<model.numObjs(); ++yy)
    {
        // Get aliase names for all the lists.
        const vector<Vec3F>    &vtxList = model.getPtList(yy);

        // Calculate the info
        for (unsigned long pt=0; pt<vtxList.size(); ++pt)
        {
            centroid += vtxList[pt];
            ++numTotalVtx;
            if (yy==0 && pt==0)
            {
                lowBound = vtxList[pt];
                hiBound = vtxList[pt];
            }
            else
            {
                if (lowBound[0] > vtxList[pt][0]) lowBound[0]=vtxList[pt][0];
                if (lowBound[1] > vtxList[pt][1]) lowBound[1]=vtxList[pt][1];
                if (lowBound[2] > vtxList[pt][2]) lowBound[2]=vtxList[pt][2];
                if (hiBound[0] < vtxList[pt][0]) hiBound[0]=vtxList[pt][0];
                if (hiBound[1] < vtxList[pt][1]) hiBound[1]=vtxList[pt][1];
                if (hiBound[2] < vtxList[pt][2]) hiBound[2]=vtxList[pt][2];
            }
        }
    }
    centroid /= float(numTotalVtx);
    Vec3F camPos = centroid;
    float zdiff = hiBound[2] - lowBound[2];
    camPos[2] += zdiff*2.0f;
    float camNearPlane = 0.1f;
    float camFarPlane = zdiff*5.0f;

    //
    // Write (version 3.0) file header
    //
    ofs << "xsi 0300txt 0032\n\n";

    //
    // Output some standard info
    //
    ofs << "\n";
    ofs << "SI_FileInfo\n";
    ofs << "{\n";
    ofs << "   \"\",\n";
    ofs << "   \"" << appName << "\",\n";
    ofs << "}\n";
    ofs << "\n";
    ofs << "SI_Scene scene1\n";
    ofs << "{ \n";
    ofs << "   \"FRAMES\",\n";
    ofs << "   1.000000,\n";
    if (numTargets)
        ofs << "   " << numTargets+1 << ".000000,\n";
    else
        ofs << "   100.000000,\n";
    ofs << "   30.0,\n";
    ofs << "}\n";
    ofs << "\n";
    ofs << "SI_CoordinateSystem coord1\n";
    ofs << "{\n";
    ofs << "   1;\n";
    ofs << "   0;\n";
    ofs << "   1;\n";
    ofs << "   0;\n";
    ofs << "   2;\n";
    ofs << "   5;\n";
    ofs << "}\n";
    ofs << "\n";
    ofs << "SI_Angle\n";
    ofs << "{\n";
    ofs << "   0;\n";
    ofs << "}\n";
    ofs << "\n";
    ofs << "SI_Camera camera1\n";
    ofs << "{\n";
                // Position
    ofs << "   " << floatToString(camPos[0]) << "; " 
                 << floatToString(camPos[1]) << "; " 
                 << floatToString(camPos[2]) << ";;\n";
                // Point of interest (camera direction)
    ofs << "   " << floatToString(centroid[0]) << "; "
                 << floatToString(centroid[1]) << "; "
                 << floatToString(centroid[2]) << ";;\n";
                // Roll
    ofs << "   0.0;\n";
                // Field of view
    ofs << "   55.0;\n";
                // Near plane units
    ofs << "   " << floatToString(camNearPlane) << ";\n";
                // Far plane units
    ofs << "   " << floatToString(camFarPlane) << ";\n";
    ofs << "}\n";
    ofs << "\n";
    ofs << "SI_Ambience\n";
    ofs << "{\n";
    ofs << "   0.2; 0.2; 0.2;;\n";
    ofs << "}\n";
    ofs << "\n";
    ofs << "SI_Light light1\n";
    ofs << "{\n";
                // Type
    ofs << "   0;\n";
                // Colour of light
    ofs << "   1.0; 1.0; 1.0;;\n";
                // Position of light (above the camera)
    ofs << "   " << floatToString(camPos[0]) << "; "
                 << floatToString(hiBound[1]) << "; "
                 << floatToString(camPos[2]) << ";;\n";
    ofs << "}\n";
    ofs << "\n";


    //
    // Now output the material list.
    //
    ofs << "SI_MaterialLibrary MATLIB-scene1\n";
    ofs << "{\n";
    ofs << "   " << model.numObjs() << ",\n";
    unsigned long xx;
    for (xx=0; xx<model.numObjs(); ++xx)
    {
        string objName = model.getModelName(xx);

        ofs << "\n";
        ofs << "   SI_Material " << objName << "\n";
        ofs << "   {\n";
        ofs << "      0.7, 0.7, 0.7, 1.0,\n";   // Diffuse colour
        ofs << "      50.0,\n";                 // Specular decay
        ofs << "      1.0, 1.0, 1.0,\n";        // Specular colour
        ofs << "      0.0, 0.0, 0.0,\n";        // Emissive colour
        ofs << "      2,\n";                    // Shading type (2=phong)
        ofs << "      0.3, 0.3, 0.3,\n";        // Ambient colour

        // Now output texture image info.
        string txtFname = model.getTextureFilename(xx);
        unsigned long imgWd=0, imgHgt=0;
        if (txtFname != "" && txtFname.size() > 0)
        {
            // Get the image size by loading the image
            ImgC4UC     tmpImg;
            imgLoadAnyFormat(path.dir()+txtFname,tmpImg);
            {
                imgWd = tmpImg.width();
                imgHgt = tmpImg.height();
            }
        }

        if (imgWd != 0 && imgHgt != 0)
        {
            string objTexName = objName + string(".Material.") 
                              + objName + string(".texture.map");
            ofs << "      SI_Texture2D " << objTexName << "\n"
                "      {\n"
                "         \"" << txtFname << "\";\n"
                "         3;\n"              // UV map (unwrapped)
                "         " << imgWd << ";" << imgHgt << ";\n"
                "         0;" << imgWd-1 << ";0;" << imgHgt-1 << ";\n"
                "         0;\n"              // No UV swap
                "         1;1;\n"
                "         0;0;\n"
                "         1.0;1.0;\n"        // UV scale
                "         0.0;0.0;\n"        // UV offset
                "         1.0,0.0,0.0,0.0,\n"    // Project matrix
                "         0.0,1.0,0.0,0.0,\n"
                "         0.0,0.0,1.0,0.0,\n"
                "         0.0,0.0,0.0,1.0;;\n"
                "         3;\n"              // No mask blending
                "         1.0;\n"            // Blending value
                "         0.0;\n"            // Texture ambient
                "         0.0;\n"            // Texture diffuse
                "         0.0;\n"            // Texture specular
                "         0.0;\n"            // Texture transparency
                "         0.0;\n"            // Texture reflectivity
                "         0.0;\n"            // Texture roughness
                "      }\n";
        }

        ofs << "   }\n";
    }
    ofs << "}\n";
    ofs << "\n";

    //
    // Now we output the models
    //
    for (xx=0; xx<model.numObjs(); ++xx)
    {
        string objName = model.getModelName(xx);

        // Get aliase names for all the lists.
        const vector<Vec3F>    &vtxList = model.getPtList(xx);
        const vector<Vec3UI>    &triList = model.getTriList(xx);
        const vector<Vec4UI>    &quadList = model.getQuadList(xx);
        const vector<Vec2F>    &txtList = model.getTextCoord(xx);
        const vector<Vec3UI>    &txtTriList = model.getTexTriList(xx);
        const vector<Vec4UI>    &txtQuadList = model.getTexQuadList(xx);
        bool perFacet = false;
        bool perVertex = false;
        if (vtxList.size() == txtList.size() &&
            txtTriList.size() == 0 &&
            txtQuadList.size() == 0)
            perVertex = true;
        else if (txtList.size() > 0 &&
                 txtTriList.size() == triList.size() &&
                 txtQuadList.size() == quadList.size())
            perFacet = true;

        // Create a new list for vertex normals
        vector<Vec3F> normList;
        normList.resize(vtxList.size());

        // Output the current model
        ofs << "SI_Model MDL-" << objName << "\n"
            "{\n"
            "   SI_Transform SRT-" << objName << "\n"
            "   {\n"
            "      1.0, 1.0, 1.0, \n"
            "      0.0, 0.0, 0.0, \n"
            "      0.0, 0.0, 0.0, \n"
            "   }\n"
            "\n"
            "   SI_GlobalMaterial\n"
            "   {\n"
            "      \"" << objName << "\",\n"
            "      \"BRANCH\",\n"
            "   }\n"
            "\n"
            "   SI_Visibility\n"
            "   {\n"
            "      1, \n"
            "   }\n"
            "\n"
            "   SI_Mesh MSH-" << objName << "\n"
            "   {\n";

            // Coordinates (position, normal, and UV)
        ofs << "      SI_Shape SHP-" << objName << "-ORG\n"
            "      {\n";
        if (perVertex || perFacet)
            ofs << "         3,\n";
        else
            ofs << "         2,\n";
        ofs << "         \"ORDERED\",\n"
            "\n"
            "         " << vtxList.size() << ",\n"
            "         \"POSITION\",\n";
        unsigned long pt;
        for (pt=0; pt<vtxList.size(); ++pt)
        {
            ofs << "         "
                << floatToString(vtxList[pt][0]) << ", "
                << floatToString(vtxList[pt][1]) << ", "
                << floatToString(vtxList[pt][2]) << ",\n";

            // Initialize the normal list to zero.
            normList[pt] = Vec3F(0);
        }
        ofs << "\n";
        // Calculate the normal first
        for (unsigned long tt=0; tt<triList.size(); ++tt)
        {
            Vec3F norm = calTriNormals(vtxList,triList,tt);
            normList[ triList[tt][0] ] += norm;
            normList[ triList[tt][1] ] += norm;
            normList[ triList[tt][2] ] += norm;
        }
        for (unsigned long qq=0; qq<quadList.size(); ++qq)
        {
            Vec3F norm = calQuadNormals(vtxList,quadList,qq);
            normList[ quadList[qq][0] ] += norm;
            normList[ quadList[qq][1] ] += norm;
            normList[ quadList[qq][2] ] += norm;
            normList[ quadList[qq][3] ] += norm;
        }
        ofs << "         " << normList.size() << ",\n";
        ofs << "         \"NORMAL\",\n";
        for (pt=0; pt<normList.size(); ++pt)
        {
            float ln = normList[pt].len();
            if (ln > 0.0f) normList[pt] /= ln;
            ofs << "         "
                << floatToString(normList[pt][0]) << ", "
                << floatToString(normList[pt][1]) << ", "
                << floatToString(normList[pt][2]) << ",\n";
        }
        if (perVertex || perFacet)
        {
            ofs << "\n"
                "         " << txtList.size() << ",\n"
                "         \"TEX_COORD_UV\",\n";
            for (pt=0; pt<txtList.size(); ++pt)
            {
                ofs << "         "
                    << floatToString(txtList[pt][0]) << ", "
                    << floatToString(txtList[pt][1]) << ", \n";
            }
        }
        ofs << "      }\n";

            //
            // Facets
            //
        if (triList.size())
        {
            ofs << "\n";
            ofs << "      SI_TriangleList " << objName << "\n";
            ofs << "      {\n";
            ofs << "         " << triList.size() << ",\n";
            if (perVertex || perFacet)
                ofs << "         \"NORMAL|TEX_COORD_UV\",\n";
            else
                ofs << "         \"NORMAL\",\n";
            ofs << "         \"" << objName << "\",\n";

            // Triangle vertex id.
            ofs << "\n";
            unsigned long tri;
            for (tri=0; tri<triList.size(); ++tri)
            {
                ofs << "         "
                    << triList[tri][0] << ", "
                    << triList[tri][1] << ", "
                    << triList[tri][2] << ", \n";
            }

            // Triangle's normal id.
            ofs << "\n";
            for (tri=0; tri<triList.size(); ++tri)
            {
                ofs << "         "
                    << triList[tri][0] << ", "
                    << triList[tri][1] << ", "
                    << triList[tri][2] << ", \n";
            }

            // Triangle's texture coordinate id.
            if (perVertex)
            {
                ofs << "\n";
                for (tri=0; tri<triList.size(); ++tri)
                {
                    ofs << "         "
                        << triList[tri][0] << ", "
                        << triList[tri][1] << ", "
                        << triList[tri][2] << ", \n";
                }
            }
            else if (perFacet)
            {
                ofs << "\n";
                for (tri=0; tri<txtTriList.size(); ++tri)
                {
                    ofs << "         "
                        << txtTriList[tri][0] << ", "
                        << txtTriList[tri][1] << ", "
                        << txtTriList[tri][2] << ", \n";
                }
            }
            ofs << "      }\n";
        }

        if (quadList.size())
        {
            ofs << "\n";
            ofs << "      SI_PolygonList " << objName << "\n";
            ofs << "      {\n";
            ofs << "         " << quadList.size() << ",\n";
            if (perVertex || perFacet)
                ofs << "         \"NORMAL|TEX_COORD_UV\",\n";
            else
                ofs << "         \"NORMAL\",\n";
            ofs << "         \"" << objName << "\",\n";
            ofs << "         " << 4*quadList.size() << ",\n";
            unsigned long qu;
            for (qu=0; qu<quadList.size(); ++qu)
            {
                ofs << "         4,\n";
            }

            // Quad's vertex id.
            ofs << "\n";
            for (qu=0; qu<quadList.size(); ++qu)
            {
                ofs << "         "
                    << quadList[qu][0] << ", "
                    << quadList[qu][1] << ", "
                    << quadList[qu][2] << ", "
                    << quadList[qu][3] << ", \n";
            }

            // Quad's normal id.
            ofs << "\n";
            for (qu=0; qu<quadList.size(); ++qu)
            {
                ofs << "         "
                    << quadList[qu][0] << ", "
                    << quadList[qu][1] << ", "
                    << quadList[qu][2] << ", "
                    << quadList[qu][3] << ", \n";
            }

            // Quad's texture coordinate id.
            if (perVertex)
            {
                ofs << "\n";
                for (qu=0; qu<quadList.size(); ++qu)
                {
                    ofs << "         "
                        << quadList[qu][0] << ", "
                        << quadList[qu][1] << ", "
                        << quadList[qu][2] << ", "
                        << quadList[qu][3] << ", \n";
                }
            }
            else if (perFacet)
            {
                ofs << "\n";
                for (qu=0; qu<txtQuadList.size(); ++qu)
                {
                    ofs << "         "
                        << txtQuadList[qu][0] << ", "
                        << txtQuadList[qu][1] << ", "
                        << txtQuadList[qu][2] << ", "
                        << txtQuadList[qu][3] << ", \n";
                }
            }
            ofs << "      }\n";
        }

        // Output morph targets as animations
        if (morphTargets)
        {
            size_t numMorphs = morphTargets->size();
            if (numMorphs)
            {
                ofs << "\n";
                ofs << "      SI_ShapeAnimation SHPANIM-" << objName << "\n";
                ofs << "      {\n";
                ofs << "         \"LINEAR\",\n";
                ofs << "         " << numMorphs+1 << ",\n";
                for (unsigned long mm=0; mm<numMorphs+1; ++mm)
                {
                    const vector<Vec3F> *mvtxList = &vtxList;
                    if (mm > 0)
                        mvtxList = &((*morphTargets)[mm-1].getPtList(xx));

                    normList.resize( mvtxList->size() );

                    ofs << "\n";
                    ofs << "         SI_Shape SHP-" << objName 
                                                    << "-" << mm << "\n";
                    ofs << "         {\n";
                    if (perVertex || perFacet)
                        ofs << "            3,\n";
                    else
                        ofs << "            2,\n";
                    ofs << "            \"INDEXED\",\n";
 
                    // New vertex list.
                    ofs << "\n";
                    ofs << "            " << mvtxList->size() << ",\n";
                    ofs << "            \"POSITION\",\n";
                    for (pt=0; pt<mvtxList->size(); ++pt)
                    {
                        ofs << "            " << pt << ", "
                            << floatToString((*mvtxList)[pt][0]) << ", "
                            << floatToString((*mvtxList)[pt][1]) << ", "
                            << floatToString((*mvtxList)[pt][2]) << ",\n";

                        // Initialize the normal list to zero.
                        normList[pt] = Vec3F(0.0f);
                    }

                    // New normal list.
                    size_t  tt;
                    for (tt=0; tt<triList.size(); ++tt)
                    {
                        Vec3F norm=calTriNormals(*mvtxList,triList,ulong(tt));
                        normList[ triList[tt][0] ] += norm;
                        normList[ triList[tt][1] ] += norm;
                        normList[ triList[tt][2] ] += norm;
                    }
                    size_t  qq;
                    for (qq=0; qq<quadList.size(); ++qq)
                    {
                        Vec3F norm=calQuadNormals(*mvtxList,quadList,ulong(qq));
                        normList[ quadList[qq][0] ] += norm;
                        normList[ quadList[qq][1] ] += norm;
                        normList[ quadList[qq][2] ] += norm;
                        normList[ quadList[qq][3] ] += norm;
                    }
                    ofs << "\n";
                    ofs << "            " << normList.size() << ",\n";
                    ofs << "            \"NORMAL\",\n";
                    for (pt=0; pt<normList.size(); ++pt)
                    {
                        float ln = normList[pt].len();
                        if (ln > 0.0f) normList[pt] /= ln;
                        ofs << "            " << pt << ", "
                            << floatToString(normList[pt][0]) << ", "
                            << floatToString(normList[pt][1]) << ", "
                            << floatToString(normList[pt][2]) << ",\n";
                    }

                    // Texture UV
                    if (perVertex || perFacet)
                    {
                        ofs << "\n";
                        ofs << "            " << txtList.size() << ",\n";
                        ofs << "            \"TEX_COORD_UV\",\n";
                        for (pt=0; pt<txtList.size(); ++pt)
                        {
                            ofs << "            " << pt << ", "
                                << floatToString(txtList[pt][0]) << ", "
                                << floatToString(txtList[pt][1]) << ", \n";
                        }
                    }

                    ofs << "         }\n";
                }
                ofs << "\n"
                    "         SI_FCurve " << objName << "-SHPANIM-1\n"
                    "         {\n"
                    "            \"" << objName << "\",\n"
                    "            \"SHPANIM-1\",\n"
                    "            \"LINEAR\",\n"
                    "            1,1,\n"
                    "            " << numMorphs+1 << ",\n";
                size_t      mm;
                for (mm=0; mm<numMorphs+1; ++mm)
                {
                    ofs << "            " << mm+1 << ", " << mm << ".0,\n";
                }
                ofs << "         }\n";
                ofs << "      }\n";
            }
        }

        // End of SI_Mesh
        ofs << "   }\n";

        // End of SI_Model
        ofs << "}\n";

        // End of current model
        ofs << "\n";
    }

    //
    // Done.
    //
    if (ofs.fail())
    {
        ofs.close();
        return false;
    }
    else
    {
        ofs.close();
        return true;
    }
}


//****************************************************************************
//                              calTriNormals
//****************************************************************************
static Vec3F calTriNormals(

    const vector<Vec3F> &vtxList,
    const vector<Vec3UI> &triList,
    unsigned long            tri)
{
    Vec3F v0 = vtxList[ triList[tri][0] ];
    Vec3F v1 = vtxList[ triList[tri][1] ];
    Vec3F v2 = vtxList[ triList[tri][2] ];

    // The least square fit normal for this triangle.
    Vec3F norm( (v0[1]-v1[1]) * (v0[2]+v1[2]) +
                     (v1[1]-v2[1]) * (v1[2]+v2[2]) +
                     (v2[1]-v0[1]) * (v2[2]+v0[2]),
                     (v0[2]-v1[2]) * (v0[0]+v1[0]) +
                     (v1[2]-v2[2]) * (v1[0]+v2[0]) +
                     (v2[2]-v0[2]) * (v2[0]+v0[0]),
                     (v0[0]-v1[0]) * (v0[1]+v1[1]) +
                     (v1[0]-v2[0]) * (v1[1]+v2[1]) +
                     (v2[0]-v0[0]) * (v2[1]+v0[1]) );

    float ln = norm.len();
    if (ln > FLT_EPSILON)
        norm /= ln;

    return norm;
}


//****************************************************************************
//                              calQuadNormals
//****************************************************************************
static Vec3F calQuadNormals(

    const vector<Vec3F> &vtxList,
    const vector<Vec4UI> &quadList,
    unsigned long            quad)
{
    Vec3F v0 = vtxList[ quadList[quad][0] ];
    Vec3F v1 = vtxList[ quadList[quad][1] ];
    Vec3F v2 = vtxList[ quadList[quad][2] ];
    Vec3F v3 = vtxList[ quadList[quad][3] ];

    // The least square fit normal for this quad polygon.
    Vec3F norm( (v0[1]-v1[1]) * (v0[2]+v1[2]) +
                     (v1[1]-v2[1]) * (v1[2]+v2[2]) +
                     (v2[1]-v3[1]) * (v2[2]+v3[2]) +
                     (v3[1]-v0[1]) * (v3[2]+v0[2]),
                     (v0[2]-v1[2]) * (v0[0]+v1[0]) +
                     (v1[2]-v2[2]) * (v1[0]+v2[0]) +
                     (v2[2]-v3[2]) * (v2[0]+v3[0]) +
                     (v3[2]-v0[2]) * (v3[0]+v0[0]),
                     (v0[0]-v1[0]) * (v0[1]+v1[1]) +
                     (v1[0]-v2[0]) * (v1[1]+v2[1]) +
                     (v2[0]-v3[0]) * (v2[1]+v3[1]) +
                     (v3[0]-v0[0]) * (v3[1]+v0[1]) );

    float ln = norm.len();
    if (ln > FLT_EPSILON)
        norm /= ln;

    return norm;
}


//****************************************************************************
//                              floatToString
//****************************************************************************
//
// Softimage|3D has a very old parser and requires all floating point output
// to be printed as x.xxx even if the number doesn't have any decimal values.
//
static string floatToString(float val)
{
    std::ostringstream   oss;
    oss << std::fixed << std::setw(4) << std::setfill('0') << val;
    return oss.str();
}

void
saveXsi(
    Ustring const &        fname,
    const vector<Mesh> & meshes,
    string                  imgFormat)
{
    FgMeshLegacy    ml = fgMeshLegacy(meshes,fname,imgFormat);
    if (ml.morphs.empty())
        fffSaveXsiFile(fname,ml.base,"FaceGen");
    else
        fffSaveXsiFile(fname,ml.base,ml.morphs,"FaceGen");
}

void
fgSaveXsiTest(CLArgs const & args)
{
    FGTESTDIR
    Ustring    dd = dataDir();
    string      rd = "base/";
    Mesh    mouth = loadTri(dd+rd+"Mouth.tri");
    mouth.surfaces[0].setAlbedoMap(imgLoadAnyFormat(dd+rd+"MouthSmall.png"));
    Mesh    glasses = loadTri(dd+rd+"Glasses.tri");
    glasses.surfaces[0].setAlbedoMap(imgLoadAnyFormat(dd+rd+"Glasses.tga"));
    saveXsi("meshExportXsi",fgSvec(mouth,glasses));
    regressFileRel("meshExportXsi.xsi","base/test/");
    regressFileRel("meshExportXsi0.png","base/test/");
    regressFileRel("meshExportXsi1.png","base/test/");
}

}

// */
