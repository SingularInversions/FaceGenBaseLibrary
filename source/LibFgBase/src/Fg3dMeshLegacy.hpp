//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     John Leung
// Created:     March 11, 2001
//

#ifndef FG3DMESHLEGACY_HPP
#define FG3DMESHLEGACY_HPP

#include "FgDiagnostics.hpp"
#include "FgMatrix.hpp"
#include "Fg3dMesh.hpp"

struct  FffMultiObjectC
{
        // Function interfaces to get information.
        unsigned long   numObjs() const 
                        { return uint(m_objs.size()); }
        unsigned long   numPoints(int objId) const 
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return uint(m_objs[objId].ptList.size()); 
                        }
        unsigned long   numTriangles(int objId) const 
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return uint(m_objs[objId].triList.size());
                        }
        unsigned long   numQuads(int objId) const
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return uint(m_objs[objId].quadList.size());
                        }
        unsigned long   numTxtCoord(int objId) const
                        {
                            FGASSERT(size_t(objId) < m_objs.size());
                            return uint(m_objs[objId].textCoord.size());
                        }
        unsigned long   numTxtTris(int objId) const
                        {
                            FGASSERT(size_t(objId) < m_objs.size());
                            return uint(m_objs[objId].texTriList.size());
                        }
        unsigned long   numTxtQuads(int objId) const
                        {
                            FGASSERT(size_t(objId) < m_objs.size());
                            return uint(m_objs[objId].texQuadList.size());
                        }
        unsigned long   totalNumPts() const
                        {
                            unsigned long total=0;
                            for (unsigned long ii=0; ii<numObjs(); ++ii)
                                total += numPoints(ii);
                            return total;
                        }
        unsigned long   totalNumTriangles() const
                        {
                            unsigned long total=0;
                            for (unsigned long ii=0; ii<numObjs(); ++ii)
                                total += numTriangles(ii);
                            return total;
                        }
        unsigned long   totalNumQuads() const
                        {
                            unsigned long total=0;
                            for (unsigned long ii=0; ii<numObjs(); ++ii)
                                total += numQuads(ii);
                            return total;
                        }
        const FgVect3F *ptArray(int objId) const
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return &(m_objs[objId].ptList[0]);
                        }
        const FgVect3UI *triArray(int objId) const
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return &(m_objs[objId].triList[0]);
                        }
        const FgVect4UI *quadArray(int objId) const
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return &(m_objs[objId].quadList[0]);
                        }
        const std::vector<FgVect3F> &getPtList(int objId) const
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return m_objs[objId].ptList;
                        }
        std::vector<FgVect3F> &getPtList(int objId)
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return m_objs[objId].ptList;
                        }
        const std::vector<FgVect3UI> &getTriList(int objId) const
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return m_objs[objId].triList;
                        }
        std::vector<FgVect3UI> &getTriList(int objId)
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return m_objs[objId].triList;
                        }
        const std::vector<FgVect4UI> &getQuadList(int objId) const
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return m_objs[objId].quadList;
                        }
        std::vector<FgVect4UI> &getQuadList(int objId)
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return m_objs[objId].quadList;
                        }
        const std::vector<FgVect2F> &getTextCoord(int objId) const
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return m_objs[objId].textCoord;
                        }
        std::vector<FgVect2F> &getTextCoord(int objId)
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return m_objs[objId].textCoord;
                        }
        const std::vector<FgVect3UI> &getTexTriList(int objId) const
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return m_objs[objId].texTriList;
                        }
        std::vector<FgVect3UI> &getTexTriList(int objId)
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return m_objs[objId].texTriList;
                        }
        const std::vector<FgVect4UI> &getTexQuadList(int objId) const
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return m_objs[objId].texQuadList;
                        }
        std::vector<FgVect4UI> &getTexQuadList(int objId)
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return m_objs[objId].texQuadList;
                        }
        std::string     getTextureFilename(int objId=0) const
                        {
                            FGASSERT(size_t(objId) < m_objs.size());
                            std::string textureFile("");
                            if (m_objs[objId].textureFile.size() != 0 &&
                                m_objs[objId].textureFile != "")
                                textureFile = m_objs[objId].textureFile;
                            return textureFile;
                        }
        std::string     getModelName(int objId=0) const
        {
            FGASSERT(size_t(objId) < m_objs.size());
            char str[48];
            sprintf(str,"Object%d",objId);
            std::string modelName(str);
            if (m_objs[objId].modelName.size() != 0 &&
                m_objs[objId].modelName != "")
                modelName = m_objs[objId].modelName;
            return modelName;
        }

        void            clear() { m_objs.clear(); }

        void            clearExceptVtxData();
        bool            copyVtxData(const FffMultiObjectC &data);

        bool            forcePerVertexTextCoord();
        bool            forceObjectPerVertexTextCoord(int objId,
                            std::vector<int> *newVtxToOldVtxMap=0);

        void            optimizeVertexList();
        void            optimizeObjectVertexList(int objId);


        // Functions for adding new objects or setting data to a
        // specific obhect.
        void            setTextureFilename(
                            int                 objId, 
                            const std::string   &filename)
                        {
                            FGASSERT(size_t(objId) < m_objs.size());
                            m_objs[objId].textureFile = filename;
                        }
        void            setModelName(
                            int                 objId, 
                            const std::string   &modelName)
                        {
                            FGASSERT(size_t(objId) < m_objs.size());
                            m_objs[objId].modelName = modelName;
                        }

        struct objData
        {
            std::vector<FgVect3F> ptList;
            std::vector<FgVect3UI> triList;
            std::vector<FgVect4UI> quadList;
            std::vector<FgVect2F> textCoord;
            std::vector<FgVect3UI> texTriList;
            std::vector<FgVect4UI> texQuadList;
            std::string             textureFile;
            std::string             modelName;
        };

        std::vector<objData> m_objs;
};

struct  FgMeshLegacy
{
    FffMultiObjectC                 base;
    std::vector<FffMultiObjectC>    morphs;
    std::vector<std::string>        morphNames;
};

// Also saves the texture images to appropriate filenames:
FgMeshLegacy
fgMeshLegacy(
    const std::vector<Fg3dMesh> &   meshes,
    const FgString &                fname,
    const std::string &             imgFormat,
    uint                            maxLen=0);      // If non-zero, maxmum base filename length allowed (eg. 3DS)

#endif
