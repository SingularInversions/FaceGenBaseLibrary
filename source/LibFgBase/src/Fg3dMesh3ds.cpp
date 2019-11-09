//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

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

#define     M3D_VERSION     0x0002
#define     MDATA           0x3d3d
#define     COLOUR_RGB_BYTE 0x0011
#define     MAT_ENTRY       0xafff
#define     MAT_NAME        0xa000
#define     MAT_AMBIENT     0xa010
#define     MAT_DIFFUSE     0xa020
#define     MAT_SPECULAR    0xa030
#define     MAT_TEXMAP      0xa200
#define     MAT_MAP_FNAME   0xa300
#define     MAT_MAP_USCALE  0xa354
#define     MAT_MAP_VSCALE  0xa356
#define     MAT_MAP_UOFFSET 0xa358
#define     MAT_MAP_VOFFSET 0xa35a
#define     MAT_MAP_ANG     0xa35c
#define     OBJECT          0x4000
#define     TRI_OBJECT      0x4100
#define     TRI_POINT_ARRAY 0x4110
#define     TRI_FACE_ARRAY  0x4120
#define     TRI_MAT_GROUP   0x4130
#define     TRI_TEX_COORD   0x4140
#define     TRI_SMOOTH      0x4150
#define     TRI_TRANS       0x4160

