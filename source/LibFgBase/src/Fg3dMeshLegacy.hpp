//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FG3DMESHLEGACY_HPP
#define FG3DMESHLEGACY_HPP

#include "FgSerial.hpp"
#include "FgMatrixC.hpp"
#include "FgMatrixV.hpp"
#include "Fg3dMesh.hpp"

namespace Fg {

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
        Vec3F const *ptArray(int objId) const
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return &(m_objs[objId].ptList[0]);
                        }
        Arr3UI const *triArray(int objId) const
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return &(m_objs[objId].triList[0]);
                        }
        const Arr4UI *quadArray(int objId) const
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return &(m_objs[objId].quadList[0]);
                        }
        const Svec<Vec3F> &getPtList(int objId) const
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return m_objs[objId].ptList;
                        }
        Svec<Vec3F> &getPtList(int objId)
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return m_objs[objId].ptList;
                        }
        const Svec<Arr3UI> &getTriList(int objId) const
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return m_objs[objId].triList;
                        }
        Svec<Arr3UI> &getTriList(int objId)
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return m_objs[objId].triList;
                        }
        const Svec<Arr4UI> &getQuadList(int objId) const
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return m_objs[objId].quadList;
                        }
        Svec<Arr4UI> &getQuadList(int objId)
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return m_objs[objId].quadList;
                        }
        const Svec<Vec2F> &getTextCoord(int objId) const
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return m_objs[objId].textCoord;
                        }
        Svec<Vec2F> &getTextCoord(int objId)
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return m_objs[objId].textCoord;
                        }
        const Svec<Arr3UI> &getTexTriList(int objId) const
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return m_objs[objId].texTriList;
                        }
        Svec<Arr3UI> &getTexTriList(int objId)
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return m_objs[objId].texTriList;
                        }
        const Svec<Arr4UI> &getTexQuadList(int objId) const
                        { 
                            FGASSERT(size_t(objId) < m_objs.size());
                            return m_objs[objId].texQuadList;
                        }
        Svec<Arr4UI> &getTexQuadList(int objId)
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
            String          modelName = "Object" + toStr(objId);
            if (m_objs[objId].modelName.size() != 0 && m_objs[objId].modelName != "")
                modelName = m_objs[objId].modelName;
            return modelName;
        }

        void            clear() { m_objs.clear(); }

        void            forcePerVertexTextCoord();

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
            Svec<Vec3F> ptList;
            Svec<Arr3UI> triList;
            Svec<Arr4UI> quadList;
            Svec<Vec2F> textCoord;
            Svec<Arr3UI> texTriList;
            Svec<Arr4UI> texQuadList;
            std::string             textureFile;
            std::string             modelName;
        };

        Svec<objData> m_objs;
};

struct  FgMeshLegacy
{
    FffMultiObjectC                 base;
    Svec<FffMultiObjectC>    morphs;
    Svec<std::string>        morphNames;

    void
    forcePerVertexTextCoord()
    {
        base.forcePerVertexTextCoord();
        for (size_t ii=0; ii<morphs.size(); ++ii)
            morphs[ii].forcePerVertexTextCoord();
    }
};

// Also saves the texture images to appropriate filenames:
FgMeshLegacy
fgMeshLegacy(
    const Svec<Mesh> &   meshes,
    String8 const &                fname,
    const std::string &             imgFormat,
    uint                            maxLen=0);      // If non-zero, maxmum base filename length allowed (eg. 3DS)

}

#endif
