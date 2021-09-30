//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Very basic LWO export

#include "stdafx.h"
#include "Fg3dMeshLegacy.hpp"
#include "FgStdStream.hpp"
#include "FgImage.hpp"
#include "FgFileSystem.hpp"
#include "Fg3dMesh.hpp"
#include "Fg3dMeshIo.hpp"
#include "FgCommand.hpp"
#include "FgTestUtils.hpp"

using namespace std;

//****************************************************************************
// Local defines
//****************************************************************************
#define LWID(a,b,c,d) (((a)<<24)|((b)<<16)|((c)<<8)|(d))

#define ID_FORM  LWID('F','O','R','M')
#define ID_LWO2  LWID('L','W','O','2')
#define ID_TAGS  LWID('T','A','G','S')
#define ID_CLIP  LWID('C','L','I','P')
#define ID_STIL  LWID('S','T','I','L')
#define ID_LAYR  LWID('L','A','Y','R')
#define ID_PNTS  LWID('P','N','T','S')
#define ID_BBOX  LWID('B','B','O','X')
#define ID_VMAP  LWID('V','M','A','P')
#define ID_TXUV  LWID('T','X','U','V')
#define ID_MORF  LWID('M','O','R','F')
#define ID_POLS  LWID('P','O','L','S')
#define ID_FACE  LWID('F','A','C','E')
#define ID_PTAG  LWID('P','T','A','G')
#define ID_VMAD  LWID('V','M','A','D')

#define ID_SURF  LWID('S','U','R','F')
#define ID_COLR  LWID('C','O','L','R')
#define ID_LUMI  LWID('L','U','M','I')
#define ID_DIFF  LWID('D','I','F','F')
#define ID_SPEC  LWID('S','P','E','C')
#define ID_REFL  LWID('R','E','F','L')
#define ID_TRAN  LWID('T','R','A','N')
#define ID_RIND  LWID('R','I','N','D')
#define ID_GLOS  LWID('G','L','O','S')
#define ID_SIDE  LWID('S','I','D','E')
#define ID_SMAN  LWID('S','M','A','N')
#define ID_BUMP  LWID('B','U','M','P')
#define ID_BLOK  LWID('B','L','O','K')

#define ID_IMAP  LWID('I','M','A','P')
#define ID_CHAN  LWID('C','H','A','N')
#define ID_ENAB  LWID('E','N','A','B')
#define ID_OPAC  LWID('O','P','A','C')
#define ID_NEGA  LWID('N','E','G','A')

#define ID_TMAP  LWID('T','M','A','P')
#define ID_CNTR  LWID('C','N','T','R')
#define ID_SIZE  LWID('S','I','Z','E')
#define ID_ROTA  LWID('R','O','T','A')
#define ID_FALL  LWID('F','A','L','L')
#define ID_OREF  LWID('O','R','E','F')
#define ID_CSYS  LWID('C','S','Y','S')

#define ID_PROJ  LWID('P','R','O','J')
#define ID_AXIS  LWID('A','X','I','S')
#define ID_IMAG  LWID('I','M','A','G')
#define ID_WRAP  LWID('W','R','A','P')
#define ID_WRPW  LWID('W','R','P','W')
#define ID_WRPH  LWID('W','R','P','H')
#define ID_AAST  LWID('A','A','S','T')
#define ID_PIXB  LWID('P','I','X','B')

