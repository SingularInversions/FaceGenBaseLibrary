//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: John Leung
// Created: Feb 5, 2002
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

using namespace std;

template <class T>
struct FR2Vect2LessThanByAxisT
{
    bool operator()(const FgMatrixC<T,2,1> &x, const FgMatrixC<T,2,1> &y) const
            { return ((x[1] < y[1]) ||
                      (x[1] == y[1] && x[0] < y[0])); }
};

typedef FR2Vect2LessThanByAxisT<short>  FutVect2LessSC;
typedef FR2Vect2LessThanByAxisT<uint>   FutVect2LessIC;
typedef FR2Vect2LessThanByAxisT<float>  FutVect2LessFC;
typedef FR2Vect2LessThanByAxisT<double> FutVect2LessDC;

typedef map<FgVect2UI,unsigned long,FutVect2LessIC> EdgeMapT;
typedef EdgeMapT::iterator EdgeMapItr;
typedef EdgeMapT::const_iterator EdgeMapCItr;


//****************************************************************************
// Local function prototypes
//****************************************************************************
static bool saveMayaAsciiFile(
        const FgString                    &fname,
        const FffMultiObjectC           &model,
        const vector<FffMultiObjectC>   *morphTargets,
        const vector<string>            *morphNames,
        const vector<string>            *cmts);
static void buildEdgeList(
        const vector<FgVect3UI>    &triList,
        const vector<FgVect4UI>    &quadList,
        EdgeMapT                    &edgeMap,
        vector<FgVect2UI>          &edgeList);
static bool findEdgeId(
        unsigned long   vtx1,
        unsigned long   vtx2,
        const EdgeMapT  &edgeMap,
        long            &edgeId);

static string neutralGrpName() { return string("Neutral"); }
static string getObjectName(
        const FffMultiObjectC &model, unsigned long objId);
static string getObjectShapeName(
        const FffMultiObjectC &model, unsigned long objId);
static string getObjectShapeOrigName(
        const FffMultiObjectC &model, unsigned long objId);
static string getObjPolySurfName(
        const FffMultiObjectC &model, unsigned long objId);
static string getObjPolySoftEdgeName(
        const FffMultiObjectC &model, unsigned long objId);
static string getObjPolySoftEdgeGrpId(
        const FffMultiObjectC &model, unsigned long objId);
static string getObjPolySoftEdgeGrpParts(
        const FffMultiObjectC &model, unsigned long objId);

static string getMorphName(const string &morphName);
static string getObjMorphName(
        const FffMultiObjectC &model, unsigned long objId, 
        const string &morphName);
static string getObjMorphShapeName(
        const FffMultiObjectC &model, unsigned long objId, 
        const string &morphName);

static string getObjectShadingEngName(
        const FffMultiObjectC &model, unsigned long mm);
static string getObjectMaterialName(
        const FffMultiObjectC &model, unsigned long mm);
static string getObjectPhongName(
        const FffMultiObjectC &model, unsigned long mm);
static string getObjectShadeGrpIdName(
        const FffMultiObjectC &model, unsigned long mm);
static string getObjectNodeGrpIdName(
        const FffMultiObjectC &model, unsigned long mm);

static string getObjMorphShadingEngName(
        const FffMultiObjectC &model, unsigned long mm, const string &name);
static string getObjMorphMaterialName(
        const FffMultiObjectC &model, unsigned long mm, const string &name);
static string getObjMorphPhongName(
        const FffMultiObjectC &model, unsigned long mm, const string &name);
static string getObjMorphShadeGrpIdName(
        const FffMultiObjectC &model, unsigned long mm, const string &name);
static string getObjMorphNodeGrpIdName(
        const FffMultiObjectC &model, unsigned long mm, const string &name);

static string getBlendShapeName();
static string getBlendShapeSetName();
static string getObjBlendShapeName(
        const FffMultiObjectC &model, unsigned long objId);
static string getObjBlendTexGrpIdName(
        const FffMultiObjectC &model, unsigned long mm);
static string getObjBlendTexGrpPartsName(
        const FffMultiObjectC &model, unsigned long mm);
static string getObjBlendShapeGrpIdName(
        const FffMultiObjectC &model, unsigned long objId);
static string getObjBlendShapeGrpPartsName(
        const FffMultiObjectC &model, unsigned long objId);
static string getObjTweakName(
        const FffMultiObjectC &model, unsigned long objId);
static string getObjTweakSetName(
        const FffMultiObjectC &model, unsigned long objId);
static string getObjTweakGrpIdName(
        const FffMultiObjectC &model, unsigned long objId);
static string getObjTweakGrpPartsName(
        const FffMultiObjectC &model, unsigned long objId);
static string getObjectTexFileName(
        const FffMultiObjectC &model, unsigned long mm);
static string getObjectTexPlaceName(
        const FffMultiObjectC &model, unsigned long mm);

static void writeTexCoord(ofstream &ofs, const vector<FgVect2F> &texCoord);
static void writeVertices(ofstream &ofs, const vector<FgVect3F> &vtxList);
static void writeEdges(ofstream &ofs, const vector<FgVect2UI> &edgeList);
static void writeFacets(
        ofstream                    &ofs,
        const vector<FgVect3F>    &vtxList,
        const vector<FgVect3UI>    &triList,
        const vector<FgVect4UI>    &quadList,
        const EdgeMapT              &edgeMap,
        const vector<FgVect2F>    &texCoord,
        const vector<FgVect3UI>    &texTriList,
        const vector<FgVect4UI>    &texQuadList);
static void writeObjects(
        ofstream                        &ofs,
        const FffMultiObjectC           &model,
        const vector<FffMultiObjectC>   *morphTargets,
        const vector<string>            *morphNames,
        vector<int>                     &edgeSizeList);
static int writeShadingMaterialPhong(
        ofstream                        &ofs,
        const FffMultiObjectC           &model,
        const vector<string>            *morphNames);
static void writeBlendShapes(
        ofstream                        &ofs, 
        const FffMultiObjectC           &model,
        const vector<string>            *morphNames);
static void writePolySoftEdge(
        ofstream                        &ofs, 
        const FffMultiObjectC           &model,
        const vector<string>            *morphNames,
        const vector<int>               &edgeSizeList);
static void connectAttributes(
        ofstream                        &ofs,
        const FffMultiObjectC           &model,
        const vector<string>            *morphNames);


//****************************************************************************
//                              fffSaveMayaAsciiFile
//****************************************************************************
static bool    fffSaveMayaAsciiFile(

    const FgString            &fname,
    const FffMultiObjectC   &model,
    const vector<string>    *cmts)
{
    return saveMayaAsciiFile(fname,model,0,0,cmts);
}

static bool    fffSaveMayaAsciiFile(

    const FgString                    &fname,
    const FffMultiObjectC           &model,
    const vector<FffMultiObjectC>   &morphTargets,
    const vector<string>            &morphNames,
    const vector<string>            *cmts)
{
    return saveMayaAsciiFile(fname,model,
                             &morphTargets,&morphNames,cmts);
}