namespace Fg {

static string fffMdlNameTo3dsName(const string & mdlName,vector<string> & nameList)
{
    string name = mdlName;
    if (name.size() > 10) {
        name = mdlName.substr(0,10);
        int doAgainCount=0;
        do {
            // Make sure that this name hasn't been used already.
            size_t  ii;
            for (ii=0; ii<nameList.size(); ++ii) {
                if (name == nameList[ii])
                    break;
            }
            if (ii != nameList.size()) {
                ++doAgainCount;
                char numStr[5];
                sprintf(numStr,"%d",doAgainCount);
                size_t len = strlen(numStr);
                name = name.substr(0,10-len) + string(numStr);
            }
            else
                doAgainCount = 51;
        }
        while (doAgainCount < 50);
    }
    nameList.push_back(name);
    return name;
}

static bool fffUpdateChunkSizeInfo_local(FILE *fptr,int newChunkSize,int chunkStartPos)
{
    int currPos = ftell(fptr);
    if (fseek(fptr,chunkStartPos+2,SEEK_SET))
        return false;
    if (fwrite(&newChunkSize,sizeof(int),1,fptr) != 1)
        return false;
    if (fseek(fptr,currPos,SEEK_SET))
        return false;
    return true;
}

static bool fffWriteChunkHeader_local(FILE *fptr,unsigned short id,int ln)
{
    if (fwrite(&id,sizeof(unsigned short),1,fptr) != 1)
        return false;
    if (fwrite(&ln,sizeof(int),1,fptr) != 1)
        return false;
    return true;
}

static bool fffWrite3dsString_local(FILE *fptr, const string &str)
{
    if (fwrite(str.c_str(),sizeof(char),str.size()+1,fptr) !=
            str.size()+1)
        return false;
    else
        return true;
}

static bool fffWriteTriObjectChunk_local(FILE *fptr,int &chunkSize,const map<string,string> &textureInfo,unsigned int objId,const FffMultiObjectC &model)
{
    int chunkStartPos = ftell(fptr);
    unsigned short id = TRI_OBJECT;
    chunkSize = 6;
    if (!fffWriteChunkHeader_local(fptr,id,chunkSize))
        return false;
    unsigned short numVtx = (unsigned short) model.numPoints(objId);
    if (numVtx != 0) {
        id = TRI_POINT_ARRAY;
        int ptChunkSize = 6+sizeof(unsigned short)+numVtx*sizeof(float)*3;
        if (!fffWriteChunkHeader_local(fptr,id,ptChunkSize))
            return false;
        if (fwrite(&numVtx,sizeof(unsigned short),1,fptr) != 1)
            return false;
        const vector<Vec3F> &ptList = model.getPtList(objId);
        for (unsigned short ii=0; ii<numVtx; ++ii) {
            if (fwrite(&ptList[ii][0],sizeof(float),1,fptr) != 1)
                return false;
            if (fwrite(&ptList[ii][1],sizeof(float),1,fptr) != 1)
                return false;
            if (fwrite(&ptList[ii][2],sizeof(float),1,fptr) != 1)
                return false;
        }
        chunkSize += ptChunkSize;
    }
    // Check if it is per vertex texture
    bool perVertexTexture = 
            (model.numPoints(objId) == model.numTxtCoord(objId) &&
             model.numTxtTris(objId) == 0 &&
             model.numTxtQuads(objId) == 0);
    if (!perVertexTexture) {
        if (model.numTriangles(objId) == model.numTxtTris(objId) && model.numQuads(objId) == model.numTxtQuads(objId)) {
            if (model.numPoints(objId) == model.numTxtCoord(objId)) {
                bool identical = true;
                // Check if the facet lists are identical
                const vector<Vec3UI> &triList = 
                        model.getTriList(objId);
                const vector<Vec3UI> &txtTriList = 
                        model.getTexTriList(objId);
                for (unsigned int tri=0; identical && tri<model.numTxtTris(objId); ++tri) {
                    if (triList[tri] != txtTriList[tri])
                        identical = false;
                }
                const vector<Vec4UI> &quadList =  model.getQuadList(objId);
                const vector<Vec4UI> &txtQuadList = model.getTexQuadList(objId);
                for (unsigned int quad=0; identical && quad<model.numTxtQuads(objId); ++quad) {
                    if (quadList[quad] != txtQuadList[quad])
                        identical = false;
                }
                perVertexTexture = identical;
                if (!identical)
                    fgout << "Skipping per-facet texture data\n";
            }
            else if (model.numTxtCoord(objId) != 0)
                fgout << "Skipping per-facet texture data\n";
        }
    }
    if (perVertexTexture && model.numTxtCoord(objId) != 0) {
        id = TRI_TEX_COORD;
        int ptChunkSize = 6+sizeof(unsigned short)+numVtx*sizeof(float)*2;
        if (!fffWriteChunkHeader_local(fptr,id,ptChunkSize))
            return false;
        if (fwrite(&numVtx,sizeof(unsigned short),1,fptr) != 1)
            return false;
        const vector<Vec2F> &ptList = model.getTextCoord(objId);
        for (unsigned short ii=0; ii<numVtx; ++ii) {
            if (fwrite(&ptList[ii][0],sizeof(float),1,fptr) != 1)
                return false;
            if (fwrite(&ptList[ii][1],sizeof(float),1,fptr) != 1)
                return false;
        }
        chunkSize += ptChunkSize;
    }
    // Now save facet info.
    unsigned short numTris = (unsigned short) model.numTriangles(objId);
    unsigned short numQuads = (unsigned short) model.numQuads(objId);
    unsigned short totalTris = numTris + numQuads*2;
    if (totalTris != 0) {
        int triChunkStartPos = ftell(fptr);
        // Facet (tris) info.
        unsigned short fourthNum = 7;   // The first 3 bits turned on.
        id = TRI_FACE_ARRAY;
        int smoothGrpChunkSize = totalTris*sizeof(unsigned int)+6;
        int triChunkSize = 6 + sizeof(unsigned short)
                          + totalTris*sizeof(unsigned short)*4
                          + smoothGrpChunkSize;
        if (!fffWriteChunkHeader_local(fptr,id,triChunkSize))
            return false;
        if (fwrite(&totalTris,sizeof(unsigned short),1,fptr) != 1)
            return false;
        const vector<Vec3UI> &triList = model.getTriList(objId);
        for (unsigned short ii=0; ii<numTris; ++ii) {
            unsigned short idx1 = (unsigned short) triList[ii][0];
            unsigned short idx2 = (unsigned short) triList[ii][1];
            unsigned short idx3 = (unsigned short) triList[ii][2];
            if (fwrite(&idx1,sizeof(unsigned short),1,fptr) != 1)
                return false;
            if (fwrite(&idx2,sizeof(unsigned short),1,fptr) != 1)
                return false;
            if (fwrite(&idx3,sizeof(unsigned short),1,fptr) != 1)
                return false;
            if (fwrite(&fourthNum,sizeof(unsigned short),1,fptr) != 1)
                return false;
        }
        const vector<Vec4UI> &quadList = model.getQuadList(objId);
        ushort ii;
        for (ii=0; ii<numQuads; ++ii) {
            for (int xx=0; xx<2; ++xx) {
                unsigned short idx1 = (unsigned short) quadList[ii][0];
                unsigned short idx2 = (unsigned short) quadList[ii][xx+1];
                unsigned short idx3 = (unsigned short) quadList[ii][xx+2];
                if (fwrite(&idx1,sizeof(unsigned short),1,fptr) != 1)
                    return false;
                if (fwrite(&idx2,sizeof(unsigned short),1,fptr) != 1)
                    return false;
                if (fwrite(&idx3,sizeof(unsigned short),1,fptr) != 1)
                    return false;
                if (fwrite(&fourthNum,sizeof(unsigned short),1,fptr) != 1)
                    return false;
            }
        }
        // Define smooth group (this whole surface is one smooth group)
        id = TRI_SMOOTH;
        if (!fffWriteChunkHeader_local(fptr,id,smoothGrpChunkSize))
            return false;
        unsigned int smoothGrp = objId+1;
        for (ii=0; ii<totalTris; ++ii) {
            if (fwrite(&smoothGrp,sizeof(unsigned int),1,fptr) != 1)
                return false;
        }
        // Texture mapping info.
        string textureFile = model.getTextureFilename(objId);
        map<string,string>::const_iterator mapItr = 
            textureInfo.find(textureFile);
        if (mapItr != textureInfo.end()) {
            id = TRI_MAT_GROUP;
            int triMatChunkSize = 6 + int(mapItr->second.size()) + 1
                                 + sizeof(unsigned short)*(totalTris+1);
            if (!fffWriteChunkHeader_local(fptr,id,triMatChunkSize))
                return false;
            if (!fffWrite3dsString_local(fptr,mapItr->second))
                return false;
            if (fwrite(&totalTris,sizeof(unsigned short),1,fptr) != 1)
                return false;
            vector<unsigned short> fList;
            fList.resize(totalTris);
            for (ii=0; ii<totalTris; ++ii)
                fList[ii] = ii;
            if (fwrite(&fList[0],sizeof(unsigned short),totalTris,fptr)
                    != totalTris)
                return false;
            triChunkSize += triMatChunkSize;
            if (!fffUpdateChunkSizeInfo_local(fptr,triChunkSize,
                    triChunkStartPos))
                return false;
        }
        chunkSize += triChunkSize;
    }
    // Correct the chunk size info for TRI_OBJECT chunk
    if (!fffUpdateChunkSizeInfo_local(fptr,chunkSize,chunkStartPos))
        return false;
    return true;
}

static bool fffWriteMdataChunk_local(FILE *fptr,int &chunkSize,const FffMultiObjectC &model)
{
    int chunkStartPos = ftell(fptr);
    unsigned short id = MDATA;
    chunkSize = 6;
    if (!fffWriteChunkHeader_local(fptr,id,chunkSize))
        return false;
    // Build Texture info
    map<string,string> textureInfoMap;  // Key = fname, map = mapName
    vector< string > mapNameList;
    for (unsigned int objId=0; objId < model.numObjs(); ++objId) {
        string textureFile = model.getTextureFilename(objId);
        if (textureFile != "") {
            // Make the filename in DOS 8.3 format
            Path  upath(textureFile);
            string path=upath.dir().m_str, root=upath.base.m_str, suff=upath.ext.m_str;
            if (root.size() > 8) {
                textureFile = path + root.substr(0,8) + "." + suff;
            }
            if (textureInfoMap.find(textureFile) == textureInfoMap.end()) {
                textureInfoMap[textureFile] = 
                    fffMdlNameTo3dsName(model.getModelName(objId),
                                        mapNameList);
            }
        }
    }

    for (map<string,string>::const_iterator itr = textureInfoMap.begin();
         itr != textureInfoMap.end(); ++itr)
    {
        int matEntryChunkStartPos = ftell(fptr);
        id = MAT_ENTRY;
        int matEntryChunkSize=6;
        if (!fffWriteChunkHeader_local(fptr,id,matEntryChunkSize))
            return false;

        id = MAT_NAME;

        int mapNameChunkSize = 6 + int(itr->second.size()) + 1;
        if (!fffWriteChunkHeader_local(fptr,id,mapNameChunkSize))
            return false;
        if (!fffWrite3dsString_local(fptr,itr->second))
            return false;

        matEntryChunkSize += mapNameChunkSize;

        int colourChunkSize = 6 + 3;

        id = MAT_AMBIENT;
        int mapAmbientColourChunkSize = 6 + colourChunkSize;
        if (!fffWriteChunkHeader_local(fptr,id,mapAmbientColourChunkSize))
            return false;
        id = COLOUR_RGB_BYTE;
        if (!fffWriteChunkHeader_local(fptr,id,colourChunkSize))
            return false;
        char red=(char)0;
        char green=(char)0;
        char blue=(char)0;
        if (fwrite(&red,1,1,fptr) != 1)
            return false;
        if (fwrite(&green,1,1,fptr) != 1)
            return false;
        if (fwrite(&blue,1,1,fptr) != 1)
            return false;

        matEntryChunkSize += mapAmbientColourChunkSize;

        id = MAT_DIFFUSE;
        int mapDiffuseColourChunkSize = 6 + colourChunkSize;
        if (!fffWriteChunkHeader_local(fptr,id,mapDiffuseColourChunkSize))
            return false;
        id = COLOUR_RGB_BYTE;
        if (!fffWriteChunkHeader_local(fptr,id,colourChunkSize))
            return false;
        red= '\xFF';
        green='\xFF';
        blue='\xFF';
        if (fwrite(&red,1,1,fptr) != 1)
            return false;
        if (fwrite(&green,1,1,fptr) != 1)
            return false;
        if (fwrite(&blue,1,1,fptr) != 1)
            return false;

        matEntryChunkSize += mapDiffuseColourChunkSize;

        id = MAT_SPECULAR;
        int mapSpecularColourChunkSize = 6 + colourChunkSize;
        if (!fffWriteChunkHeader_local(fptr,id,mapSpecularColourChunkSize))
            return false;
        id = COLOUR_RGB_BYTE;
        if (!fffWriteChunkHeader_local(fptr,id,colourChunkSize))
            return false;
        red=(char)0;
        green=(char)0;
        blue=(char)0;
        if (fwrite(&red,1,1,fptr) != 1)
            return false;
        if (fwrite(&green,1,1,fptr) != 1)
            return false;
        if (fwrite(&blue,1,1,fptr) != 1)
            return false;

        matEntryChunkSize += mapSpecularColourChunkSize;

        id = MAT_TEXMAP;
        int mapFilenameChunkSize = 6 + int(itr->first.size()) + 1;
        int textMapChunkSize = 6 + mapFilenameChunkSize;
        if (!fffWriteChunkHeader_local(fptr,id,textMapChunkSize))
            return false;
        id = MAT_MAP_FNAME;
        if (!fffWriteChunkHeader_local(fptr,id,mapFilenameChunkSize))
            return false;
        if (!fffWrite3dsString_local(fptr,itr->first))
            return false;

        matEntryChunkSize += textMapChunkSize;

        if (!fffUpdateChunkSizeInfo_local(fptr,matEntryChunkSize,
                matEntryChunkStartPos))
            return false;

        chunkSize += matEntryChunkSize;
    }

    vector< string > objNameList;
    ulong objId;
    for (objId=0; objId<model.numObjs(); ++objId)
    {
        int objChunkStartPos = ftell(fptr);
        id = OBJECT;
        int    objChunkSize = 6;
        if (!fffWriteChunkHeader_local(fptr,id,objChunkSize))
            return false;

        string objName = fffMdlNameTo3dsName(model.getModelName(objId),
                                             objNameList);
        if (!fffWrite3dsString_local(fptr,objName))
            return false;
        objChunkSize += int(objName.size())+1;

        int triObjChunkSize;
        if (!fffWriteTriObjectChunk_local(fptr,triObjChunkSize,
                textureInfoMap,objId,model))
            return false;
        objChunkSize += triObjChunkSize;

        if (!fffUpdateChunkSizeInfo_local(fptr,objChunkSize,objChunkStartPos))
            return false;

        chunkSize += objChunkSize;
    }

    if (!fffUpdateChunkSizeInfo_local(fptr,chunkSize,chunkStartPos))
        return false;

    return true;
}

static
bool
fffSave3dsFile(const Ustring &name, const FffMultiObjectC &model)
{
    Path      path(name);
    path.ext = "3ds";
    Ustring    fname = path.str();
#ifdef _WIN32
    FILE *fptr = _wfopen(fname.as_wstring().c_str(),L"wb,ccs=UNICODE");
#else
    FILE *fptr = fopen(fname.m_str.c_str(),"wb");
#endif
    if (!fptr) {
        return false;
    }
    unsigned short id = 0x4D4D;     // 3DS magic number
    int chunkSize = sizeof(unsigned short) + sizeof(int);
    int chunkStartPos = ftell(fptr);
    if (!fffWriteChunkHeader_local(fptr,id,chunkSize)) {
        fclose(fptr);
        return false;
    }
    {
        id = M3D_VERSION;
        int version = 3;
        int verChunkSize = sizeof(unsigned short) + sizeof(int)
                          + sizeof(int);
        if (!fffWriteChunkHeader_local(fptr,id,verChunkSize))
        {
            fclose(fptr);
            return false;
        }
        if (fwrite(&version,sizeof(int),1,fptr) != 1)
        {
            fclose(fptr);
            return false;
        }
        chunkSize += verChunkSize;
    }
    int mdataChunkSize=0;
    if (!fffWriteMdataChunk_local(fptr,mdataChunkSize,model)) {
        fclose(fptr);
        return false;
    }
    chunkSize += mdataChunkSize;
    if (!fffUpdateChunkSizeInfo_local(fptr,chunkSize,chunkStartPos)) {
        fclose(fptr);
        return false;
    }
    fclose(fptr);
    return true;
}

void
save3ds(
    const Ustring &        fname,
    vector<Mesh>        meshes,
    string                  imgFormat)
{
    for (size_t ii=0; ii<meshes.size(); ++ii) {
        meshes[ii].deltaMorphs.clear();
        meshes[ii].targetMorphs.clear();
    }
    // 3DS internal filenames have 8 chars max so leave 1 for tex number:
    FgMeshLegacy    ml = fgMeshLegacy(meshes,fname,imgFormat,7);
    ml.forcePerVertexTextCoord();
    fffSave3dsFile(fname,ml.base);
}

void
fgSave3dsTest(const CLArgs & args)
{
    FGTESTDIR
    Ustring    dd = dataDir();
    string      rd = "base/";
    Mesh    mouth = loadTri(dd+rd+"Mouth"+".tri");
    mouth.surfaces[0].setAlbedoMap(imgLoadAnyFormat(dd+rd+"MouthSmall.png"));
    Mesh    glasses = loadTri(dd+rd+"Glasses.tri");
    glasses.surfaces[0].setAlbedoMap(imgLoadAnyFormat(dd+rd+"Glasses.tga"));
    save3ds("mshX3ds",fgSvec(mouth,glasses));
    regressFileRel("mshX3ds.3ds","base/test/");
    regressFileRel("mshX3ds0.png","base/test/");
    regressFileRel("mshX3ds1.png","base/test/");
}

}

// */
