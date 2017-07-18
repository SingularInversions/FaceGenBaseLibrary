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
#include "FgStdMap.hpp"
#include "FgStdPair.hpp"

using namespace std;

static
FffMultiObjectC::objData
convertUvs(const FffMultiObjectC::objData & obj)
{
    FffMultiObjectC::objData    ret;
    ret.textureFile = obj.textureFile;
    ret.modelName = obj.modelName;
    ret.ptList = obj.ptList;
    ret.triList = obj.triList;
    ret.quadList = obj.quadList;
    // Loop through surface, assign per-vert UV values, and duplicate as necessary:
    ret.textCoord.resize(ret.ptList.size(),FgVect2F(0));
    vector<bool>                    vertMapped(ret.ptList.size(),false);
    map<pair<uint,FgVect2F>,uint>   toNew;          // Map vertIdx-uvCoord -> vertIdx
    for (size_t tt=0; tt<obj.triList.size(); ++tt) {
        for (uint ii=0; ii<3; ++ii) {
            uint            vertIdx = obj.triList[tt][ii],
                            uvIdx = obj.texTriList[tt][ii];
            FgVect2F        uv = obj.textCoord[uvIdx];
            pair<uint,FgVect2F>     idxUv = make_pair(vertIdx,uv);
            if (!vertMapped[vertIdx]) {             // Preserve original structure
                ret.textCoord[vertIdx] = obj.textCoord[uvIdx];
                vertMapped[vertIdx] = true;
                toNew[idxUv] = vertIdx;
            }
            else if (fgContains(toNew,idxUv)) {     // Re-use existing vert-uv pair
                ret.triList[tt][ii] = toNew[idxUv];
            }
            else {                                  // Duplicate as necessary
                ret.triList[tt][ii] = uint(ret.ptList.size());
                ret.ptList.push_back(obj.ptList[vertIdx]);
                ret.textCoord.push_back(obj.textCoord[uvIdx]);
                toNew[idxUv] = vertIdx;
            }
        }
    }
    for (size_t tt=0; tt<obj.quadList.size(); ++tt) {
        for (uint ii=0; ii<4; ++ii) {
            uint            vertIdx = obj.quadList[tt][ii],
                            uvIdx = obj.texQuadList[tt][ii];
            FgVect2F        uv = obj.textCoord[uvIdx];
            pair<uint,FgVect2F>     idxUv = make_pair(vertIdx,uv);
            if (!vertMapped[vertIdx]) {             // Preserve original structure
                ret.textCoord[vertIdx] = obj.textCoord[uvIdx];
                vertMapped[vertIdx] = true;
                toNew[idxUv] = vertIdx;
            }
            else if (fgContains(toNew,idxUv)) {     // Re-use existing vert-uv pair
                ret.quadList[tt][ii] = toNew[idxUv];
            }
            else {                                  // Duplicate as necessary
                ret.quadList[tt][ii] = uint(ret.ptList.size());
                ret.ptList.push_back(obj.ptList[vertIdx]);
                ret.textCoord.push_back(obj.textCoord[uvIdx]);
                toNew[idxUv] = vertIdx;
            }
        }
    }
    return ret;
}

void
FffMultiObjectC::forcePerVertexTextCoord()
{
    for (size_t ii=0; ii<m_objs.size(); ++ii) {
        if ((m_objs[ii].texTriList.size() == m_objs[ii].triList.size()) &&
            (m_objs[ii].texQuadList.size() == m_objs[ii].quadList.size()))
            m_objs[ii] = convertUvs(m_objs[ii]);
    }
}

// */