namespace Fg {

struct LwoTextureInfoS
{
    string                  uvTexName;
    vector<unsigned long>   texVtxList;
    Vec2Fs      texTxtCoord;
};


//****************************************************************************
// Local functions
//****************************************************************************
static bool saveLwoLwsFile(String8 const &fname,
                const FffMultiObjectC &model,
                const vector<FffMultiObjectC> *targets,
                Strings const          *names);

static bool searchVtxTexMap(unsigned long vtxId, Vec2F tex,
                const vector<unsigned long> &vtxList,
                const Vec2Fs &texCoord);
static bool errorFcloseExit(FILE *fptr, String8 const &fname);
static Vec3F toLwoCoord(Vec3F vec) { vec[2]=-vec[2]; return vec; }
static bool swap4BytesWrite(FILE *fptr, const void *ptr);
static bool swap2BytesWrite(FILE *fptr, const void *ptr);
static int  writeVx(FILE *fptr, unsigned long idx);
static bool writeVec12(FILE *fptr, Vec3F const &vect);
static bool writeVec8(FILE *fptr, const Vec2F &vect);
static bool padByte(FILE *fptr)
            { char byte=0; return (fwrite(&byte,1,1,fptr) == 1); }
static bool writeId(FILE *fptr, unsigned long id)
            { return swap4BytesWrite(fptr,&id); }
static bool writeChunkHdr(FILE *, unsigned long id, unsigned long sz);
static bool writeSubChunkHdr(FILE *,unsigned long id,unsigned short sz);
static bool updateChunkSize(FILE *fptr, unsigned long sz, long pos);
static bool updateSubChunkSize(FILE *fptr, unsigned short sz, long pos);

static bool writeTagsChunk(FILE *fptr, unsigned long &chunkSize,
                const FffMultiObjectC &model,
                Strings &tagNameList);
static bool writeLayrChunk(FILE *fptr, unsigned long &chunkSize,
                unsigned short num, string name,
                Vec3F pivot);
static bool writePntsChunks(FILE *fptr, unsigned long &chunkSize,
                const FffMultiObjectC &model, int objIdx);
static bool writeBboxChunks(FILE *fptr, unsigned long &chunkSize,
                const FffMultiObjectC &model, 
                Vec3F &minVec, Vec3F &maxVec, int objIdx);
static bool writeVmapTxuvChunks(FILE *fptr, unsigned long &chunkSize,
                vector<unsigned long> &tmpVtxList,
                Vec2Fs &tmpTexCoord,
                string const &uvTexName,
                const FffMultiObjectC &model, int objIdx, bool singleLayer);
static bool writeVmapMorfChunks(FILE *fptr, unsigned long &chunkSize,
                const FffMultiObjectC &target, string const &targetName,
                const FffMultiObjectC &model, int objIndex);
static bool writePolsChunks(FILE *fptr, unsigned long &chunkSize,
                const FffMultiObjectC &model, int objIndex);
static bool writePtagChunks(FILE *fptr, unsigned long &chunkSize,
                const FffMultiObjectC &model, int objIndex);
static bool writeVmadChunks(FILE *fptr, unsigned long &chunkSize,
                const vector<unsigned long> &tmpVtxList,
                const Vec2Fs &tmpTexCoord,
                string const &uvTexName,
                const FffMultiObjectC &model, int objIdx, bool singleLayer);
static bool writeClipChunks(FILE *fptr, unsigned long &chunkSize,
                vector<unsigned long> &clipIdList, 
                const FffMultiObjectC &model);
static bool writeSurfChunk(FILE *fptr, unsigned long &chunkSize,
                string const &tagName, string const &sourceName,
                string const &uvTexName, unsigned long clipIdx);
static bool writeSurfColrSubChunk(FILE *fptr, unsigned short &chunkSize,
                float red, float green, float blue,
                unsigned long envelope);
static bool writeSurfSideSubChunk(FILE *fptr, unsigned short &chunkSize,
                unsigned short val);
static bool writeSurfSmanSubChunk(FILE *fptr, unsigned short &chunkSize,
                float angle);
static bool writeSurfBlokSubChunk(FILE *fptr, unsigned short &chunkSize,
                string const &uvTexName, unsigned long clipIdx);
static bool writeSurfBlokImapSubChunk(FILE *fptr, unsigned short &chunkSize);
static bool writeSurfBlokTmapSubChunk(FILE *fptr, unsigned short &chunkSize);
static bool writeSurfBlokProjSubChunk(FILE *fptr, unsigned short &chunkSize);
static bool writeSurfBlokAxisSubChunk(FILE *fptr, unsigned short &chunkSize);
static bool writeSurfBlokImagSubChunk(FILE *fptr, unsigned short &chunkSize,
                unsigned long clipIdx);
static bool writeSurfBlokWrapSubChunk(FILE *fptr, unsigned short &chunkSize);
static bool writeSurfBlokWrpwSubChunk(FILE *fptr, unsigned short &chunkSize);
static bool writeSurfBlokWrphSubChunk(FILE *fptr, unsigned short &chunkSize);
static bool writeSurfBlokVmapSubChunk(FILE *fptr, unsigned short &chunkSize,
                string const &uvTexName);
static bool writeSurfBlokAastSubChunk(FILE *fptr, unsigned short &chunkSize);
static bool writeSurfBlokPixbSubChunk(FILE *fptr, unsigned short &chunkSize);

static bool writeLwsFile(String8 const &lwsName, String8 const &lwoName,
                Vec3F minVect, Vec3F maxVect,
                unsigned long numLayers, unsigned long numMorphs, 
                Strings const *morphNames);


//****************************************************************************
//                              fffSaveLwoLwsFile
//****************************************************************************
static bool    fffSaveLwoLwsFile(
    String8 const            &fname,
    const FffMultiObjectC   &model)
{
    return saveLwoLwsFile(fname,model,0,0);
}

static string translateName(string input)
{
	string		out = input,
				token = ": ";
    string::size_type loc = out.find(token);
    if(loc != string::npos)
	    out.replace(loc,token.size(),".");
	return out;
}

static bool    fffSaveLwoLwsFile(

    String8 const                    &fname,
    const FffMultiObjectC           &model,
    const vector<FffMultiObjectC>   &morphTargets,  // Only use the vertices.
    Strings const            &morphNames)
{
	Strings					names;

	names.resize(morphNames.size());
	for (uint ii=0; ii<names.size(); ii++)
		names[ii] = translateName(morphNames[ii]);
    return saveLwoLwsFile(fname,model,&morphTargets,&names);
}


//****************************************************************************
//                              saveLwoLwsFile
//****************************************************************************
static bool saveLwoLwsFile(
    String8 const                  &fname,
    const FffMultiObjectC           &model,
    const vector<FffMultiObjectC>   *targets,       // Only use the vertices.
    Strings const            *names)
{
    Path          path(fname);
    String8        lwsName = path.dirBase() + ".lws";
    String8        lwoName = path.base + ".lwo";
    String8        fullLwoName = path.dirBase() + ".lwo";
#ifdef _WIN32
    FILE *fptr = _wfopen(fullLwoName.as_wstring().c_str(),L"wb,ccs=UNICODE");
#else
    FILE *fptr = fopen(fullLwoName.m_str.c_str(),"wb");
#endif
    if (!fptr) {
        fgThrow("Unable to write to LWO file",fullLwoName);
        return false;
    }

    // Write the FORM file chunk header
    long fileChunkStartPos = ftell(fptr);
    unsigned long fileChunkSize = 0;
    if (!writeChunkHdr(fptr,ID_FORM,fileChunkSize))
        return errorFcloseExit(fptr,fname);

    // Write the LWO2 file chunk header
    if (!writeId(fptr,ID_LWO2))
        return errorFcloseExit(fptr,fname);
    fileChunkSize += 4;

    //
    // Now write the actual file data.
    //

    // TAGS chunk
    unsigned long tagChunkSize = 0;
    Strings tagNameList;
    if (!writeTagsChunk(fptr,tagChunkSize,model,tagNameList))
        return errorFcloseExit(fptr,fname);
    fileChunkSize += tagChunkSize;

    // LAYR chunk
    string layerName = string("Layer");
    Vec3F pivot(0.0f,0.0f,0.0f);
    unsigned long layrChunkSize = 0;
    if (!writeLayrChunk(fptr,layrChunkSize,0,layerName,pivot))
        return errorFcloseExit(fptr,fname);
    fileChunkSize += layrChunkSize;

    // PNTS chunk
    unsigned long pntsChunkSize = 0;
    if (!writePntsChunks(fptr,pntsChunkSize,model,-1))
        return errorFcloseExit(fptr,fname);
    fileChunkSize += pntsChunkSize;

    // BBOX chunk
    Vec3F minVect(0.0f,0.0f,0.0f);
    Vec3F maxVect(0.0f,0.0f,0.0f);
    unsigned long bboxChunkSize = 0;
    if (!writeBboxChunks(fptr,bboxChunkSize,model,minVect,maxVect,-1))
        return errorFcloseExit(fptr,fname);
    fileChunkSize += bboxChunkSize;

    // VMAP (MORF) chunk
    if (targets && names)
    {
        if (targets->size() == names->size())
        {
            for (unsigned long ii=0; ii<targets->size(); ++ii)
            {
                unsigned long morfChunkSize = 0;
                if (!writeVmapMorfChunks(fptr,morfChunkSize,
                        (*targets)[ii],(*names)[ii],model,-1))
                    return errorFcloseExit(fptr,fname);
                fileChunkSize += morfChunkSize;
            }
        }
    }

    vector<LwoTextureInfoS> texInfo;

    // VMAP (TXUV) chunks
    unsigned long obj;
    for (obj=0; obj<model.numObjs(); ++obj)
    {
        char numStr[12];
        sprintf(numStr,"%02d",int(obj));
        string uvTexName = string("UV Texture");
        if (model.numObjs() > 1) uvTexName += string(" ") + string(numStr);

        texInfo.push_back(LwoTextureInfoS());

        unsigned long txuvChunkSize = 0;
        if (!writeVmapTxuvChunks(fptr,txuvChunkSize,
                texInfo[obj].texVtxList,
                texInfo[obj].texTxtCoord,
                uvTexName,model,obj,
                true))
            return errorFcloseExit(fptr,fname);
        fileChunkSize += txuvChunkSize;
        if (texInfo[obj].texVtxList.size() == 0)
            uvTexName = string("");

        texInfo[obj].uvTexName = uvTexName;
    }

    // POLS chunk
    unsigned long polsChunkSize = 0;
    if (!writePolsChunks(fptr,polsChunkSize,model,-1))
        return errorFcloseExit(fptr,fname);
    fileChunkSize += polsChunkSize;

    // PTAG chunk (specify which facet belongs to which surface)
    unsigned long ptagChunkSize = 0;
    if (!writePtagChunks(fptr,ptagChunkSize,model,-1))
        return errorFcloseExit(fptr,fname);
    fileChunkSize += ptagChunkSize;

    // VMAD chunks (for per-facet textures)
    for (obj=0; obj<model.numObjs(); ++obj)
    {
        unsigned long vmadChunkSize = 0;
        if (!writeVmadChunks(fptr,vmadChunkSize,
                texInfo[obj].texVtxList,
                texInfo[obj].texTxtCoord,
                texInfo[obj].uvTexName,
                model,obj,
                true))
            return errorFcloseExit(fptr,fname);
        fileChunkSize += vmadChunkSize;
    }

    // CLIP chunk and STIL sub-chunk (Texture names)
    vector<unsigned long> clipIdList;
    unsigned long clipChunkSize = 0;
    if (!writeClipChunks(fptr,clipChunkSize,clipIdList,model))
        return errorFcloseExit(fptr,fname);
    fileChunkSize += clipChunkSize;

    // SURF chunks
    for (unsigned long ii=0; ii<model.numObjs(); ++ii)
    {
        unsigned long surfChunkSize = 0;
        if (!writeSurfChunk(fptr, surfChunkSize, 
                tagNameList[ii], "",
                texInfo[ii].uvTexName, 
                clipIdList[ii]))
            return errorFcloseExit(fptr,fname);
        fileChunkSize += surfChunkSize;
    }

    // Now update the file chunk size info
    if (!updateChunkSize(fptr,fileChunkSize,fileChunkStartPos+4))
        return errorFcloseExit(fptr,fname);

    if (fileChunkSize % 2 != 0)
    {
        if (!padByte(fptr))
            return errorFcloseExit(fptr,fname);
    }

    fclose(fptr);

    // If morph data exists, save a LWS file as well.
    if (names)
    {
        writeLwsFile(lwsName,lwoName,minVect,maxVect,1,uint(names->size()),names);
    }
    else
    {
        writeLwsFile(lwsName,lwoName,minVect,maxVect,
                     1,
                     0,0);
    }

    return true;
}


//****************************************************************************
//                              searchVtxTexMap
//****************************************************************************
static bool searchVtxTexMap(

    unsigned long               vtxId,
    Vec2F                  tex,
    const vector<unsigned long> &vtxList,
    const Vec2Fs    &texCoord)
{
    bool found = false;

    vector<unsigned long>::const_iterator itr = lower_bound(
        vtxList.begin(),vtxList.end(),vtxId);

    if (itr != vtxList.end())
    {
        if (*itr == vtxId)
        {
            unsigned long idx = itr - vtxList.begin();
            if (texCoord[idx] == tex)
                found = true;
        }
    }

    return found;
}


//****************************************************************************
//                              errorFcloseExit
//****************************************************************************
static bool errorFcloseExit(FILE *fptr, String8 const &)
{
    fclose(fptr);
    return false;
}


//****************************************************************************
//                              swap4BytesWrite
//****************************************************************************
static bool swap4BytesWrite(FILE *fptr, const void *ptr)
{
    char *pp = (char*)ptr;

    if (fwrite(&pp[3],1,1,fptr) != 1)
        return false;
    if (fwrite(&pp[2],1,1,fptr) != 1)
        return false;
    if (fwrite(&pp[1],1,1,fptr) != 1)
        return false;
    if (fwrite(&pp[0],1,1,fptr) != 1)
        return false;

    return true;
}


//****************************************************************************
//                              swap2BytesWrite
//****************************************************************************
static bool swap2BytesWrite(FILE *fptr, const void *ptr)
{
    char *pp = (char*)ptr;

    if (fwrite(&pp[1],1,1,fptr) != 1)
        return false;
    if (fwrite(&pp[0],1,1,fptr) != 1)
        return false;

    return true;
}


//****************************************************************************
//                                  writeVx
//****************************************************************************
static int  writeVx(FILE *fptr, unsigned long idx)
{
    if (idx < 0xFF00)
    {
        unsigned short idx2 = (unsigned short) idx;
        if (!swap2BytesWrite(fptr,&idx2))
            return 0;
        else
            return 2;
    }
    else
    {
        unsigned long idx2 = idx | 0xFF000000;
        if (!swap4BytesWrite(fptr,&idx2))
            return 0;
        else
            return 4;
    }
}


//****************************************************************************
//                              writeVec12
//****************************************************************************
static bool writeVec12(FILE *fptr, Vec3F const &vect)
{
    if (!swap4BytesWrite(fptr,&vect[0]))
        return false;
    if (!swap4BytesWrite(fptr,&vect[1]))
        return false;
    return swap4BytesWrite(fptr,&vect[2]);
}


//****************************************************************************
//                              writeVec8
//****************************************************************************
static bool writeVec8(FILE *fptr, const Vec2F &vect)
{
    if (!swap4BytesWrite(fptr,&vect[0]))
        return false;
    return swap4BytesWrite(fptr,&vect[1]);
}


//****************************************************************************
//                              writeChunkHdr
//****************************************************************************
static bool writeChunkHdr(FILE *fptr, unsigned long id, unsigned long sz)
{
    if (!writeId(fptr,id))
        return false;

    return swap4BytesWrite(fptr,&sz);
}


//****************************************************************************
//                              writeSubChunkHdr
//****************************************************************************
static bool writeSubChunkHdr(FILE *fptr, unsigned long id, unsigned short sz)
{
    if (!writeId(fptr,id))
        return false;

    return swap2BytesWrite(fptr,&sz);
}


//****************************************************************************
//                              updateChunkSize
//****************************************************************************
static bool updateChunkSize(FILE *fptr, unsigned long sz, long pos)
{
    long currPos = ftell(fptr);

    if (fseek(fptr,pos,SEEK_SET))
        return false;

    if (!swap4BytesWrite(fptr,&sz))
        return false;

    if (fseek(fptr,currPos,SEEK_SET))
        return false;

    return true;
}


//****************************************************************************
//                              updateSubChunkSize
//****************************************************************************
static bool updateSubChunkSize(FILE *fptr, unsigned short sz, long pos)
{
    long currPos = ftell(fptr);

    if (fseek(fptr,pos,SEEK_SET))
        return false;

    if (!swap2BytesWrite(fptr,&sz))
        return false;

    if (fseek(fptr,currPos,SEEK_SET))
        return false;

    return true;
}


//****************************************************************************
//                              writeTagsChunk
//****************************************************************************
static bool writeTagsChunk(

    FILE                    *fptr,
    unsigned long           &chunkSize,
    const FffMultiObjectC   &model,
    Strings          &tagNameList)
{
    unsigned long localChunkSize=0;
    long localChunkStart = ftell(fptr);

    if (!writeChunkHdr(fptr,ID_TAGS,localChunkSize))
        return false;
     chunkSize += 8;

    unsigned long numObjs = model.numObjs();

    for (unsigned long ii=0; ii<numObjs; ++ii)
    {
        // Surface name
        string tagName = model.getModelName(ii);

        tagNameList.push_back(tagName);

        unsigned long tagNameLn = uint(strlen(tagName.c_str()))+1;
        tagNameLn = (tagNameLn + 1) & ~0x00000001;      // even byte pad

        if (fwrite(tagName.c_str(),1,tagNameLn,fptr) != tagNameLn)
            return false;

        localChunkSize += tagNameLn;
        chunkSize += tagNameLn;
    }

    if (!updateChunkSize(fptr,localChunkSize,localChunkStart+4))
        return false;

    return true;
}


//****************************************************************************
//                              writeLayrChunk
//****************************************************************************
static bool writeLayrChunk(

    FILE            *fptr,
    unsigned long   &chunkSize,
    unsigned short  num,
    string          name,
    Vec3F      pivot)
{
    chunkSize = 0;

    unsigned long nameLn = uint(strlen(name.c_str())) + 1;
    unsigned long layrSize = (2 + 2 + 12 + nameLn+1) & ~0x00000001;
    unsigned short flags = 0;

    // LAYR chunk
    if (!writeChunkHdr(fptr,ID_LAYR,layrSize))
        return false;
    chunkSize += 8;

    // Layer number
    if (!swap2BytesWrite(fptr,&num))
        return false;
    chunkSize += 2;

    // flags
    if (!swap2BytesWrite(fptr,&flags))
        return false;
    chunkSize += 2;

    // pivot
    pivot = toLwoCoord(pivot);
    if (!writeVec12(fptr,pivot))
        return false;
    chunkSize += 12;

    // name
    if (fwrite(name.c_str(),1,nameLn,fptr) != nameLn)
        return false;
    chunkSize += nameLn;
    if (nameLn % 2 != 0)
    {
        if (!padByte(fptr))
            return false;
        chunkSize++;
    }

    return true;
}


//****************************************************************************
//                              writePntsChunk
//****************************************************************************
static bool writePntsChunks(

    FILE                    *fptr,
    unsigned long           &chunkSize,
    const FffMultiObjectC   &model,
    int                     objIdx)
{
    chunkSize = 0;

    unsigned long pntsSize = 0;
    unsigned long startObj = 0;
    unsigned long endObj = model.numObjs();
    if (objIdx != -1)
    {
        startObj = objIdx;
        endObj = objIdx+1;
    }
    unsigned long obj;
    for (obj=startObj; obj<endObj; ++obj)
    {
        pntsSize += model.numPoints(obj) * 12;
    }

    // PNTS chunk
    if (!writeChunkHdr(fptr,ID_PNTS,pntsSize))
        return false;
    chunkSize += 8;

    // Now write the points
    for (obj=startObj; obj<endObj; ++obj)
    {
        const Vec3Fs &pts = model.getPtList(obj);
        for (unsigned long ii=0; ii<pts.size(); ++ii)
        {
            // Lightwave uses left-handed coordinate system!
            Vec3F pt = toLwoCoord(pts[ii]);
            if (!writeVec12(fptr,pt))
                return false;
            chunkSize += 12;
        }
    }

    return true;
}


//****************************************************************************
//                              writeBboxChunks
//****************************************************************************
static bool writeBboxChunks(

    FILE                    *fptr, 
    unsigned long           &chunkSize,
    const FffMultiObjectC   &model, 
    Vec3F              &minPnt,
    Vec3F              &maxPnt,
    int                     objIdx)
{
    chunkSize = 0;

    unsigned long startObj = 0;
    unsigned long endObj = model.numObjs();
    if (objIdx != -1)
    {
        startObj = objIdx;
        endObj = objIdx+1;
    }

    bool maxMinInitialized = false;
    for (unsigned long obj=startObj; obj<endObj; ++obj)
    {
        const Vec3Fs &pts = model.getPtList(obj);
        if (!maxMinInitialized && pts.size() > 0)
        {
            minPnt = maxPnt = pts[0];
            maxMinInitialized = true;
        }
        for (unsigned long ii=0; ii<pts.size(); ++ii)
        {
            if (minPnt[0] > pts[ii][0]) minPnt[0] = pts[ii][0];
            if (minPnt[1] > pts[ii][1]) minPnt[1] = pts[ii][1];
            if (minPnt[2] > pts[ii][2]) minPnt[2] = pts[ii][2];
            if (maxPnt[0] < pts[ii][0]) maxPnt[0] = pts[ii][0];
            if (maxPnt[1] < pts[ii][1]) maxPnt[1] = pts[ii][1];
            if (maxPnt[2] < pts[ii][2]) maxPnt[2] = pts[ii][2];
        }
    }

    if (!maxMinInitialized) return true;

    // BBOX chunk
    unsigned long bboxSize = 24;
    if (!writeChunkHdr(fptr,ID_BBOX,bboxSize))
        return false;
    chunkSize += 8;

    // Write out the bounding box values
    Vec3F minVal = toLwoCoord(minPnt);
    Vec3F maxVal = toLwoCoord(maxPnt);
    if (!writeVec12(fptr,minVal))
        return false;
    chunkSize += 12;
    if (!writeVec12(fptr,maxVal))
        return false;
    chunkSize += 12;

    return true;
}


//****************************************************************************
//                              writeVmapTxuvChunk
//****************************************************************************
static bool writeVmapTxuvChunks(

    FILE                    *fptr,
    unsigned long           &chunkSize,
    vector<unsigned long>   &tmpVtxList,
    Vec2Fs      &tmpTxtCoord,
    string const            &uvTexName,
    const FffMultiObjectC   &model,
    int                     objIdx,
    bool                    singleLayer)
{
    chunkSize = 0;

    unsigned long vtxOffset = 0;
    if (singleLayer)
    {
        for (int obj=0; obj<objIdx; ++obj)
        {
            vtxOffset += model.numPoints(obj);
        }
    }

    // Build the tmpVtxList and tmpTxtCoord (vertex to texCoord mapping)
    tmpVtxList.clear();
    tmpTxtCoord.clear();
    unsigned long obj = objIdx;

    Vec2Fs txtCoord;
    vector<bool>       touched;
    txtCoord.resize(model.numPoints(obj),Vec2F(0.0f,0.0f));
    touched.resize(model.numPoints(obj),false);

    if (model.numTxtCoord(obj) != 0)
    {
        if (model.numTxtCoord(obj) == model.numPoints(obj) &&
            model.numTxtTris(obj) == 0 &&
            model.numTxtQuads(obj) == 0)
        {
            // Per-vertex texture
            const Vec2Fs &txt = model.getTextCoord(obj);
            txtCoord = txt;
            touched.clear();
            touched.resize(txt.size(),true);
        }
        else if (model.numTxtTris(obj) == model.numTriangles(obj) &&
                 model.numTxtQuads(obj) == model.numQuads(obj))
        {
            // Per-facet texture

            // Build a temporary per-vertex mapping.  Incorrect ones
            // will be remapped by the VMAD chunk.
            const Vec2Fs &txt = model.getTextCoord(obj);
            Vec3UIs const &tris = model.getTriList(obj);
            const vector<Vec4UI> &quads = model.getQuadList(obj);
            Vec3UIs const &txtTris = model.getTexTriList(obj);
            const vector<Vec4UI> &txtQuads = model.getTexQuadList(obj);
            for (unsigned long tt=0; tt<tris.size(); ++tt)
            {
                txtCoord[ tris[ tt ][0] ] = txt[ txtTris[ tt ][0] ];
                txtCoord[ tris[ tt ][1] ] = txt[ txtTris[ tt ][1] ];
                txtCoord[ tris[ tt ][2] ] = txt[ txtTris[ tt ][2] ];

                touched[ tris[ tt ][0] ] = true;
                touched[ tris[ tt ][1] ] = true;
                touched[ tris[ tt ][2] ] = true;
            }
            for (unsigned long qq=0; qq<quads.size(); ++qq)
            {
                txtCoord[ quads[ qq ][0] ] = txt[ txtQuads[ qq ][0] ];
                txtCoord[ quads[ qq ][1] ] = txt[ txtQuads[ qq ][1] ];
                txtCoord[ quads[ qq ][2] ] = txt[ txtQuads[ qq ][2] ];
                txtCoord[ quads[ qq ][3] ] = txt[ txtQuads[ qq ][3] ];

                touched[ quads[ qq ][0] ] = true;
                touched[ quads[ qq ][1] ] = true;
                touched[ quads[ qq ][2] ] = true;
                touched[ quads[ qq ][3] ] = true;
            }
        }
    }

    for (unsigned long vv=0; vv<touched.size(); ++vv)
    {
        if (touched[vv])
        {
            tmpVtxList.push_back(vv+vtxOffset);
            tmpTxtCoord.push_back(txtCoord[vv]);
        }
    }

    if (tmpTxtCoord.size() == 0)
    {
        return true;
    }

    // VMAP chunk
    unsigned long vmapSize = 0;
    long vmapChunkStart = ftell(fptr);
    unsigned short dimension = 2;

    if (!writeChunkHdr(fptr,ID_VMAP,vmapSize))
        return false;
    chunkSize += 8;

    // TXUV type
    if (!writeId(fptr,ID_TXUV))
        return false;
    chunkSize += 4;
    vmapSize += 4;

    // dimension
    if (!swap2BytesWrite(fptr,&dimension))
        return false;
    chunkSize += 2;
    vmapSize += 2;

    // name
    size_t  ln = strlen(uvTexName.c_str()) + 1;
    if (fwrite(uvTexName.c_str(),1,ln,fptr) != ln)
        return false;
    if (ln % 2 != 0)
    {
        if (!padByte(fptr))
            return false;
        ln++;
    }
    chunkSize += ulong(ln);
    vmapSize += ulong(ln);

    // texture coordinate data
    for (unsigned long ii=0; ii<tmpTxtCoord.size(); ++ii)
    {
        int bytes = writeVx(fptr,tmpVtxList[ii]);
        if (bytes == 0) return false;
        chunkSize += bytes;
        vmapSize += bytes;

        if (!writeVec8(fptr,tmpTxtCoord[ii]))
            return false;
        chunkSize += 8;
        vmapSize += 8;
    }

    // Update the local chunk size
    if (!updateChunkSize(fptr,vmapSize,vmapChunkStart+4))
        return false;

    return true;
}


//****************************************************************************
//                              writeVmapMorfChunk
//****************************************************************************
static bool writeVmapMorfChunks(

    FILE                    *fptr,
    unsigned long           &chunkSize,
    const FffMultiObjectC   &target,
    string const            &targetName,
    const FffMultiObjectC   &model,
    int                     objIdx)
{
    chunkSize = 0;

    if (target.numObjs() != model.numObjs()) return true;

    unsigned long startObj = 0;
    unsigned long endObj = model.numObjs();
    if (objIdx != -1)
    {
        startObj = objIdx;
        endObj = objIdx+1;
    }

    vector<unsigned long> tmpVtxList;
    Vec3Fs tmpMorfDelta;
    tmpVtxList.clear();
    tmpMorfDelta.clear();

    unsigned long vtxOffset = 0;
    for (unsigned long obj=startObj; obj<endObj; ++obj)
    {
        if (model.numPoints(obj) == target.numPoints(obj))
        {
            const Vec3Fs &vtxList = model.getPtList(obj);
            const Vec3Fs &tvtxList = target.getPtList(obj);
            for (unsigned long vtx=0; vtx<vtxList.size(); ++vtx)
            {
                tmpVtxList.push_back(vtx+vtxOffset);
                tmpMorfDelta.push_back(tvtxList[vtx] - vtxList[vtx]);
            }
        }

        vtxOffset += model.numPoints(obj);
    }

    if (tmpMorfDelta.size() == 0)
        return true;

    // VMAP chunk
    unsigned long vmapSize = 0;
    long vmapChunkStart = ftell(fptr);
    unsigned short dimension = 3;

    if (!writeChunkHdr(fptr,ID_VMAP,vmapSize))
        return false;
    chunkSize += 8;

    // MORF type
    if (!writeId(fptr,ID_MORF))
        return false;
    chunkSize += 4;
    vmapSize += 4;

    // dimension
    if (!swap2BytesWrite(fptr,&dimension))
        return false;
    chunkSize += 2;
    vmapSize += 2;

    // name
    size_t ln = strlen(targetName.c_str()) + 1;
    if (fwrite(targetName.c_str(),1,ln,fptr) != ln)
        return false;
    if (ln % 2 != 0)
    {
        if (!padByte(fptr))
            return false;
        ln++;
    }
    chunkSize += ulong(ln);
    vmapSize += ulong(ln);

    // texture coordinate data
    for (unsigned long ii=0; ii<tmpMorfDelta.size(); ++ii)
    {
        int bytes = writeVx(fptr,tmpVtxList[ii]);
        if (bytes == 0) return false;
        chunkSize += bytes;
        vmapSize += bytes;

        // Lightwave uses left-handed coordinate system!
        tmpMorfDelta[ii] = toLwoCoord(tmpMorfDelta[ii]);
        if (!writeVec12(fptr,tmpMorfDelta[ii]))
            return false;
        chunkSize += 12;
        vmapSize += 12;
    }

    // Update the local chunk size
    if (!updateChunkSize(fptr,vmapSize,vmapChunkStart+4))
        return false;

    return true;
}


//****************************************************************************
//                              writePolsChunk
//****************************************************************************
static bool writePolsChunks(

    FILE                    *fptr,
    unsigned long           &chunkSize,
    const FffMultiObjectC   &model,
    int                     objIdx)
{
    chunkSize = 0;

    unsigned long startObj = 0;
    unsigned long endObj = model.numObjs();
    if (objIdx != -1)
    {
        startObj = objIdx;
        endObj = objIdx+1;
    }

    unsigned long polsSize = 0;
    long polsChunkStart = ftell(fptr);

    // POLS chunk
    if (!writeChunkHdr(fptr,ID_POLS,polsSize))
        return false;
    chunkSize += 8;

    // FACE type
    if (!writeId(fptr,ID_FACE))
        return false;
    chunkSize += 4;
    polsSize += 4;

    unsigned long vtxOffset=0;
    for (unsigned long obj=startObj; obj<endObj; ++obj)
    {
        unsigned long vtx;
        int bytes;

        //
        // Poly list
        //

        // Note: Lightwave requires the poly to be posed clockwise.

        unsigned short polySize = 3;
        Vec3UIs const &triList = model.getTriList(obj);
        for (unsigned long tri=0; tri<triList.size(); ++tri)
        {
            if (!swap2BytesWrite(fptr,&polySize))
                return false;
            chunkSize += 2;
            polsSize += 2;

            vtx = triList[tri][2] + vtxOffset;
            bytes = writeVx(fptr,vtx);
            if (bytes == 0) return false;
            chunkSize += bytes;
            polsSize += bytes;

            vtx = triList[tri][1] + vtxOffset;
            bytes = writeVx(fptr,vtx);
            if (bytes == 0) return false;
            chunkSize += bytes;
            polsSize += bytes;

            vtx = triList[tri][0] + vtxOffset;
            bytes = writeVx(fptr,vtx);
            if (bytes == 0) return false;
            chunkSize += bytes;
            polsSize += bytes;
        }

        polySize = 4;
        const vector<Vec4UI> &quadList = model.getQuadList(obj);
        for (unsigned long quad=0; quad<quadList.size(); ++quad)
        {
            if (!swap2BytesWrite(fptr,&polySize))
                return false;
            chunkSize += 2;
            polsSize += 2;

            vtx = quadList[quad][3] + vtxOffset;
            bytes = writeVx(fptr,vtx);
            if (bytes == 0) return false;
            chunkSize += bytes;
            polsSize += bytes;

            vtx = quadList[quad][2] + vtxOffset;
            bytes = writeVx(fptr,vtx);
            if (bytes == 0) return false;
            chunkSize += bytes;
            polsSize += bytes;

            vtx = quadList[quad][1] + vtxOffset;
            bytes = writeVx(fptr,vtx);
            if (bytes == 0) return false;
            chunkSize += bytes;
            polsSize += bytes;

            vtx = quadList[quad][0] + vtxOffset;
            bytes = writeVx(fptr,vtx);
            if (bytes == 0) return false;
            chunkSize += bytes;
            polsSize += bytes;
        }

        vtxOffset += model.numPoints(obj);
    }

    // Update the local chunk size
    if (!updateChunkSize(fptr,polsSize,polsChunkStart+4))
        return false;

    return true;
}


//****************************************************************************
//                              writePtagChunk
//****************************************************************************
static bool writePtagChunks(

    FILE                    *fptr,
    unsigned long           &chunkSize,
    const FffMultiObjectC   &model,
    int                     objIdx)
{
    chunkSize = 0;

    unsigned long startObj = 0;
    unsigned long endObj = model.numObjs();
    if (objIdx != -1)
    {
        startObj = objIdx;
        endObj = objIdx+1;
    }

    unsigned long ptagSize = 0;
    long ptagChunkStart = ftell(fptr);

    // PTAG chunk
    if (!writeChunkHdr(fptr,ID_PTAG,ptagSize))
        return false;
    chunkSize += 8;

    // SURF type
    if (!writeId(fptr,ID_SURF))
        return false;
    chunkSize += 4;
    ptagSize += 4;

    unsigned long polyOffset=0;
    for (unsigned long obj=startObj; obj<endObj; ++obj)
    {
        unsigned short tagId = obj;

        for (unsigned long tri=0; tri<model.numTriangles(obj); ++tri)
        {
            unsigned long poly = tri + polyOffset;

            int bytes = writeVx(fptr,poly);
            if (bytes == 0) return false;
            chunkSize += bytes;
            ptagSize += bytes;

            if (!swap2BytesWrite(fptr,&tagId))
                return false;
            chunkSize += 2;
            ptagSize += 2;
        }
        polyOffset += model.numTriangles(obj);

        for (unsigned long quad=0; quad<model.numQuads(obj); ++quad)
        {
            unsigned long poly = quad + polyOffset;

            int bytes = writeVx(fptr,poly);
            if (bytes == 0) return false;
            chunkSize += bytes;
            ptagSize += bytes;

            if (!swap2BytesWrite(fptr,&tagId))
                return false;
            chunkSize += 2;
            ptagSize += 2;
        }
        polyOffset += model.numQuads(obj);;
    }

    // Update the local chunk size
    if (!updateChunkSize(fptr,ptagSize,ptagChunkStart+4))
        return false;

    return true;
}


//****************************************************************************
//                              writeVmadChunk
//****************************************************************************
//
// Update the per-facet texture with proper values.
//
static bool writeVmadChunks(

    FILE                    *fptr,
    unsigned long           &chunkSize,
    const vector<unsigned long> &tmpVtxList,
    const Vec2Fs &tmpTexCoord,
    string const            &uvTexName,
    const FffMultiObjectC   &model,
    int                     objIdx,
    bool                    singleLayer)
{
    chunkSize = 0;
    if (tmpTexCoord.size() == 0 || tmpVtxList.size() == 0) return true;
    if (tmpTexCoord.size() != tmpVtxList.size()) return true;

    unsigned long vtxOffset=0;
    unsigned long polyOffset=0;
    if (singleLayer)
    {
        for (int obj=0; obj<objIdx; ++obj)
        {
            polyOffset += uint(model.getTriList(obj).size());
            polyOffset += uint(model.getQuadList(obj).size());
            vtxOffset += uint(model.numPoints(obj));
        }
    }

    // Build the per-facet info
    vector<unsigned long>   vtxList;
    vector<unsigned long>   polyList;
    Vec2Fs      texList;
    unsigned long obj = objIdx;

    Vec3UIs const &triList = model.getTriList(obj);
    Vec3UIs const &texTriList = model.getTexTriList(obj);
    const vector<Vec4UI> &quadList = model.getQuadList(obj);
    const vector<Vec4UI> &texQuadList = model.getTexQuadList(obj);
    const Vec2Fs &txList = model.getTextCoord(obj);

    // Only needs to do this for per-facet texture data.
    if (txList.size() != 0 &&
        triList.size() == texTriList.size() &&
        quadList.size() == texQuadList.size())
    {
        unsigned long vtx;
        unsigned long poly;
        Vec2F tex;
        for (unsigned long tri=0; tri<triList.size(); ++tri)
        {
            poly = tri + polyOffset;

            vtx = triList[tri][0] + vtxOffset;
            tex = txList[ texTriList[tri][0] ];
            if (!searchVtxTexMap(vtx,tex,tmpVtxList,tmpTexCoord))
            {
                vtxList.push_back(vtx);
                polyList.push_back(poly);
                texList.push_back(tex);
            }

            vtx = triList[tri][1] + vtxOffset;
            tex = txList[ texTriList[tri][1] ];
            if (!searchVtxTexMap(vtx,tex,tmpVtxList,tmpTexCoord))
            {
                vtxList.push_back(vtx);
                polyList.push_back(poly);
                texList.push_back(tex);
            }

            vtx = triList[tri][2] + vtxOffset;
            tex = txList[ texTriList[tri][2] ];
            if (!searchVtxTexMap(vtx,tex,tmpVtxList,tmpTexCoord))
            {
                vtxList.push_back(vtx);
                polyList.push_back(poly);
                texList.push_back(tex);
            }
        }
        polyOffset += uint(triList.size());

        for (unsigned long quad=0; quad<quadList.size(); ++quad)
        {
            poly = quad + polyOffset;

            vtx = quadList[quad][0] + vtxOffset;
            tex = txList[ texQuadList[quad][0] ];
            if (!searchVtxTexMap(vtx,tex,tmpVtxList,tmpTexCoord))
            {
                vtxList.push_back(vtx);
                polyList.push_back(poly);
                texList.push_back(tex);
            }

            vtx = quadList[quad][1] + vtxOffset;
            tex = txList[ texQuadList[quad][1] ];
            if (!searchVtxTexMap(vtx,tex,tmpVtxList,tmpTexCoord))
            {
                vtxList.push_back(vtx);
                polyList.push_back(poly);
                texList.push_back(tex);
            }

            vtx = quadList[quad][2] + vtxOffset;
            tex = txList[ texQuadList[quad][2] ];
            if (!searchVtxTexMap(vtx,tex,tmpVtxList,tmpTexCoord))
            {
                vtxList.push_back(vtx);
                polyList.push_back(poly);
                texList.push_back(tex);
            }

            vtx = quadList[quad][3] + vtxOffset;
            tex = txList[ texQuadList[quad][3] ];
            if (!searchVtxTexMap(vtx,tex,tmpVtxList,tmpTexCoord))
            {
                vtxList.push_back(vtx);
                polyList.push_back(poly);
                texList.push_back(tex);
            }
        }
        polyOffset += uint(quadList.size());
    }
    else
    {
        polyOffset += uint(triList.size());
        polyOffset += uint(quadList.size());
    }
    vtxOffset += model.numPoints(obj);

    // Check if there are any per-facet texture data that needs to be
    // corrected.
    if (vtxList.size() == 0)
        return true;

    unsigned long vmadSize = 0;
    long vmadChunkStart = ftell(fptr);
    unsigned short dimension = 2;

    // VMAD chunk
    if (!writeChunkHdr(fptr,ID_VMAD,vmadSize))
        return false;
    chunkSize += 8;

    // TXUV type
    if (!writeId(fptr,ID_TXUV))
        return false;
    chunkSize += 4;
    vmadSize += 4;

    // dimension
    if (!swap2BytesWrite(fptr,&dimension))
        return false;
    chunkSize += 2;
    vmadSize += 2;

    // name
    size_t  ln = strlen(uvTexName.c_str()) + 1;
    if (fwrite(uvTexName.c_str(),1,ln,fptr) != ln)
        return false;
    chunkSize += ulong(ln);
    vmadSize += ulong(ln);
    if (ln % 2 != 0)
    {
        if (!padByte(fptr))
            return false;
        chunkSize ++;
        vmadSize ++;
    }

    // Now save the new per-facet mapping info
    for (unsigned long ii=0; ii<vtxList.size(); ++ii)
    {
        int bytes = writeVx(fptr,vtxList[ii]);
        if (bytes == 0) return false;
        chunkSize += bytes;
        vmadSize += bytes;

        bytes = writeVx(fptr,polyList[ii]);
        if (bytes == 0) return false;
        chunkSize += bytes;
        vmadSize += bytes;

        if (!writeVec8(fptr,texList[ii]))
            return false;
        chunkSize += 8;
        vmadSize += 8;
    }

    // Update the local chunk size
    if (!updateChunkSize(fptr,vmadSize,vmadChunkStart+4))
        return false;

    return true;
}


//****************************************************************************
//                              writeClipChunks
//****************************************************************************
static bool writeClipChunks(

    FILE                    *fptr,
    unsigned long           &chunkSize,
    vector<unsigned long>   &clipIdList,
    const FffMultiObjectC   &model)
{
    clipIdList.clear();

    for (unsigned long ii=0; ii<model.numObjs(); ++ii)
    {
        string texFname = model.getTextureFilename(ii);
        if (texFname != "")
        {
            unsigned long clipIdx = ii+1;
            clipIdList.push_back(clipIdx);

            // CLIP chunk
            unsigned long clipChunkSize=0;
            long clipChunkPos = ftell(fptr);
            if (!writeChunkHdr(fptr,ID_CLIP,clipChunkSize))
                return false;
            chunkSize += 8;

            // clip index
            if (!swap4BytesWrite(fptr,&clipIdx))
                return false;
            clipChunkSize += 4;
            chunkSize += 4;

            /////////////////
            // STIL sub-chunk
            /////////////////

            unsigned short ln = ushort(strlen(texFname.c_str())) + 1;
            unsigned short stilSubSize = (ln + 1) & ~0x0001;
            if (!writeSubChunkHdr(fptr,ID_STIL,stilSubSize))
                return false;
            clipChunkSize += 6;
            chunkSize += 6;

            // Image filename
            if (fwrite(texFname.c_str(),1,ln,fptr) != ln)
                return false;
            if (ln != stilSubSize)
            {
                if (!padByte(fptr))
                    return false;
            }
            clipChunkSize += stilSubSize;
            chunkSize += stilSubSize;

            ////////////////////////
            // End of STIL sub-chunk
            ////////////////////////

            // Update CLIP chunk's size info
            if (!updateChunkSize(fptr,clipChunkSize,clipChunkPos+4))
                return false;
        }
        else
            clipIdList.push_back(0);
    }

    return true;
}


//****************************************************************************
//                              writeSurfChunk
//****************************************************************************
static bool writeSurfChunk(

    FILE            *fptr,
    unsigned long   &chunkSize,
    string const    &tagName,
    string const    &sourceName,
    string const    &uvTexName,
    unsigned long   clipIdx)
{
    chunkSize = 0;

    unsigned long surfChunkSize = 0;
    long surfChunkStart = ftell(fptr);

    // SURF tag
    if (!writeChunkHdr(fptr,ID_SURF,surfChunkSize))
        return false;
    chunkSize += 8;

    // tag name
    size_t  ln = strlen(tagName.c_str()) + 1;
    if (fwrite(tagName.c_str(),1,ln,fptr) != ln)
        return false;
    chunkSize += ulong(ln);
    surfChunkSize += ulong(ln);
    if (ln % 2 != 0)
    {
        if (!padByte(fptr))
            return false;
        chunkSize ++;
        surfChunkSize ++;
    }

    // source
    ln = strlen(sourceName.c_str()) + 1;
    if (fwrite(sourceName.c_str(),1,ln,fptr) != ln)
        return false;
    chunkSize += ulong(ln);
    surfChunkSize += ulong(ln);
    if (ln % 2 != 0)
    {
        if (!padByte(fptr))
            return false;
        chunkSize ++;
        surfChunkSize ++;
    }

    //
    // attributes (sub-chunks)
    //

    // COLR sub-chunk
    unsigned short colrSubChunkSize=0;
    if (!writeSurfColrSubChunk(fptr,colrSubChunkSize,1.0f,1.0f,1.0f,0))
        return false;
    chunkSize += colrSubChunkSize;
    surfChunkSize += colrSubChunkSize;

    // SMAN sub-chunk   -- max smoothing angle in radian
    unsigned short smanSubChunkSize=0;
    if (!writeSurfSmanSubChunk(fptr,smanSubChunkSize,1.5))
        return false;
    chunkSize += smanSubChunkSize;
    surfChunkSize += smanSubChunkSize;

    // SIDE sub-chunk
    unsigned short sideSubChunkSize=0;
    if (!writeSurfSideSubChunk(fptr,sideSubChunkSize,1))
        return false;
    chunkSize += sideSubChunkSize;
    surfChunkSize += sideSubChunkSize;

    // Now the texture stuff
    if (clipIdx == 0 && uvTexName == "")    // No texture image or tex coord
    {
        // Update the current SURF chunk size
        if (!updateChunkSize(fptr,surfChunkSize,surfChunkStart+4))
            return false;
        return true;
    }

    // BLOK sub-chunk
    unsigned short blokSubChunkSize=0;
    if (!writeSurfBlokSubChunk(fptr,blokSubChunkSize,uvTexName,clipIdx))
        return false;
    chunkSize += blokSubChunkSize;
    surfChunkSize += blokSubChunkSize;

    // Update the current SURF chunk size
    if (!updateChunkSize(fptr,surfChunkSize,surfChunkStart+4))
        return false;

    return true;
}


//****************************************************************************
//                          writeSurfColrSubChunk
//****************************************************************************
static bool writeSurfColrSubChunk(

    FILE            *fptr,
    unsigned short  &chunkSize,
    float           red,
    float           green,
    float           blue,
    unsigned long   envelope)
{
    chunkSize = 0;

    if (!writeSubChunkHdr(fptr,ID_COLR,14))
        return false;
    chunkSize += 6;

    if (!swap4BytesWrite(fptr,&red))
        return false;
    if (!swap4BytesWrite(fptr,&green))
        return false;
    if (!swap4BytesWrite(fptr,&blue))
        return false;
    if (!swap2BytesWrite(fptr,&envelope))
        return false;
    chunkSize += 14;

    return true;
}


//****************************************************************************
//                          writeSurfSideSubChunk
//****************************************************************************
static bool writeSurfSideSubChunk(

    FILE            *fptr,
    unsigned short  &chunkSize,
    unsigned short  val)
{
    chunkSize = 0;

    if (!writeSubChunkHdr(fptr,ID_SIDE,2))
        return false;
    chunkSize += 6;

    if (!swap2BytesWrite(fptr,&val))
        return false;
    chunkSize += 2;

    return true;
}


//****************************************************************************
//                          writeSurfSmanSubChunk
//****************************************************************************
static bool writeSurfSmanSubChunk(

    FILE            *fptr,
    unsigned short  &chunkSize,
    float           angle)
{
    chunkSize = 0;

    if (!writeSubChunkHdr(fptr,ID_SMAN,4))
        return false;
    chunkSize += 6;

    if (!swap4BytesWrite(fptr,&angle))
        return false;
    chunkSize += 4;

    return true;
}


//****************************************************************************
//                          writeSurfBlokSubChunk
//****************************************************************************
static bool writeSurfBlokSubChunk(

    FILE            *fptr,
    unsigned short  &chunkSize,
    string const    &uvTexName,
    unsigned long   clipIdx)
{
    chunkSize = 0;

    unsigned short blokSubChunkSize = 0;
    long blokSubChunkStart = ftell(fptr);

    // BLOK sub-chunk
    if (!writeSubChunkHdr(fptr,ID_BLOK,blokSubChunkSize))
        return false;
    chunkSize += 6;

    // BLOK's IMAP sub-chunk
    unsigned short imapSubChunkSize=0;
    if (!writeSurfBlokImapSubChunk(fptr,imapSubChunkSize))
        return false;
    chunkSize += imapSubChunkSize;
    blokSubChunkSize += imapSubChunkSize;

    // BLOK's TMAP sub-chunk
    unsigned short tmapSubChunkSize=0;
    if (!writeSurfBlokTmapSubChunk(fptr,tmapSubChunkSize))
        return false;
    chunkSize += tmapSubChunkSize;
    blokSubChunkSize += tmapSubChunkSize;

    // BLOK's PROJ sub-chunk
    if (uvTexName != "")
    {
        unsigned short projSubChunkSize=0;
        if (!writeSurfBlokProjSubChunk(fptr,projSubChunkSize))
            return false;
        chunkSize += projSubChunkSize;
        blokSubChunkSize += projSubChunkSize;
    }

    // BLOK's AXIS sub-chunk
    unsigned short axisSubChunkSize=0;
    if (!writeSurfBlokAxisSubChunk(fptr,axisSubChunkSize))
        return false;
    chunkSize += axisSubChunkSize;
    blokSubChunkSize += axisSubChunkSize;

    // BLOK's IMAG sub-chunk
    unsigned short imagSubChunkSize=0;
    if (!writeSurfBlokImagSubChunk(fptr,imagSubChunkSize,clipIdx))
        return false;
    chunkSize += imagSubChunkSize;
    blokSubChunkSize += imagSubChunkSize;

    // BLOK's WRAP sub-chunk
    unsigned short wrapSubChunkSize=0;
    if (!writeSurfBlokWrapSubChunk(fptr,wrapSubChunkSize))
        return false;
    chunkSize += wrapSubChunkSize;
    blokSubChunkSize += wrapSubChunkSize;

    // BLOK's WRPW sub-chunk
    unsigned short wrpwSubChunkSize=0;
    if (!writeSurfBlokWrpwSubChunk(fptr,wrpwSubChunkSize))
        return false;
    chunkSize += wrpwSubChunkSize;
    blokSubChunkSize += wrpwSubChunkSize;

    // BLOK's WRPH sub-chunk
    unsigned short wrphSubChunkSize=0;
    if (!writeSurfBlokWrphSubChunk(fptr,wrphSubChunkSize))
        return false;
    chunkSize += wrphSubChunkSize;
    blokSubChunkSize += wrphSubChunkSize;

    // BLOK's VMAP sub-chunk
    unsigned short vmapSubChunkSize=0;
    if (!writeSurfBlokVmapSubChunk(fptr,vmapSubChunkSize,uvTexName))
        return false;
    chunkSize += vmapSubChunkSize;
    blokSubChunkSize += vmapSubChunkSize;

    // BLOK's AAST sub-chunk
    unsigned short aastSubChunkSize=0;
    if (!writeSurfBlokAastSubChunk(fptr,aastSubChunkSize))
        return false;
    chunkSize += aastSubChunkSize;
    blokSubChunkSize += aastSubChunkSize;

    // BLOK's PIXB sub-chunk
    unsigned short pixbSubChunkSize=0;
    if (!writeSurfBlokPixbSubChunk(fptr,pixbSubChunkSize))
        return false;
    chunkSize += pixbSubChunkSize;
    blokSubChunkSize += pixbSubChunkSize;

    //
    // Update BLOK sub-chunk's size info
    //
    if (!updateSubChunkSize(fptr,blokSubChunkSize,blokSubChunkStart+4))
        return false;

    return true;
}


//****************************************************************************
//                          writeSurfBlokImapSubChunk
//****************************************************************************
static bool writeSurfBlokImapSubChunk(

    FILE            *fptr,
    unsigned short  &chunkSize)
{
    chunkSize = 0;

    unsigned short imapSubChunkSize = 0;
    long imapSubChunkStart = ftell(fptr);

    // IMAP sub-chunk
    if (!writeSubChunkHdr(fptr,ID_IMAP,imapSubChunkSize))
        return false;
    chunkSize += 6;

    // IMAP's ordinal string (since we only have one block, use 0x80)
    unsigned char ordStr[2] = { (unsigned char)0x80, 0 };
    if (fwrite(ordStr,1,2,fptr) != 2)
        return false;
    chunkSize += 2;
    imapSubChunkSize += 2;

    // IMAP's CHAN sub-chunk
    unsigned short chanSubChunkSize = 4;
    if (!writeSubChunkHdr(fptr,ID_CHAN,chanSubChunkSize))
        return false;
    chunkSize += 6;
    imapSubChunkSize += 6;

    // IMAP's CHAN sub-chunk's data
    if (!writeId(fptr,ID_COLR))
        return false;
    chunkSize += chanSubChunkSize;
    imapSubChunkSize += chanSubChunkSize;

    // IMAP's OPAC sub-chunk
    unsigned short opacSubChunkSize = 8;
    if (!writeSubChunkHdr(fptr,ID_OPAC,opacSubChunkSize))
        return false;
    chunkSize += 6;
    imapSubChunkSize += 6;

    // IMAP's OPAC sub-chunk's data
    unsigned short opacType = 0;
    float opac = 1.0f;
    unsigned short envelope = 0;

    if (!swap2BytesWrite(fptr,&opacType))
        return false;
    if (!swap4BytesWrite(fptr,&opac))
        return false;
    if (!swap2BytesWrite(fptr,&envelope))
        return false;
    chunkSize += opacSubChunkSize;
    imapSubChunkSize += opacSubChunkSize;

    // IMAP's ENAB sub-chunk
    unsigned short enabSubChunkSize = 2;
    if (!writeSubChunkHdr(fptr,ID_ENAB,enabSubChunkSize))
        return false;
    chunkSize += 6;
    imapSubChunkSize += 6;

    // IMAP's ENAB sub-chunk's data
    unsigned short enable=1;
    if (!swap2BytesWrite(fptr,&enable))
        return false;
    chunkSize += enabSubChunkSize;
    imapSubChunkSize += enabSubChunkSize;

    // IMAP's NEGA sub-chunk
    unsigned short negaSubChunkSize = 2;
    if (!writeSubChunkHdr(fptr,ID_NEGA,negaSubChunkSize))
        return false;
    chunkSize += 6;
    imapSubChunkSize += 6;

    // IMAP's NEGA sub-chunk's data
    unsigned short nega=0;
    if (!swap2BytesWrite(fptr,&nega))
        return false;
    chunkSize += negaSubChunkSize;
    imapSubChunkSize += negaSubChunkSize;

    // Update IMAP sub-chunk's size info
    if (!updateSubChunkSize(fptr,imapSubChunkSize,imapSubChunkStart+4))
        return false;

    return true;
}


//****************************************************************************
//                          writeSurfBlokTmapSubChunk
//****************************************************************************
static bool writeSurfBlokTmapSubChunk(FILE *fptr, unsigned short &chunkSize)
{
    chunkSize = 0;

    unsigned short tmapSubChunkSize = 0;
    long tmapSubChunkStart = ftell(fptr);

    // TMAP sub-chunk
    if (!writeSubChunkHdr(fptr,ID_TMAP,tmapSubChunkSize))
        return false;
    chunkSize += 6;

    unsigned short envelope = 0;

    // TMAP's CNTR sub-chunk
    unsigned short cntrSubChunkSize = 14;
    if (!writeSubChunkHdr(fptr,ID_CNTR,cntrSubChunkSize))
        return false;
    chunkSize += 6;
    tmapSubChunkSize += 6;

    // TMAP's CNTR sub-chunk's data
    Vec3F cntr(0.0f,0.0f,0.0f);
    if (!writeVec12(fptr,cntr))
        return false;
    if (!swap2BytesWrite(fptr,&envelope))
        return false;
    chunkSize += cntrSubChunkSize;
    tmapSubChunkSize += cntrSubChunkSize;

    // TMAP's SIZE sub-chunk
    unsigned short sizeSubChunkSize = 14;
    if (!writeSubChunkHdr(fptr,ID_SIZE,sizeSubChunkSize))
        return false;
    chunkSize += 6;
    tmapSubChunkSize += 6;

    // TMAP's SIZE sub-chunk's data
    Vec3F size(1.0f,1.0f,1.0f);
    if (!writeVec12(fptr,size))
        return false;
    if (!swap2BytesWrite(fptr,&envelope))
        return false;
    chunkSize += sizeSubChunkSize;
    tmapSubChunkSize += sizeSubChunkSize;

    // TMAP's ROTA sub-chunk
    unsigned short rotaSubChunkSize = 14;
    if (!writeSubChunkHdr(fptr,ID_ROTA,rotaSubChunkSize))
        return false;
    chunkSize += 6;
    tmapSubChunkSize += 6;

    // TMAP's ROTA sub-chunk's data
    Vec3F rota(0.0f,0.0f,0.0f);
    if (!writeVec12(fptr,rota))
        return false;
    if (!swap2BytesWrite(fptr,&envelope))
        return false;
    chunkSize += rotaSubChunkSize;
    tmapSubChunkSize += rotaSubChunkSize;

    // TMAP's FALL sub-chunk
    unsigned short fallSubChunkSize = 16;
    if (!writeSubChunkHdr(fptr,ID_FALL,fallSubChunkSize))
        return false;
    chunkSize += 6;
    tmapSubChunkSize += 6;

    // TMAP's FALL sub-chunk's data
    unsigned short type = 0;
    Vec3F fall(0.0f,0.0f,0.0f);
    if (!swap2BytesWrite(fptr,&type))
        return false;
    if (!writeVec12(fptr,fall))
        return false;
    if (!swap2BytesWrite(fptr,&envelope))
        return false;
    chunkSize += fallSubChunkSize;
    tmapSubChunkSize += fallSubChunkSize;

    // TMAP's OREF sub-chunk
    unsigned short orefSubChunkSize = 8;
    if (!writeSubChunkHdr(fptr,ID_OREF,orefSubChunkSize))
        return false;
    chunkSize += 6;
    tmapSubChunkSize += 6;

    // TMAP's OREF sub-chunk's data
    char oref[8] = { '(', 'n', 'o', 'n', 'e', ')', 0, 0 };
    if (fwrite(oref,1,8,fptr) != 8)
        return false;
    chunkSize += orefSubChunkSize;
    tmapSubChunkSize += orefSubChunkSize;

    // TMAP's CSYS sub-chunk
    unsigned short csysSubChunkSize = 2;
    if (!writeSubChunkHdr(fptr,ID_CSYS,csysSubChunkSize))
        return false;
    chunkSize += 6;
    tmapSubChunkSize += 6;

    // TMAP's CSYS sub-chunk's data
    unsigned short csysType = 0;
    if (!swap2BytesWrite(fptr,&csysType))
        return false;
    chunkSize += csysSubChunkSize;
    tmapSubChunkSize += csysSubChunkSize;

    // Update TMAP sub-chunk's size info
    if (!updateSubChunkSize(fptr,tmapSubChunkSize,tmapSubChunkStart+4))
        return false;

    return true;
}


//****************************************************************************
//                          writeSurfBlokProjSubChunk
//****************************************************************************
static bool writeSurfBlokProjSubChunk(FILE *fptr, unsigned short &chunkSize)
{
    chunkSize = 0;

    unsigned short val = 5;

    if (!writeSubChunkHdr(fptr,ID_PROJ,2))
        return false;
    chunkSize += 6;

    if (!swap2BytesWrite(fptr,&val))
        return false;
    chunkSize += 2;

    return true;
}


//****************************************************************************
//                          writeSurfBlokAxisSubChunk
//****************************************************************************
static bool writeSurfBlokAxisSubChunk(FILE *fptr, unsigned short &chunkSize)
{
    chunkSize = 0;

    unsigned short val = 2;

    if (!writeSubChunkHdr(fptr,ID_AXIS,2))
        return false;
    chunkSize += 6;

    if (!swap2BytesWrite(fptr,&val))
        return false;
    chunkSize += 2;

    return true;
}


//****************************************************************************
//                          writeSurfBlokImagSubChunk
//****************************************************************************
static bool writeSurfBlokImagSubChunk(

    FILE            *fptr,
    unsigned short  &chunkSize,
    unsigned long   clipIdx)
{
    chunkSize = 0;

    if (clipIdx == 0) return true;

    unsigned short sz = 2;
    if (clipIdx >= 0xFF00) sz += 2;

    if (!writeSubChunkHdr(fptr,ID_IMAG,sz))
        return false;
    chunkSize += 6;

    int bytes = writeVx(fptr,clipIdx);
    if (bytes != sz) return false;
    chunkSize += bytes;

    return true;
}


//****************************************************************************
//                          writeSurfBlokWrapSubChunk
//****************************************************************************
static bool writeSurfBlokWrapSubChunk(FILE *fptr, unsigned short &chunkSize)
{
    chunkSize = 0;

    unsigned short val = 1;

    if (!writeSubChunkHdr(fptr,ID_WRAP,4))
        return false;
    chunkSize += 6;

    if (!swap2BytesWrite(fptr,&val))
        return false;
    chunkSize += 2;

    if (!swap2BytesWrite(fptr,&val))
        return false;
    chunkSize += 2;

    return true;
}


//****************************************************************************
//                          writeSurfBlokWrpwSubChunk
//****************************************************************************
static bool writeSurfBlokWrpwSubChunk(FILE *fptr, unsigned short &chunkSize)
{
    chunkSize = 0;

    float cycles = 1.0f;
    unsigned short envelope = 0;

    if (!writeSubChunkHdr(fptr,ID_WRPW,6))
        return false;
    chunkSize += 6;

    if (!swap4BytesWrite(fptr,&cycles))
        return false;
    chunkSize += 4;

    if (!swap2BytesWrite(fptr,&envelope))
        return false;
    chunkSize += 2;

    return true;
}


//****************************************************************************
//                          writeSurfBlokWrphSubChunk
//****************************************************************************
static bool writeSurfBlokWrphSubChunk(FILE *fptr, unsigned short &chunkSize)
{
    chunkSize = 0;

    float cycles = 1.0f;
    unsigned short envelope = 0;

    if (!writeSubChunkHdr(fptr,ID_WRPH,6))
        return false;
    chunkSize += 6;

    if (!swap4BytesWrite(fptr,&cycles))
        return false;
    chunkSize += 4;

    if (!swap2BytesWrite(fptr,&envelope))
        return false;
    chunkSize += 2;

    return true;
}


//****************************************************************************
//                          writeSurfBlokVmapSubChunk
//****************************************************************************
static bool writeSurfBlokVmapSubChunk(

    FILE            *fptr,
    unsigned short  &chunkSize,
    string const    &uvTexName)
{
    chunkSize = 0;

    if (uvTexName == "") return true;

    unsigned short ln = ushort(strlen(uvTexName.c_str())) + 1;
    unsigned short sz = (ln+1) & ~((unsigned short)1);

    if (!writeSubChunkHdr(fptr,ID_VMAP,sz))
        return false;
    chunkSize += 6;

    if (fwrite(uvTexName.c_str(),1,ln,fptr) != ln)
        return false;
    chunkSize += ln;
    if (ln != sz)
    {
        if (!padByte(fptr))
            return false;
        ln++;
        if (ln != sz) return false;
        chunkSize++;
    }

    return true;
}


//****************************************************************************
//                          writeSurfBlokAastSubChunk
//****************************************************************************
static bool writeSurfBlokAastSubChunk(FILE *fptr, unsigned short &chunkSize)
{
    chunkSize = 0;

    float aast = 1.0f;
    unsigned short flags = 1;

    if (!writeSubChunkHdr(fptr,ID_AAST,6))
        return false;
    chunkSize += 6;

    if (!swap2BytesWrite(fptr,&flags))
        return false;
    chunkSize += 2;

    if (!swap4BytesWrite(fptr,&aast))
        return false;
    chunkSize += 4;

    return true;
}


//****************************************************************************
//                          writeSurfBlokPixbSubChunk
//****************************************************************************
static bool writeSurfBlokPixbSubChunk(FILE *fptr, unsigned short &chunkSize)
{
    chunkSize = 0;

    unsigned short flags = 1;

    if (!writeSubChunkHdr(fptr,ID_PIXB,2))
        return false;
    chunkSize += 6;

    if (!swap2BytesWrite(fptr,&flags))
        return false;
    chunkSize += 2;

    return true;
}


//****************************************************************************
//                                  writeLwsFile
//****************************************************************************
//
// Helper function to translate our morph naming scheme to one compatible with
// programs using LWO2 format (ie replacing the ": " with ".").
//
static bool writeLwsFile(
    String8 const            &lwsName, 
    String8 const            &lwoName,
    Vec3F              minVect,
    Vec3F              maxVect,
    unsigned long           numLayers,
    unsigned long           numMorphs, 
    Strings const    *morphNames)
{
    Ofstream file(lwsName);
    if (!file)
    {
        return false;
    }

    // Start the file write
    file << "LWSC\n"
        "3\n"
        "\n"
        "FirstFrame 1\n"
        "LastFrame 60\n"
        "FrameStep 1\n"
        "PreviewFirstFrame 0\n"
        "PreviewLastFrame 60\n"
        "PreviewFrameStep 1\n"
        "CurrentFrame 0\n"
        "FramesPerSecond 30\n"
        "\n";

    for (unsigned long layer=0; layer<numLayers; ++layer)
    {
        file << "LoadObjectLayer " << layer+1 << " " << lwoName.as_ascii() << "\n"
            "ShowObject 6 3\n"
            "ObjectMotion\n"
            "NumChannels 9\n";
        unsigned long ii;
        for (ii=0; ii<=5; ++ii)
        {
            file << "Channel " << ii << "\n"
                "{ Envelope\n"
                "  1\n"
                "  Key 0 0 0 0 0 0 0 0 0\n"
                "  Behaviors 1 1\n"
                "}\n";
        }
        for (ii=6; ii<=8; ++ii)
        {
            file << "Channel " << ii << "\n"
                "{ Envelope\n"
                "  1\n"
                "  Key 1 0 0 0 0 0 0 0 0\n"
                "  Behaviors 1 1\n"
                "}\n";
        }
        file << "\n";

        if (numMorphs && morphNames)
        {
            file << "Plugin DisplacementHandler 1 LW_MorphMixer\n"
                << numMorphs << "\n"
                "1\n"
                "{ Group\n"
                "  " << numMorphs << "\n"
                "  \"Miscellaneous\"\n"
                "}\n";
            for (unsigned long mm=0; mm<numMorphs; ++mm)
            {
                file << "{ MorfForm\n"
                    "  \"" << (*morphNames)[mm] << "\"\n"
                    "  0\n"
                    "  { Envelope\n"
                    "    1\n"
                    "    Key 0 0 0 0 0 0 0 0 0\n"
                    "    Behaviors 1 1\n"
                    "  }\n"
                    "}\n";
            }
            file << "EndPlugin\n";
        }
        file << "ShadowOptions 7\n"
            "\n";
    }

    // Estimate a proper grid size
    double maxVal = fabs(minVect[0]);
    maxVal = (maxVal >= fabs(minVect[1])) ? maxVal : fabs(minVect[1]);
    maxVal = (maxVal >= fabs(minVect[2])) ? maxVal : fabs(minVect[2]);
    maxVal = (maxVal >= fabs(maxVect[0])) ? maxVal : fabs(maxVect[0]);
    maxVal = (maxVal >= fabs(maxVect[1])) ? maxVal : fabs(maxVect[1]);
    maxVal = (maxVal >= fabs(maxVect[2])) ? maxVal : fabs(maxVect[2]);
    int power10=1;
    double      val;
    for (val=maxVal; val>=10.0; val/=10.0) { power10 *= 10; }
    int gridSize = (int)(val);
    if (gridSize >= 5)      gridSize = 5 * power10;
    else if (gridSize >= 2) gridSize = 2 * power10;
    else                    gridSize = power10;

    file << "GridSize " << gridSize << "\n";

    return (!file.fail());
}

void
saveLwo(
    String8 const &        fname,
    Meshes const & meshes,
    string                  imgFormat)
{
    FgMeshLegacy            leg = fgMeshLegacy(meshes,fname,imgFormat);
    if (leg.morphs.empty())
        fffSaveLwoLwsFile(fname,leg.base);
    else {
        fffSaveLwoLwsFile(fname,leg.base,leg.morphs,leg.morphNames);
    }
}

void
fgSaveLwoTest(CLArgs const & args)
{
    FGTESTDIR
    String8    dd = dataDir();
    string      rd = "base/";
    Mesh    mouth = loadTri(dd+rd+"Mouth"+".tri");
    mouth.surfaces[0].setAlbedoMap(loadImage(dd+rd+"MouthSmall.png"));
    Mesh    glasses = loadTri(dd+rd+"Glasses.tri");
    glasses.surfaces[0].setAlbedoMap(loadImage(dd+rd+"Glasses.tga"));
    saveLwo("meshExportLwo",svec(mouth,glasses));
    regressFileRel("meshExportLwo.lwo","base/test/");
    regressFileRel("meshExportLwo.lws","base/test/",equateFilesText);
    regressFileRel("meshExportLwo0.png","base/test/");
    regressFileRel("meshExportLwo1.png","base/test/");
}

}

// */