//****************************************************************************
//                              saveMayaAsciiFile
//****************************************************************************
static bool saveMayaAsciiFile(

    const FgString                  &fname,
    const FffMultiObjectC           &model,
    const vector<FffMultiObjectC>   *morphTargets,
    const vector<string>            *morphNames,
    const vector<string>            *cmts)
{
    unsigned long totalNumTargets=0;
    if (morphTargets && morphNames)
    {
        if (morphTargets->size() == morphNames->size())
            totalNumTargets = ulong(morphTargets->size());
        else
        {
            return false;
        }
    }
    else if (morphTargets)
    {
        return false;
    }
    else if (morphNames)
    {
        return false;
    }

    FgPath      path(fname);
    path.ext = "ma";
    FgOfstream ofs(path.str());
    if (!ofs)
    {
        return false;
    }

    //
    // Write header
    //
    ofs << "//Maya ASCII 4.0 scene\n";
    ofs << "//Name: " << path.base << endl;
    if (cmts)
    {
        ofs << "//\n";
        for (unsigned long cc=0; cc<cmts->size(); ++cc)
        {
            ofs << "//" << (*cmts)[cc] << endl;
        }
        ofs << "//\n";
    }
    ofs << "requires maya \"4.0\";\n";
    ofs << flush;

    //
    // Now write out the shape information for each object and target.
    //
    vector<int> edgeSizeList;
    writeObjects(ofs,model,morphTargets,morphNames,edgeSizeList);

    //
    // Create the rendering / lighting nodes
    //
    ofs << "createNode lightLinker -n \"lightLinker1\";\n";
    ofs << "createNode displayLayerManager -n \"layerManager\";\n";
    ofs << "createNode displayLayer -n \"defaultLayer\";\n";
    ofs << "createNode renderLayerManager -n \"renderLayerManager\";\n";
    ofs << "createNode renderLayer -n \"defaultRenderLayer\";\n";
    ofs << "createNode renderLayer -s -n \"globalRender\";\n";

    //
    // Create Texture related nodes
    //
    int numObjsWithTexture = writeShadingMaterialPhong(ofs,model,morphNames);

    //
    // Create blend shapes
    //
    writeBlendShapes(ofs,model,morphNames);

    //
    // Create the polySoftEdge nodes
    //
    writePolySoftEdge(ofs,model,morphNames,edgeSizeList);

    //
    // Make selection
    //
    int numRenders = 2 + numObjsWithTexture;
    int numSgDsm = 0;
    int numSgGn = 0;
    for (unsigned long mm=0; mm<model.numObjs(); ++mm)
    {
        int numMorphs = totalNumTargets;

        string texFname = model.getTextureFilename(mm);
        if (texFname.length() == 0 || texFname == "")
        {
            numSgDsm += (1 + numMorphs);
            continue;
        }

        numRenders += numMorphs;
        numSgDsm += numMorphs;
        numSgGn += numMorphs;
    }

    ofs << "select -ne :time1;\n";
    ofs << "\tsetAttr \".o\" 1;\n";
    ofs << "select -ne :renderPartition;\n";
    ofs << "\tsetAttr -size " << numRenders << " \".st\";\n";
    ofs << "select -ne :renderGlobalsList1;\n";
    ofs << "select -ne :defaultShaderList1;\n";
    ofs << "\tsetAttr -s " << numRenders << " \".s\";\n";
    ofs << "select -ne :postProcessList1;\n";
    ofs << "\tsetAttr -s " << "2 \".p\";\n";
    if (numObjsWithTexture)
    {
        ofs << "select -ne :defaultRenderUtilityList1;\n";
        ofs << "\tsetAttr -s " << numObjsWithTexture << " \".u\";\n";
    }
    ofs << "select -ne :lightList1;\n";
    if (numObjsWithTexture)
    {
        ofs << "select -ne :defaultTextureList1;\n";
        ofs << "\tsetAttr -s " << numObjsWithTexture << " \".tx\";\n";
    }
    ofs << "select -ne :initialShadingGroup;\n";
    if (numSgDsm)
    {
        ofs << "\tsetAttr -s " << numSgDsm << " \".dsm\";\n";
    }
    ofs << "\tsetAttr \".ro\" yes;\n";
    if (numSgGn)
    {
        ofs << "\tsetAttr -s " << numSgGn << " \".gn\";\n";
    }
    ofs << "select -ne :initialParticleSE;\n";
    ofs << "\tsetAttr \".ro\" yes;\n";

    //
    // Make connections
    //
    connectAttributes(ofs,model,morphNames);

    ofs << "// End of " << path.base << endl << flush;

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
//                              buildEdgeList
//****************************************************************************
static void buildEdgeList(

    const vector<FgVect3UI>    &triList,
    const vector<FgVect4UI>    &quadList,
    EdgeMapT                    &edgeMap,
    vector<FgVect2UI>          &edgeList)
{
    unsigned long edgeId=0;

    for (unsigned long tt=0; tt<triList.size(); ++tt)
    {
        for (int ii=0; ii<3; ++ii)
        {
            int ii2 = (ii+1) % 3;
            FgVect2UI edg1 = FgVect2UI(triList[tt][ii],
                                         triList[tt][ii2]);
            FgVect2UI edg2 = FgVect2UI(edg1[1],edg1[0]);
            if (edgeMap.find(edg1) != edgeMap.end() ||
                edgeMap.find(edg2) != edgeMap.end())
                continue;

            // New edge
            edgeMap[edg1] = edgeId;
            ++edgeId;
        }
    }

    for (unsigned long qq=0; qq<quadList.size(); ++qq)
    {
        for (int ii=0; ii<4; ++ii)
        {
            int ii2 = (ii+1) % 4;
            FgVect2UI edg1 = FgVect2UI(quadList[qq][ii],
                                         quadList[qq][ii2]);
            FgVect2UI edg2 = FgVect2UI(edg1[1],edg1[0]);
            if (edgeMap.find(edg1) != edgeMap.end() ||
                edgeMap.find(edg2) != edgeMap.end())
                continue;

            // New edge
            edgeMap[edg1] = edgeId;
            ++edgeId;
        }
    }

    edgeList.resize(edgeId);
    for (EdgeMapItr itr=edgeMap.begin(); itr != edgeMap.end(); ++itr)
    {
        edgeList[itr->second] = itr->first;
    }
}


//****************************************************************************
//                              findEdgeId
//****************************************************************************
static bool findEdgeId(

    unsigned long   vtx1,
    unsigned long   vtx2,
    const EdgeMapT  &edgeMap,
    long            &edgeId)
{
    FgVect2UI edge1 = FgVect2UI(vtx1,vtx2);
    FgVect2UI edge2 = FgVect2UI(vtx2,vtx1);

    EdgeMapCItr itr;

    if ((itr=edgeMap.find(edge1)) != edgeMap.end())
        edgeId = (long)itr->second;
    else if ((itr=edgeMap.find(edge2)) != edgeMap.end())
        edgeId = -(long)((itr->second+1));
    else
        return false;

    return true;
}


//****************************************************************************
//                              getObjectName
//****************************************************************************
static string getObjectName(const FffMultiObjectC &model, unsigned long objId)
{
    return model.getModelName(objId);
}


//****************************************************************************
//                              getObjectShapeName
//****************************************************************************
static string getObjectShapeName(

    const FffMultiObjectC   &model, 
    unsigned long           objId)
{
    return (getObjectName(model,objId) + string("Shape"));
}


//****************************************************************************
//                          getObjectShapeOrigName
//****************************************************************************
static string getObjectShapeOrigName(

    const FffMultiObjectC   &model, 
    unsigned long           objId)
{
    return (getObjectShapeName(model,objId) + string("Orig"));
}


//****************************************************************************
//                          getObjPolySurfName
//****************************************************************************
static string getObjPolySurfName(

    const FffMultiObjectC   &model, 
    unsigned long           objId)
{
    return (getObjectName(model,objId) + string("PolySurfaceShape"));
}


//****************************************************************************
//                          getObjPolySoftEdgeName
//****************************************************************************
static string getObjPolySoftEdgeName(

    const FffMultiObjectC   &model, 
    unsigned long           objId)
{
    return (getObjectName(model,objId) + string("PolySoftEdge"));
}


//****************************************************************************
//                          getObjPolySoftEdgeGrpId
//****************************************************************************
static string getObjPolySoftEdgeGrpId(

    const FffMultiObjectC   &model, 
    unsigned long           objId)
{
    return (getObjPolySoftEdgeName(model,objId) + string("GrpId"));
}


//****************************************************************************
//                          getObjPolySoftEdgeGrpParts
//****************************************************************************
static string getObjPolySoftEdgeGrpParts(

    const FffMultiObjectC   &model, 
    unsigned long           objId)
{
    return (getObjPolySoftEdgeName(model,objId) + string("GrpParts"));
}


//****************************************************************************
//                              getMorphName
//****************************************************************************
static string getMorphName(const string &morphName)
{
    string name = morphName;

    unsigned long ln = ulong(strlen(name.c_str()));

    for (ulong xx=0; xx<ln; ++xx)
    {
        if (name[xx] == ':' || name[xx] == ' ' || name[xx] == ',' ||
            name[xx] == '.' || name[xx] == ';')
            name[xx] = '_';
    }

    return name;
}


//****************************************************************************
//                              getObjMorphName
//****************************************************************************
static string getObjMorphName(

    const FffMultiObjectC   &model, 
    unsigned long           objId, 
    const string            &morphName)
{
    return (model.getModelName(objId)+string("_")+getMorphName(morphName));
}


//****************************************************************************
//                          getObjMorphShapeName
//****************************************************************************
static string getObjMorphShapeName(

    const FffMultiObjectC   &model, 
    unsigned long           objId, 
    const string            &morphName)
{
    return (getObjMorphName(model,objId,morphName) + string("Shape"));
}


//****************************************************************************
//                          getObjectShadingEngName
//****************************************************************************
static string getObjectShadingEngName(

    const FffMultiObjectC   &model, 
    unsigned long           mm)
{
    return (getObjectName(model,mm) + string("ShadingEngine"));
}


//****************************************************************************
//                          getObjectMaterialName
//****************************************************************************
static string getObjectMaterialName(

    const FffMultiObjectC   &model, 
    unsigned long           mm)
{
    return (getObjectName(model,mm) + string("Material"));
}


//****************************************************************************
//                          getObjectPhongName
//****************************************************************************
static string getObjectPhongName(

    const FffMultiObjectC   &model, 
    unsigned long           mm)
{
    return (getObjectName(model,mm) + string("Phong"));
}


//****************************************************************************
//                          getObjectShadeGrpIdName
//****************************************************************************
static string getObjectShadeGrpIdName(

    const FffMultiObjectC   &model, 
    unsigned long           mm)
{
    return (getObjectName(model,mm) + string("ShadeGrpId"));
}


//****************************************************************************
//                          getObjectNodeGrpIdName
//****************************************************************************
static string getObjectNodeGrpIdName(

    const FffMultiObjectC   &model, 
    unsigned long           mm)
{
    return (getObjectName(model,mm) + string("NodeGrpId"));
}


//****************************************************************************
//                          getObjMorphShadingEngName
//****************************************************************************
static string getObjMorphShadingEngName(

    const FffMultiObjectC   &model, 
    unsigned long           mm, 
    const string            &name)
{
    return (getObjMorphName(model,mm,name) + string("ShadingEngine"));
}


//****************************************************************************
//                          getObjMorphMaterialName
//****************************************************************************
static string getObjMorphMaterialName(

    const FffMultiObjectC   &model, 
    unsigned long           mm, 
    const string            &name)
{
    return (getObjMorphName(model,mm,name) + string("Material"));
}


//****************************************************************************
//                          getObjMorphPhongName
//****************************************************************************
static string getObjMorphPhongName(

    const FffMultiObjectC   &model, 
    unsigned long           mm, 
    const string            &name)
{
    return (getObjMorphName(model,mm,name) + string("Phong"));
}


//****************************************************************************
//                          getObjMorphShadeGrpIdName
//****************************************************************************
static string getObjMorphShadeGrpIdName(

    const FffMultiObjectC   &model, 
    unsigned long           mm, 
    const string            &name)
{
    return (getObjMorphName(model,mm,name) + string("ShadeGrpId"));
}


//****************************************************************************
//                          getObjMorphNodeGrpIdName
//****************************************************************************
static string getObjMorphNodeGrpIdName(

    const FffMultiObjectC   &model, 
    unsigned long           mm, 
    const string            &name)
{
    return (getObjMorphName(model,mm,name) + string("NodeGrpId"));
}


//****************************************************************************
//                          getBlendShapeName
//****************************************************************************
static string getBlendShapeName()
{
    return string("BlendShape");
}


//****************************************************************************
//                          getBlendShapeSetName
//****************************************************************************
static string getBlendShapeSetName()
{
    return string("BlendShapeSet");
}


//****************************************************************************
//                          getObjBlendShapeName
//****************************************************************************
static string getObjBlendShapeName(

    const FffMultiObjectC   &model, 
    unsigned long           objId)
{

    return (model.getModelName(objId) + string("BlendShape"));
}


//****************************************************************************
//                          getObjBlendTexGrpIdName
//****************************************************************************
static string getObjBlendTexGrpIdName(

    const FffMultiObjectC   &model, 
    unsigned long           mm)
{
    return (getObjBlendShapeName(model,mm) + string("ShadeGrpId"));
}


//****************************************************************************
//                          getObjBlendTexGrpPartsName
//****************************************************************************
static string getObjBlendTexGrpPartsName(

    const FffMultiObjectC   &model, 
    unsigned long           mm)
{
    return (getObjBlendShapeName(model,mm) + string("ShadeGrpParts"));
}


//****************************************************************************
//                          getObjBlendShapeGrpIdName
//****************************************************************************
static string getObjBlendShapeGrpIdName(

    const FffMultiObjectC   &model, 
    unsigned long           objId)
{
    return (getObjBlendShapeName(model,objId) + string("GrpId"));
}


//****************************************************************************
//                          getObjBlendShapeGrpPartsName
//****************************************************************************
static string getObjBlendShapeGrpPartsName(

    const FffMultiObjectC   &model, 
    unsigned long           objId)
{
    return (getObjBlendShapeName(model,objId) + string("GrpParts"));
}


//****************************************************************************
//                              getObjTweakName
//****************************************************************************
static string getObjTweakName(

    const FffMultiObjectC   &model, 
    unsigned long           objId)
{
    return (model.getModelName(objId) + string("Tweak"));
}


//****************************************************************************
//                              getObjTweakSetName
//****************************************************************************
static string getObjTweakSetName(

    const FffMultiObjectC   &model, 
    unsigned long           objId)
{
    return (getObjTweakName(model,objId) + string("Set"));
}


//****************************************************************************
//                              getObjTweakGrpIdName
//****************************************************************************
static string getObjTweakGrpIdName(

    const FffMultiObjectC   &model, 
    unsigned long           objId)
{
    return (getObjTweakName(model,objId) + string("GrpId"));
}


//****************************************************************************
//                          getObjTweakGrpPartsName
//****************************************************************************
static string getObjTweakGrpPartsName(

    const FffMultiObjectC   &model, 
    unsigned long           objId)
{
    return (getObjTweakName(model,objId) + string("GrpParts"));
}


//****************************************************************************
//                          getObjectTexFileName
//****************************************************************************
static string getObjectTexFileName(

    const FffMultiObjectC   &model, 
    unsigned long           mm)
{
    return (getObjectName(model,mm) + string("File"));
}


//****************************************************************************
//                          getObjectTexPlaceName
//****************************************************************************
static string getObjectTexPlaceName(

    const FffMultiObjectC   &model, 
    unsigned long           mm)
{
    return (getObjectName(model,mm) + string("Place2dTexture"));
}


//****************************************************************************
//                              writeTexCoord
//****************************************************************************
static void writeTexCoord(ofstream &ofs, const vector<FgVect2F> &texCoord)
{
    if (texCoord.size())
    {
        ofs << "\tsetAttr -s " << texCoord.size()
                    << " \".uvst[0].uvsp[0:" << texCoord.size()-1 << "]\" "
                    << "-type \"float2\"\n";

        for (unsigned long pp=0; pp<texCoord.size(); )
        {
            ofs << "\t\t";
            for (int ii=0; ii<4 && pp<texCoord.size(); ++ii, ++pp)
            {
                ofs << texCoord[pp][0] << " " << texCoord[pp][1];
                if (pp == texCoord.size()-1)
                    ofs << ";";
                else
                    ofs << " ";
            }
            ofs << "\n";
        }
    }
}


//****************************************************************************
//                              writeVertices
//****************************************************************************
static void writeVertices(ofstream &ofs, const vector<FgVect3F> &vtxList)
{
    ofs << "\tsetAttr -s " << vtxList.size() 
                << " \".vt[0:" << vtxList.size()-1 << "]\"\n";
    for (unsigned long vv=0; vv<vtxList.size(); )
    {
        ofs << "\t\t";
        for (int ii=0; ii<3 && vv<vtxList.size(); ++ii, ++vv)
        {
            ofs << vtxList[vv][0] << " " 
                << vtxList[vv][1] << " " 
                << vtxList[vv][2];
            if (vv == vtxList.size()-1)
                ofs << ";";
            else
                ofs << " ";
        }
        ofs << "\n";
    }
}


//****************************************************************************
//                              writeEdges
//****************************************************************************
static void writeEdges(ofstream &ofs, const vector<FgVect2UI> &edgeList)
{
    ofs << "\tsetAttr -s " << edgeList.size() 
                << " \".ed[0:" << edgeList.size()-1 << "]\"\n";
    for (unsigned long ee=0; ee<edgeList.size(); )
    {
        ofs << "\t\t";
        for (int ii=0; ii<7 && ee<edgeList.size(); ++ii, ++ee)
        {
            ofs << edgeList[ee][0] << " " 
                << edgeList[ee][1] << " 0";
            if (ee == edgeList.size()-1)
                ofs << ";";
            else
                ofs << " ";
        }
        ofs << "\n";
    }
}


//****************************************************************************
//                              writeFacets
//****************************************************************************
static void writeFacets(

    ofstream                    &ofs,
    const vector<FgVect3F>    &vtxList,
    const vector<FgVect3UI>    &triList,
    const vector<FgVect4UI>    &quadList,
    const EdgeMapT              &edgeMap,
    const vector<FgVect2F>    &texCoord,
    const vector<FgVect3UI>    &texTriList,
    const vector<FgVect4UI>    &texQuadList)
{
    // Per-facet or per-vertex texture
    bool perFacet = false;
    bool perVertex = false;
    if (texCoord.size() == vtxList.size() &&
        texTriList.size() == 0 &&
        texQuadList.size() == 0)
        perVertex = true;
    else if (texCoord.size() > 0 &&
             texTriList.size() == triList.size() &&
             texQuadList.size() == quadList.size())
        perFacet = true;

    size_t      numTri = triList.size();
    size_t      numQuad = quadList.size();
    size_t      numFacets = numTri + numQuad;

    ofs << "\tsetAttr -s " << numFacets 
                << " \".fc[0:" << numFacets-1 << "]\" "
                << "-type \"polyFaces\"\n";
    for (size_t tt=0; tt<numTri; ++tt)
    {
        long ed1, ed2, ed3;
        if (!findEdgeId(triList[tt][0],triList[tt][1],edgeMap,ed1))
            continue;
        if (!findEdgeId(triList[tt][1],triList[tt][2],edgeMap,ed2))
            continue;
        if (!findEdgeId(triList[tt][2],triList[tt][0],edgeMap,ed3))
            continue;

        ofs << "\t\tf 3 " << ed1 << " " << ed2 << " " << ed3;
        if (perVertex)
        {
            ofs << "\n\t\tmu 0 3 " 
                << triList[tt][0] << " "
                << triList[tt][1] << " "
                << triList[tt][2];
        }
        else if (perFacet)
        {
            ofs << "\n\t\tmu 0 3 " 
                << texTriList[tt][0] << " "
                << texTriList[tt][1] << " "
                << texTriList[tt][2];
        }
        if (tt == numFacets-1) ofs << ";";
        ofs << "\n";
    }
    for (unsigned long qq=0; qq<numQuad; ++qq)
    {
        long ed1, ed2, ed3, ed4;
        if (!findEdgeId(quadList[qq][0],quadList[qq][1],edgeMap,ed1))
            continue;
        if (!findEdgeId(quadList[qq][1],quadList[qq][2],edgeMap,ed2))
            continue;
        if (!findEdgeId(quadList[qq][2],quadList[qq][3],edgeMap,ed3))
            continue;
        if (!findEdgeId(quadList[qq][3],quadList[qq][0],edgeMap,ed4))
            continue;

        ofs << "\t\tf 4 " 
            << ed1 << " " << ed2 << " " 
            << ed3 << " " << ed4;
        if (perVertex)
        {
            ofs << "\n\t\tmu 0 4 " 
                << quadList[qq][0] << " "
                << quadList[qq][1] << " "
                << quadList[qq][2] << " "
                << quadList[qq][3];
        }
        else if (perFacet)
        {
            ofs << "\n\t\tmu 0 4 " 
                << texQuadList[qq][0] << " "
                << texQuadList[qq][1] << " "
                << texQuadList[qq][2] << " "
                << texQuadList[qq][3];
        }
        if (qq == numQuad-1) ofs << ";";
        ofs << "\n";
    }
}


//****************************************************************************
//                              writeObjects
//****************************************************************************
static void writeObjects(

    ofstream                        &ofs,
    const FffMultiObjectC           &model,
    const vector<FffMultiObjectC>   *morphTargets,
    const vector<string>            *morphNames,
    vector<int>                     &edgeSizeList)
{
    // Number of morph targets
    size_t  numTargets=0;
    if (morphTargets && morphNames)
    {
        if (morphTargets->size() == morphNames->size())
            numTargets = morphTargets->size();
    }

    string neutralGrp = neutralGrpName();

    if (numTargets)
        ofs << "createNode transform -n \"" << neutralGrp << "\";\n";

    vector< EdgeMapT > edgeMap;
    vector< vector<FgVect2UI> > edgeList;
    edgeMap.resize(model.numObjs());
    edgeList.resize(model.numObjs());
    edgeSizeList.clear();
    for (unsigned long objId=0; objId<model.numObjs(); ++objId)
    {
        // Get an alias to the list data
        const vector<FgVect3F> &vtxList = model.getPtList(objId);
        const vector<FgVect3UI> &triList = model.getTriList(objId);
        const vector<FgVect4UI> &quadList = model.getQuadList(objId);
        const vector<FgVect2F> &texCoord = model.getTextCoord(objId);
        const vector<FgVect3UI> &texTriList = model.getTexTriList(objId);
        const vector<FgVect4UI> &texQuadList = model.getTexQuadList(objId);

        // Build an edge list first
        buildEdgeList(triList,quadList,edgeMap[objId],edgeList[objId]);
        edgeSizeList.push_back(int(edgeList[objId].size()));

        // Check if this object has texture
        string texFname = model.getTextureFilename(objId);
        if (texFname.length() == 0)
            texFname = "";

        // Now write out the information to file
        string objName = getObjectName(model,objId);
        ofs << "createNode transform -n \"" << objName << "\"";
        if (numTargets)
            ofs << " -p \"" << neutralGrp << "\"";
        ofs << ";\n";
        ofs << "createNode mesh "
                << "-n \"" << getObjectShapeName(model,objId) << "\" "
                << "-p \"" << objName << "\";\n";
        ofs << "\tsetAttr -k off \".v\";\n";

        size_t  numFacets = triList.size() + quadList.size();
        int numIog = 0;
        if (texFname != "") numIog += 2;
        if (numTargets) numIog += 4;
        if (numIog)
        {
            ofs << "\tsetAttr -s " << numIog << " \".iog[0].og\";\n";
        }
        if (numTargets)
        {
            ofs << "\tsetAttr \".uvst[0].uvsn\" " 
                    << "-type \"string\" \"map1\";\n";
            ofs << "\tsetAttr \".cuvs\" " << "-type \"string\" \"map1\";\n";
            ofs << "createNode mesh "
                    << "-n \"" << getObjectShapeOrigName(model,objId) << "\" "
                    << "-p \"" << objName << "\";\n";
            ofs << "\tsetAttr -k off \".v\";\n";
            ofs << "\tsetAttr \".io\" yes;\n";
            ofs << "\tsetAttr \".vir\" yes;\n";
            ofs << "\tsetAttr \".vif\" yes;\n";
        }
        else
        {
            ofs << "\tsetAttr \".uvst[0].uvsn\" " 
                    << "-type \"string\" \"map1\";\n";
            ofs << "\tsetAttr \".cuvs\" " << "-type \"string\" \"map1\";\n";
            ofs << "createNode mesh "
                    << "-n \"" << getObjPolySurfName(model,objId) << "\" "
                    << "-p \"" << objName << "\";\n";
            ofs << "\tsetAttr -k off \".v\";\n";
            ofs << "\tsetAttr \".io\" yes;\n";
            if (texFname != "")
            {
                ofs << "\tsetAttr -s " << numIog << " \".iog[0].og\";\n";
                ofs << "\tsetAttr \".iog[0].og[0].gcl\" "
                            "-type \"componentList\" 0;\n";
                ofs << "\tsetAttr \".iog[0].og[1].gcl\" "
                            "-type \"componentList\" 1 "
                            << "\"f[0:" << numFacets-1 << "]\";\n";
            }
        }

        // Texture coordinates
        ofs << "\tsetAttr \".uvst[0].uvsn\" -type \"string\" \"map1\";\n";
        writeTexCoord(ofs,texCoord);
        ofs << "\tsetAttr \".cuvs\" -type \"string\" \"map1\";\n";

        // Vertices
        writeVertices(ofs,vtxList);

        // Edges
        writeEdges(ofs,edgeList[objId]);

        // Faces
        writeFacets(ofs,vtxList,triList,quadList,edgeMap[objId],
                    texCoord,texTriList,texQuadList);
    }

    // Now the morph targets
    for (unsigned long mm=0; mm < numTargets; ++mm)
    {
        string morphGrp = getMorphName((*morphNames)[mm]);

        ofs << "createNode transform -n \"" << morphGrp << "\";\n";

        for (unsigned long objId=0; objId<model.numObjs(); ++objId)
        {
            string texFname = model.getTextureFilename(objId);
            if (texFname.length() == 0)
                texFname = "";

            const vector<FgVect3F> &mVtxList = 
                (*morphTargets)[mm].getPtList(objId);
            const vector<FgVect3F> &vtxList = model.getPtList(objId);
            const vector<FgVect3UI> &triList = model.getTriList(objId);
            const vector<FgVect4UI> &quadList = model.getQuadList(objId);
            const vector<FgVect2F> &texCoord = model.getTextCoord(objId);
            const vector<FgVect3UI> &texTriList = model.getTexTriList(objId);
            const vector<FgVect4UI> &texQuadList=model.getTexQuadList(objId);

            size_t  numFacets = triList.size() + quadList.size();

            string morphName = 
                getObjMorphName(model,objId,(*morphNames)[mm]);
            string morphShapeName = 
                getObjMorphShapeName(model,objId,(*morphNames)[mm]);

            ofs << "createNode transform -n \"" << morphName << "\" "
                        << "-p \"" << morphGrp << "\";\n";
            ofs << "\tsetAttr \".v\" no;\n";
            ofs << "createNode mesh -n \"" << morphShapeName << "\" "
                        << "-p \"" << morphName << "\";\n";
            ofs << "\tsetAttr -k off \".v\";\n";

            if (texFname != "")
            {
                ofs << "\tsetAttr -s 2 \".iog[0].og\";\n";
                ofs << "\tsetAttr \".iog[0].og[0].gcl\" "
                            << "-type \"componentList\" 0;\n";
                ofs << "\tsetAttr \".iog[0].og[1].gcl\" "
                            << "-type \"componentList\" 1 "
                            << "\"f[0:" << numFacets-1 << "]\";\n";
            }

            ofs << "\tsetAttr \".uvst[0].uvsn\" -type \"string\" \"map1\";\n"; 
            ofs << "\tsetAttr \".cuvs\" -type \"string\" \"map1\";\n";

            // Morph vertices
            writeVertices(ofs,mVtxList);

            // Other info.
            writeEdges(ofs,edgeList[objId]);
            writeFacets(ofs,vtxList,triList,quadList,edgeMap[objId],
                        texCoord,texTriList,texQuadList);
        }
    }
}


//****************************************************************************
//                          writeShadingMaterialPhong
//****************************************************************************
static int writeShadingMaterialPhong(

    ofstream                    &ofs,
    const FffMultiObjectC       &model,
    const vector<string>        *morphNames)
{
    size_t  numObjWithTextureFile = 0;
    size_t  totalNumTargets = 0;
    if (morphNames)
    {
        totalNumTargets = morphNames->size();
    }

    for (unsigned long mm=0; mm<model.numObjs(); ++mm)
    {
        string texFname = model.getTextureFilename(mm);
        if (texFname.length() == 0 || texFname == "") continue;

        ++numObjWithTextureFile;

        string shading = getObjectShadingEngName(model,mm);
        string material = getObjectMaterialName(model,mm);
        string phong = getObjectPhongName(model,mm);
        string shade = getObjectShadeGrpIdName(model,mm);
        string node = getObjectNodeGrpIdName(model,mm);
        string objTexName = getObjectTexFileName(model,mm);
        string objTexPlaceName = getObjectTexPlaceName(model,mm);

        ofs << "createNode shadingEngine -n \"" << shading << "\";\n";
        ofs << "\tsetAttr \".ihi\" 0;\n";
        ofs << "\tsetAttr \".ro\" yes;\n";
        ofs << "createNode materialInfo -n \"" << material << "\";\n";
        ofs << "createNode phong -n \"" << phong << "\";\n";
        ofs << "createNode file -n \"" << objTexName << "\";\n";
        ofs << "\tsetAttr \".ftn\" -type \"string\" \"" << texFname << "\";\n";
        ofs << "createNode place2dTexture -n \"" << objTexPlaceName << "\";\n";

        // Now the morph objects
        for (unsigned long tt=0; tt<totalNumTargets; ++tt)
        {
            string mphName = (*morphNames)[tt];

            string shadingEngine = getObjMorphShadingEngName(model,mm,mphName);
            string materialName = getObjMorphMaterialName(model,mm,mphName);
            string phongName = getObjMorphPhongName(model,mm,mphName);
            string shadeName = getObjMorphShadeGrpIdName(model,mm,mphName);
            string nodeName = getObjMorphNodeGrpIdName(model,mm,mphName);

            ofs << "createNode shadingEngine -n \"" << shadingEngine << "\";\n";
            ofs << "\tsetAttr \".ihi\" 0;\n";
            ofs << "\tsetAttr \".ro\" yes;\n";
            ofs << "createNode materialInfo -n \"" << materialName << "\";\n";
            ofs << "createNode groupId -n \"" << shadeName << "\";\n";
            ofs << "\tsetAttr \".ihi\" 0;\n";
            ofs << "createNode groupId -n \"" << nodeName << "\";\n";
            ofs << "\tsetAttr \".ihi\" 0;\n";
            ofs << "createNode phong -n \"" << phongName << "\";\n";
        }
    }

    return int(numObjWithTextureFile);
}


//****************************************************************************
//                              writeBlendShapes
//****************************************************************************
static void writeBlendShapes(

    ofstream                    &ofs, 
    const FffMultiObjectC       &model,
    const vector<string>        *morphNames)
{
    size_t      totalNumMorphs = 0;
    if (morphNames)
        totalNumMorphs = morphNames->size();
    if (totalNumMorphs == 0) return;

    unsigned long numObjsWithMorph = model.numObjs();

    ofs << "createNode blendShape -n \"" << getBlendShapeName() << "\";\n";
    ofs << "\taddAttr -ci true -sn \"aal\" "
                "-ln \"attributeAliasList\" "
                "-bt \"ATAL\" "
                "-dt \"attributeAlias\";\n";
    ofs << "\tsetAttr -s " << numObjsWithMorph << " \".ip\";\n";
    ofs << "\tsetAttr -s " << numObjsWithMorph << " \".og\";\n";
    ofs << "\tsetAttr -s " << totalNumMorphs 
                           << " \".w[0:" << totalNumMorphs-1 << "]\" \n";
    ofs << "\t\t";
    unsigned long xx;
    for (xx=0; xx<totalNumMorphs; ++xx)
        ofs << " 0";
    ofs << ";\n";
    ofs << "\tsetAttr -s " << numObjsWithMorph << " \".it\";\n";
    for (xx=0; xx<numObjsWithMorph; ++xx)
    {
        ofs << "\tsetAttr -s " << totalNumMorphs 
                               << " \".it[" << xx << "].itg\";\n";
    }
    ofs << "\tsetAttr \".aal\" -type \"attributeAlias\" {\n";
    for (xx=0; xx<totalNumMorphs; ++xx)
    {
        string morphName = getMorphName((*morphNames)[xx]);
        ofs << "\t\t\"" << morphName << "\",\"weight[" << xx << "]\"";
        if (xx == totalNumMorphs-1)
            ofs << "};\n";
        else
            ofs << ",\n";
    }
    unsigned long mm;
    for (mm=0; mm<model.numObjs(); ++mm)
    {
        string texFname = model.getTextureFilename(mm);

        if (texFname.length() > 0 && texFname != "")
        {
            string grpId = getObjBlendTexGrpIdName(model,mm);
            string grpParts = getObjBlendTexGrpPartsName(model,mm);

            unsigned long numFacets = model.numTriangles(mm) 
                                    + model.numQuads(mm);

            ofs << "createNode groupId -n \"" << grpId << "\";\n";
            ofs << "\tsetAttr \".ihi\" 0;\n";
            ofs << "createNode groupParts -n \"" << grpParts << "\";\n";
            ofs << "\tsetAttr \".ihi\" 0;\n";
            ofs << "\tsetAttr \".ic\" -type \"componentList\" 1 \"f[0:" 
                    << numFacets-1 << "]\";\n";
        }
        ofs << "createNode tweak -n \"" 
                    << getObjTweakName(model,mm) << "\";\n";
    }

    ofs << "createNode objectSet "
                << "-n \"" << getBlendShapeSetName() << "\";\n";
    ofs << "\tsetAttr \".ihi\" 0;\n";
    ofs << "\tsetAttr -s " << numObjsWithMorph << " \".dsm\";\n";
    ofs << "\tsetAttr \".vo\" yes;\n";
    ofs << "\tsetAttr -s " << numObjsWithMorph << " \".gn\";\n";

    for (mm=0; mm<model.numObjs(); ++mm)
    {
        ofs << "createNode groupId " 
                << "-n \"" << getObjBlendShapeGrpIdName(model,mm) << "\";\n";
        ofs << "\tsetAttr \".ihi\" 0;\n";
        ofs << "createNode groupParts " 
                << "-n \"" << getObjBlendShapeGrpPartsName(model,mm) << "\";\n";
        ofs << "\tsetAttr \".ihi\" 0;\n";
        ofs << "\tsetAttr \".ic\" -type \"componentList\" 1 \"vtx[*]\";\n";
    }

    for (mm=0; mm<model.numObjs(); ++mm)
    {
        ofs << "createNode objectSet " 
                << "-n \"" << getObjTweakSetName(model,mm) << "\";\n";
        ofs << "\tsetAttr \".ihi\" 0;\n";
        ofs << "\tsetAttr \".vo\" yes;\n";

        ofs << "createNode groupId " 
                << "-n \"" << getObjTweakGrpIdName(model,mm) << "\";\n";
        ofs << "\tsetAttr \".ihi\" 0;\n";

        ofs << "createNode groupParts " 
                << "-n \"" << getObjTweakGrpPartsName(model,mm) << "\";\n";
        ofs << "\tsetAttr \".ihi\" 0;\n";
        ofs << "\tsetAttr \".ic\" -type \"componentList\" 1 \"vtx[*]\";\n";
    }
}


//****************************************************************************
//                              writePolySoftEdge
//****************************************************************************
static void writePolySoftEdge(

    ofstream                    &ofs, 
    const FffMultiObjectC       &model,
    const vector<string>        *morphNames,
    const vector<int>           &edgeSizeList)
{
    size_t      totalNumMorphs = 0;
    if (morphNames)
        totalNumMorphs = morphNames->size();

    for (unsigned long mm=0; mm<model.numObjs(); ++mm)
    {
        string polySoftEdgeName = getObjPolySoftEdgeName(model,mm);

        ofs << "createNode polySoftEdge -n \"" << polySoftEdgeName << "\";\n";
        ofs << "\tsetAttr \".uopa\" yes;\n";
        ofs << "\tsetAttr \".ics\" -type \"componentList\" 1 "
                    << "\"e[0:" << edgeSizeList[mm]-1 << "]\";\n";
        ofs << "\tsetAttr \".ix\" -type \"matrix\" "
                    "1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1;\n";

        string texFname = model.getTextureFilename(mm);
        if (texFname.length() > 0 && texFname != "" && totalNumMorphs == 0)
        {
            string grpIdName = getObjPolySoftEdgeGrpId(model,mm);
            string grpPartsName = getObjPolySoftEdgeGrpParts(model,mm);
            size_t  numFacets = model.getTriList(mm).size()
                          + model.getQuadList(mm).size();

            ofs << "createNode groupId -n \"" << grpIdName << "\";\n";
            ofs << "\tsetAttr \".ihi\" 0;\n";
            ofs << "createNode groupParts -n \"" << grpPartsName << "\";\n";
            ofs << "\tsetAttr \".ihi\" 0;\n";
            ofs << "\tsetAttr \".ic\" -type \"componentList\" 1 "
                        << "\"f[0:" << numFacets-1 << "]\";\n";
        }
    }
}


//****************************************************************************
//                              connectAttributes
//****************************************************************************
static void connectAttributes(

    ofstream                        &ofs,
    const FffMultiObjectC           &model,
    const vector<string>            *morphNames)
{
    //
    // First make the attribute connections for the shading engine groups 
    // and the blending shape groups for each object.
    //
    size_t  totalNumTargets = 0;
    if (morphNames)
        totalNumTargets = morphNames->size();
    unsigned long mm;
    for (mm=0; mm<model.numObjs(); ++mm)
    {
        string texFname = model.getTextureFilename(mm);
        bool hasTexture = (texFname.length() > 0 && texFname != "");

        if (totalNumTargets == 0)
        {
            string shapeName = getObjectShapeName(model,mm);

            if (hasTexture)
            {
                ofs << "connectAttr \""
                        << getObjPolySoftEdgeGrpId(model,mm) << ".id\" \"" 
                        << shapeName << ".iog.og[2].gid\";\n";
                ofs << "connectAttr \""
                        << getObjectShadingEngName(model,mm) << ".mwc\" \"" 
                        << shapeName << ".iog.og[2].gco\";\n";
            }
            ofs << "connectAttr \""
                    << getObjPolySoftEdgeName(model,mm) << ".out\" \"" 
                    << shapeName << ".i\";\n";
        }
        else
        {
            string objShapeName = getObjectShapeName(model,mm);

            int bIdx = 0;
            int tIdx = 1;
            if (hasTexture)
            {
                bIdx = 3;
                tIdx = 4;

                ofs << "connectAttr \""
                        << getObjBlendTexGrpIdName(model,mm) << ".id\" \""
                        << objShapeName << ".iog.og[2].gid\";\n";
                ofs << "connectAttr \""
                        << getObjectShadingEngName(model,mm) << ".mwc\" \""
                        << objShapeName << ".iog.og[2].gco\";\n";
            }

            ofs << "connectAttr \"" 
                        << getObjBlendShapeGrpIdName(model,mm) << ".id\" \""
                        << objShapeName << ".iog.og[" << bIdx << "].gid\";\n";
            ofs << "connectAttr \""
                        << getBlendShapeSetName() << ".mwc\" \""
                        << objShapeName << ".iog.og[" << bIdx << "].gco\";\n";
            ofs << "connectAttr \"" 
                        << getObjTweakGrpIdName(model,mm) << ".id\" \""
                        << objShapeName << ".iog.og[" << tIdx << "].gid\";\n";
            ofs << "connectAttr \""
                        << getObjTweakSetName(model,mm) << ".mwc\" \""
                        << objShapeName << ".iog.og[" << tIdx << "].gco\";\n";
            ofs << "connectAttr \""
                        << getObjPolySoftEdgeName(model,mm) << ".out\" \""
                        << objShapeName << ".i\";\n";
            ofs << "connectAttr \""
                        << getObjTweakName(model,mm) << ".vl[0].vt[0]\" \""
                        << objShapeName << ".twl\";\n";
        }
    }

    for (unsigned long nt=0; nt<totalNumTargets; ++nt)
    {
        string mphName = (*morphNames)[nt];

        for (unsigned long nn=0; nn<model.numObjs(); ++nn)
        {
            string texFname = model.getTextureFilename(nn);
            if (texFname.length() == 0 || texFname == "")
                continue;

            string mphShapeName = getObjMorphShapeName(model,nn,mphName);
            string mphGrpId = getObjMorphNodeGrpIdName(model,nn,mphName);
            string mphShadeEng = getObjMorphShadingEngName(model,nn,mphName);
            string mphShadeGrpId = getObjMorphShadeGrpIdName(model,nn,mphName);

            ofs << "connectAttr \""
                    << mphGrpId << ".id\" \"" 
                    << mphShapeName << ".iog.og[1].gid\";\n";
            ofs << "connectAttr \""
                    << mphShadeEng << ".mwc\" \"" 
                    << mphShapeName << ".iog.og[1].gco\";\n";
            ofs << "connectAttr \""
                    << mphShadeGrpId << ".id\" \"" 
                    << mphShapeName << ".ciog.cog[0].cgid\";\n";
        }
    }

    //
    // Make attribute connections to default light linker.
    //
    ofs << "connectAttr \":defaultLightSet.msg\" "
                << "\"lightLinker1.lnk[0].llnk\";\n";
    ofs << "connectAttr \":initialShadingGroup.msg\" "
                << "\"lightLinker1.lnk[0].olnk\";\n";
    ofs << "connectAttr \":defaultLightSet.msg\" "
                << "\"lightLinker1.lnk[1].llnk\";\n";
    ofs << "connectAttr \":initialParticleSE.msg\" "
                << "\"lightLinker1.lnk[1].olnk\";\n";

    int lnkIdx = 2;
    for (mm=0; mm<model.numObjs(); ++mm)
    {
        string texFname = model.getTextureFilename(mm);
        if (texFname.length() == 0 || texFname == "") continue;

        ofs << "connectAttr \":defaultLightSet.msg\" "
                    << "\"lightLinker1.lnk[" << lnkIdx << "].llnk\";\n";
        ofs << "connectAttr \"" 
                    << getObjectShadingEngName(model,mm) << ".msg\" "
                    << "\"lightLinker1.lnk[" << lnkIdx << "].olnk\";\n";

        ++lnkIdx;

        for (unsigned long tt=0; tt<totalNumTargets; ++tt)
        {
            string mphName = (*morphNames)[tt];

            ofs << "connectAttr \":defaultLightSet.msg\" "
                << "\"lightLinker1.lnk[" << lnkIdx << "].llnk\";\n";
            ofs << "connectAttr \"" 
                << getObjMorphShadingEngName(model,mm,mphName) << ".msg\" "
                << "\"lightLinker1.lnk[" << lnkIdx << "].olnk\";\n";

            ++lnkIdx;
        }
    }

    ofs << "connectAttr \"layerManager.dli[0]\" \"defaultLayer.id\";\n";
    ofs << "connectAttr \"renderLayerManager.rlmi[0]\" "
                    << "\"defaultRenderLayer.rlid\";\n";

    //
    // Connect attributes for the textures
    //
    for (mm=0; mm<model.numObjs(); ++mm)
    {
        string texFname = model.getTextureFilename(mm);
        if (texFname.length() == 0 || texFname == "") continue;

        string objPhong = getObjectPhongName(model,mm);
        string objShadingEng = getObjectShadingEngName(model,mm);
        string objTexFile = getObjectTexFileName(model,mm);
        string texPlacement = getObjectTexPlaceName(model,mm);

        ofs << "connectAttr \"" << objPhong << ".oc\" \""
                                << objShadingEng << ".ss\";\n";
        if (!totalNumTargets)
        {
            ofs << "connectAttr \"" 
                    << getObjPolySoftEdgeGrpId(model,mm) << ".msg\" \""
                    << objShadingEng << ".gn\" -na;\n";
        }
        else
        {
            ofs << "connectAttr \""
                    << getObjBlendTexGrpIdName(model,mm) << ".msg\" \""
                    << objShadingEng << ".gn\" -na;\n";
        }
        ofs << "connectAttr \""
                << getObjectShapeName(model,mm) 
                    << ".iog.og[2]\" \""
                << objShadingEng << ".dsm\" -na;\n";
        ofs << "connectAttr \""
                << objShadingEng << ".msg\" \""
                << getObjectMaterialName(model,mm) << ".sg\";\n";
        ofs << "connectAttr \"" 
                << objTexFile << ".oc\" \"" 
                << objPhong << ".c\";\n";
        ofs << "connectAttr \"" << texPlacement << ".c\" \""
                                << objTexFile << ".c\";\n";
        ofs << "connectAttr \"" << texPlacement << ".tf\" \""
                                << objTexFile << ".tf\";\n";
        ofs << "connectAttr \"" << texPlacement << ".rf\" \""
                                << objTexFile << ".rf\";\n";
        ofs << "connectAttr \"" << texPlacement << ".m\" \""
                                << objTexFile << ".m\";\n";
        ofs << "connectAttr \"" << texPlacement << ".s\" \""
                                << objTexFile << ".s\";\n";
        ofs << "connectAttr \"" << texPlacement << ".wu\" \""
                                << objTexFile << ".wu\";\n";
        ofs << "connectAttr \"" << texPlacement << ".wv\" \""
                                << objTexFile << ".wv\";\n";
        ofs << "connectAttr \"" << texPlacement << ".re\" \""
                                << objTexFile << ".re\";\n";
        ofs << "connectAttr \"" << texPlacement << ".of\" \""
                                << objTexFile << ".of\";\n";
        ofs << "connectAttr \"" << texPlacement << ".r\" \""
                                << objTexFile << ".ro\";\n";
        ofs << "connectAttr \"" << texPlacement << ".n\" \""
                                << objTexFile << ".n\";\n";
        ofs << "connectAttr \"" << texPlacement << ".o\" \""
                                << objTexFile << ".uv\";\n";
        ofs << "connectAttr \"" << texPlacement << ".ofs\" \""
                                << objTexFile << ".fs\";\n";

        for (unsigned long tt=0; tt<totalNumTargets; ++tt)
        {
            string mphName = (*morphNames)[tt];

            string objPhongName = getObjMorphPhongName(model,mm,mphName);
            string objShadingEngName = getObjMorphShadingEngName(model,mm,mphName);

            ofs << "connectAttr \""
                << objPhongName << ".oc\" \""
                << objShadingEngName << ".ss\";\n";
            ofs << "connectAttr \""
                << getObjMorphNodeGrpIdName(model,mm,mphName) << ".msg\" \""
                << objShadingEngName << ".gn\" -na;\n";
            ofs << "connectAttr \""
                << getObjMorphShapeName(model,mm,mphName) << ".iog.og[1]\" \""
                << objShadingEngName << ".dsm\" -na;\n";
            ofs << "connectAttr \""
                << objShadingEngName << ".msg\" \""
                << getObjMorphMaterialName(model,mm,mphName) << ".sg\";\n";
        }
    }

    //
    // Connection for the blend shapes
    //
    string blendShapeName = getBlendShapeName();
    for (mm=0; mm<model.numObjs(); ++mm)
    {
        if (totalNumTargets == 0)
            continue;

        string blendShapeGrpIdName = getObjBlendShapeGrpIdName(model,mm);
        string blendShapeGrpPartsName = getObjBlendShapeGrpPartsName(model,mm);

        ofs << "connectAttr \""
                    << blendShapeGrpPartsName << ".og\" \""
                    << blendShapeName << ".ip[" << mm << "].ig\";\n";
        ofs << "connectAttr \""
                    << blendShapeGrpIdName << ".id\" \""
                    << blendShapeName << ".ip[" << mm << "].gi\";\n";
    }

    for (mm=0; mm<model.numObjs(); ++mm)
    {
        for (unsigned long tt=0; tt<totalNumTargets; ++tt)
        {
            string mphName = (*morphNames)[tt];

            ofs << "connectAttr \""
                << getObjMorphShapeName(model,mm,mphName) << ".w\" \""
                << blendShapeName 
                << ".it[" << mm << "].itg[" << tt << "].iti[6000].igt\";\n";
        }
    }

    for (mm=0; mm<model.numObjs(); ++mm)
    {
        if (totalNumTargets == 0)
            continue;

        string texFname = model.getTextureFilename(mm);
        if (texFname.length() > 0 && texFname != "")
        {
            ofs << "connectAttr \""
                    << getObjectShapeOrigName(model,mm) << ".w\" \""
                    << getObjBlendTexGrpPartsName(model,mm) <<".ig\";\n";
            ofs << "connectAttr \""
                    << getObjBlendTexGrpIdName(model,mm) << ".id\" \""
                    << getObjBlendTexGrpPartsName(model,mm) << ".gi\";\n";
        }

        string tweakName = getObjTweakName(model,mm);
        string tweakGrpIdName = getObjTweakGrpIdName(model,mm);
        string tweakGrpPartsName = getObjTweakGrpPartsName(model,mm);

        ofs << "connectAttr \""
                    << tweakGrpPartsName << ".og\" \""
                    << tweakName << ".ip[0].ig\";\n";
        ofs << "connectAttr \""
                    << tweakGrpIdName << ".id\" \""
                    << tweakName << ".ip[0].gi\";\n";
    }

    string blendShapeSetName = getBlendShapeSetName();
    for (mm=0; mm<model.numObjs(); ++mm)
    {
        if (totalNumTargets == 0)
            continue;

        string blendShapeGrpIdName = getObjBlendShapeGrpIdName(model,mm);
        ofs << "connectAttr \""
                    << blendShapeGrpIdName << ".msg\" \""
                    << blendShapeSetName << ".gn\" " << "-na;\n";
    }

    for (mm=0; mm<model.numObjs(); ++mm)
    {
        if (totalNumTargets == 0)
            continue;

        string texFname = model.getTextureFilename(mm);

        int bIdx = 0;
        if (texFname.length() > 0 && texFname != "")
            bIdx = 3;
        ofs << "connectAttr \""
                    << getObjectShapeName(model,mm) << ".iog.og[" 
                        << bIdx << "]\" \""
                    << blendShapeSetName << ".dsm\" -na;\n";
    }

    if (totalNumTargets != 0)
    {
        ofs << "connectAttr \""
                    << blendShapeName << ".msg\" \""
                    << blendShapeSetName << ".ub[0]\";\n";
    }

    for (mm=0; mm<model.numObjs(); ++mm)
    {
        if (totalNumTargets == 0)
            continue;

        string tweakName = getObjTweakName(model,mm);
        string blendShapeGrpIdName = getObjBlendShapeGrpIdName(model,mm);
        string blendShapeGrpPartsName = getObjBlendShapeGrpPartsName(model,mm);

        ofs << "connectAttr \""
                    << tweakName << ".og[0]\" \""
                    << blendShapeGrpPartsName << ".ig\";\n";
        ofs << "connectAttr \""
                    << blendShapeGrpIdName << ".id\" \""
                    << blendShapeGrpPartsName << ".gi\";\n";
    }

    for (mm=0; mm<model.numObjs(); ++mm)
    {
        if (totalNumTargets == 0)
            continue;

        string texFname = model.getTextureFilename(mm);

        int tIdx = 1;
        if (texFname.length() > 0 && texFname != "")
            tIdx = 4;

        string tweakName = getObjTweakName(model,mm);
        string tweakGrpIdName = getObjTweakGrpIdName(model,mm);
        string tweakGrpPartsName = getObjTweakGrpPartsName(model,mm);
        string tweakSetName = getObjTweakSetName(model,mm);

        ofs << "connectAttr \""
                    << tweakGrpIdName << ".msg\" \""
                    << tweakSetName << ".gn\" -na;\n";
        ofs << "connectAttr \""
                    << getObjectShapeName(model,mm) << ".iog.og[" 
                        << tIdx << "]\" \""
                    << tweakSetName << ".dsm\" -na;\n";
        ofs << "connectAttr \""
                    << tweakName << ".msg\" \""
                    << tweakSetName << ".ub[0]\";\n";

        if (texFname.length() == 0 || texFname == "")
        {
            ofs << "connectAttr \""
                    << getObjectShapeOrigName(model,mm) << ".w\" \""
                    << tweakGrpPartsName << ".ig\";\n";
        }
        else
        {
            ofs << "connectAttr \""
                    << getObjBlendTexGrpPartsName(model,mm) << ".og\" \""
                    << tweakGrpPartsName << ".ig\";\n";
        }
        ofs << "connectAttr \""
                    << tweakGrpIdName << ".id\" \""
                    << tweakGrpPartsName << ".gi\";\n";
    }

    //
    // PolySoftEdge Connection
    //
    for (mm=0; mm<model.numObjs(); ++mm)
    {
        string texFname = model.getTextureFilename(mm);
        bool hasTexture = (texFname.length() > 0 && texFname != "");

        string polySoftEdgeName = getObjPolySoftEdgeName(model,mm);

        if (totalNumTargets)
        {
            ofs << "connectAttr \""
                    << getBlendShapeName() << ".og[" << mm << "]\" \""
                    << polySoftEdgeName << ".ip\";\n";
            ofs << "connectAttr \""
                    << getObjectShapeName(model,mm) << ".wm\" \""
                    << polySoftEdgeName << ".mp\";\n";
        }
        else
        {
            if (hasTexture)
            {
                string grpParts = getObjPolySoftEdgeGrpParts(model,mm);

                ofs << "connectAttr \""
                        << grpParts << ".og\" \""
                        << polySoftEdgeName << ".ip\";\n";
                ofs << "connectAttr \""
                        << getObjectShapeName(model,mm) << ".wm\" \""
                        << polySoftEdgeName << ".mp\";\n";
                ofs << "connectAttr \""
                        << getObjPolySurfName(model,mm) << ".o\" \""
                        << grpParts << ".ig\";\n";
                ofs << "connectAttr \""
                        << getObjPolySoftEdgeGrpId(model,mm) << ".id\" \""
                        << grpParts << ".gi\";\n";
            }
            else
            {
                ofs << "connectAttr \""
                        << getObjPolySurfName(model,mm) << ".o\" \""
                        << polySoftEdgeName << ".ip\";\n";
                ofs << "connectAttr \""
                        << getObjectShapeName(model,mm) << ".wm\" \""
                        << polySoftEdgeName << ".mp\";\n";
            }
        }        
    }

    //
    // Shading engine connection
    //
    for (mm=0; mm<model.numObjs(); ++mm)
    {
        string texFname = model.getTextureFilename(mm);
        if (texFname.length() == 0 || texFname == "") continue;

        ofs << "connectAttr \""
                << getObjectShadingEngName(model,mm) << ".pa\" \""
                << ":renderPartition.st\" -na;\n";

        for (unsigned long tt=0; tt<totalNumTargets; ++tt)
        {
            string mphName = (*morphNames)[tt];

            ofs << "connectAttr \""
                    << getObjMorphShadingEngName(model,mm,mphName) 
                    << ".pa\" \":renderPartition.st\" -na;\n";
        }
    }

    //
    // Phong connection
    //
    for (mm=0; mm<model.numObjs(); ++mm)
    {
        string texFname = model.getTextureFilename(mm);
        if (texFname.length() == 0 || texFname == "") continue;

        ofs << "connectAttr \""
                << getObjectPhongName(model,mm) << ".msg\" \""
                << ":defaultShaderList1.s\" -na;\n";

        for (unsigned long tt=0; tt<totalNumTargets; ++tt)
        {
            string mphName = (*morphNames)[tt];

            ofs << "connectAttr \""
                    << getObjMorphPhongName(model,mm,mphName) << ".msg\" \""
                    << ":defaultShaderList1.s\" -na;\n";
        }
    }

    //
    // Texture placement
    //
    for (mm=0; mm<model.numObjs(); ++mm)
    {
        string texFname = model.getTextureFilename(mm);
        if (texFname.length() == 0 || texFname == "") continue;

        ofs << "connectAttr \""
                << getObjectTexPlaceName(model,mm) << ".msg\" \""
                << ":defaultRenderUtilityList1.u\" -na;\n";
    }

    ofs << "connectAttr \"lightLinker1.msg\" \":lightList1.ln\" -na;\n";

    //
    // Connection to default texture list
    //
    for (mm=0; mm<model.numObjs(); ++mm)
    {
        string texFname = model.getTextureFilename(mm);
        if (texFname.length() == 0 || texFname == "") continue;

        ofs << "connectAttr \""
                << getObjectTexFileName(model,mm) << ".msg\" \""
                << ":defaultTextureList1.tx\" -na;\n";
    }

    //
    // Connection to initial shading group
    //    
    for (mm=0; mm<model.numObjs(); ++mm)
    {
        string texFname = model.getTextureFilename(mm);
        if (texFname.length() > 0 && texFname != "")
        {
            for (unsigned long tt=0; tt<totalNumTargets; ++tt)
            {
                string mphName = (*morphNames)[tt];

                ofs << "connectAttr \""
                        << getObjMorphShapeName(model,mm,mphName) 
                        << ".ciog.cog[0]\" "
                        << "\":initialShadingGroup.dsm\" -na;\n";
            }
        }
        else
        {
            ofs << "connectAttr \""
                        << getObjectShapeName(model,mm) << ".iog\" "
                        << "\":initialShadingGroup.dsm\" -na;\n";

            for (unsigned long tt=0; tt<totalNumTargets; ++tt)
            {
                string mphName = (*morphNames)[tt];
                ofs << "connectAttr \""
                        << getObjMorphShapeName(model,mm,mphName) << ".iog\" "
                        << "\":initialShadingGroup.dsm\" -na;\n";
            }
        }
    }
    for (mm=0; mm<model.numObjs(); ++mm)
    {
        string texFname = model.getTextureFilename(mm);
        if (texFname.length() == 0 || texFname == "") continue;

        for (unsigned long tt=0; tt<totalNumTargets; ++tt)
        {
            string mphName = (*morphNames)[tt];

            ofs << "connectAttr \""
                    << getObjMorphShadeGrpIdName(model,mm,mphName)
                    << ".msg\" "
                    << "\":initialShadingGroup.gn\" -na;\n";
        }
    }
}

FgMeshLegacy
fgMeshLegacy(const vector<Fg3dMesh> & meshes,const FgString & fname,const string & imgFormat,uint maxLen)
{
    FgMeshLegacy                    ret;
    FgPath                          path(fname);
    // Index from legacy mesh to 'meshes' since latter can have multiple tex images per mesh:
    vector<size_t>                  meshesInds;
    size_t                          imgIdx = 0;
    for (size_t ii=0; ii<meshes.size(); ++ii) {
        const Fg3dMesh &            mesh = meshes[ii];
        FffMultiObjectC::objData    od;
        od.ptList = mesh.verts;
        od.textCoord = mesh.uvs;
        od.modelName = mesh.name.as_ascii();
        if (mesh.surfaces.empty()) {
            ret.base.m_objs.push_back(od);
            meshesInds.push_back(ii);
        }
        else {
            for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
                const Fg3dSurface &         surf = mesh.surfaces[ss];
                string                      texBase;
                if (surf.albedoMap) {
                    string                  baseName = path.base.as_ascii();
                    if (maxLen > 0)
                        if (baseName.length() > maxLen)
                            baseName.resize(maxLen);
                    texBase = baseName + fgToString(imgIdx++) + "." + imgFormat;
                    fgSaveImgAnyFormat(path.dir()+texBase,*surf.albedoMap);
                }
                od.triList = surf.tris.vertInds;
                od.quadList = surf.quads.vertInds;
                od.texTriList = surf.tris.uvInds;
                od.texQuadList = surf.quads.uvInds;
                od.textureFile = texBase;
                ret.base.m_objs.push_back(od);
                meshesInds.push_back(ii);
            }
        }
    }
    set<FgString>           morphSet = fgMorphs(meshes);
    FgStrings        morphs(morphSet.begin(),morphSet.end());
    ret.morphs.resize(morphs.size(),ret.base);
    for (size_t mm=0; mm<morphs.size(); ++mm) {
        vector<FffMultiObjectC::objData> &  ods = ret.morphs[mm].m_objs;
        for (size_t ii=0; ii<ods.size(); ++ii) {
            const Fg3dMesh &    mesh = meshes[meshesInds[ii]];
            FgValid<size_t>     morphIdx = mesh.findMorph(morphs[mm]);
            if (morphIdx.valid())
                ods[ii].ptList = mesh.morphSingle(mesh.findMorph(morphs[mm]).val());
        }
        ret.morphNames.push_back(morphs[mm].m_str);
    }
    return ret;
}

void
fgSaveMa(
    const FgString &        fname,
    const vector<Fg3dMesh> & meshes,
    string                  imgFormat)
{
    FgMeshLegacy            leg = fgMeshLegacy(meshes,fname,imgFormat);
    vector<string>          cmnts;
    if (leg.morphs.empty())
        fffSaveMayaAsciiFile(fname,leg.base,&cmnts);
    else
        fffSaveMayaAsciiFile(fname,leg.base,leg.morphs,leg.morphNames,&cmnts);
}

void
fgSaveMaTest(const FgArgs & args)
{
    FGTESTDIR
    FgString    dd = fgDataDir();
    string      rd = "base/";
    Fg3dMesh    mesh = fgLoadTri(dd+rd+"Mouth"+".tri");
    mesh.surfaces[0].setAlbedoMap(fgLoadImgAnyFormat(dd+rd+"Mouth.tga"));
    fgSaveMa("meshExportMa",fgSvec(mesh));
    fgRegressFile("meshExportMa.ma","base/test/");
    fgRegressFile("meshExportMa0.png","base/test/");
}

// */
