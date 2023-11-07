//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgAnthropometry.hpp"

using namespace std;

namespace Fg {

ostream &           operator<<(ostream & os,FaceView fv)
{
    if (fv == FaceView::front)
        return os << "front";
    else if (fv == FaceView::left)
        return os << "left";
    else if (fv == FaceView::right)
        return os << "right";
    else
        FGASSERT_FALSE;
    return os;
}

FaceLmAtts const &  getFaceLmAtts()
{
    static FaceLmAtts   ret {
        {FaceLm::endocanthionL,"EndocanthionL"},            {FaceLm::endocanthionR,"EndocanthionR"},
        {FaceLm::exocanthionL,"ExocanthionL"},              {FaceLm::exocanthionR,"ExocanthionR"},
        {FaceLm::lidLowerCentreL,"LidLowerCentreL"},        {FaceLm::lidLowerCentreR,"LidLowerCentreR"},
        {FaceLm::lidUpperCentreL,"LidUpperCentreL"},        {FaceLm::lidUpperCentreR,"LidUpperCentreR"},
        {FaceLm::lidLowerInnerHalfL,"LidLowerInnerHalfL"},  {FaceLm::lidLowerInnerHalfR,"LidLowerInnerHalfR"},
        {FaceLm::lidLowerOuterHalfL,"LidLowerOuterHalfL"},  {FaceLm::lidLowerOuterHalfR,"LidLowerOuterHalfR"},
        {FaceLm::lidUpperInnerHalfL,"LidUpperInnerHalfL"},  {FaceLm::lidUpperInnerHalfR,"LidUpperInnerHalfR"},
        {FaceLm::lidUpperOuterHalfL,"LidUpperOuterHalfL"},  {FaceLm::lidUpperOuterHalfR,"LidUpperOuterHalfR"},
        {FaceLm::pupilCentreL,"PupilCentreL"},              {FaceLm::pupilCentreR,"PupilCentreR"},
        {FaceLm::sellion,"Sellion"},
        {FaceLm::cheekboneL,"CheekboneL"},                  {FaceLm::cheekboneR,"CheekboneR"},
        {FaceLm::alarOuterL,"AlarOuterL"},                  {FaceLm::alarOuterR,"AlarOuterR"},
        {FaceLm::noseBridgeMiddle,"NoseBridgeMiddle"},
        {FaceLm::noseTip,"NoseTip"},                        // Pronasale
        {FaceLm::subNasale,"SubNasale"},
        {FaceLm::jawOuterL,"JawOuterL"},                    {FaceLm::jawOuterR,"JawOuterR"},
        {FaceLm::mouthCornerL,"MouthCornerL"},              {FaceLm::mouthCornerR,"MouthCornerR"},
        {FaceLm::cristaPhiltriL,"CristaPhiltriL"},          {FaceLm::cristaPhiltriR,"CristaPhiltriR"},
        {FaceLm::labialeSuperius,"LabialeSuperius"},
        {FaceLm::labialeInferius,"LabialeInferius"},
        {FaceLm::lipUpperFront,"LipUpperFront"},
        {FaceLm::lipLowerFront,"LipLowerFront"},
        {FaceLm::mouthCentre,"MouthCentre"},
        {FaceLm::supramentale,"Supramentale"},
        {FaceLm::chinFront,"ChinFront"},
        {FaceLm::chinBottom,"ChinBottom"},                  // Gnathion
        {FaceLm::jawBottom,"JawBottom"},                    // from PF3 "THROAT_TOP"
    };
    return ret;
}

Svec<Pair<Gender,char>> cGenderCharList()
{
    return {
        {Gender::female,'f'},
        {Gender::male,'m'},
    };
}

Svec<Pair<RaceGroup,char>> cRaceCharList()
{
    return {
        {RaceGroup::african,'a'},
        {RaceGroup::caucasian,'c'},
        {RaceGroup::eAsian,'e'},
        {RaceGroup::sAsian,'s'},
        {RaceGroup::other,'o'},
    };
}

ostream &           operator<<(std::ostream & os,FaceLm fl)
{
    return os << findFirst(getFaceLmAtts(),fl).label;
}

template<>
Opt<FaceLm>         fromStr<FaceLm>(String const & s)
{
    for (FaceLmAtt const & fla : getFaceLmAtts())
        if (s == fla.label)
            return fla.type;
    return {};
}

FaceLms             getLmsEyelid()
{
    return {
        FaceLm::endocanthionL,      FaceLm::endocanthionR,
        FaceLm::exocanthionL,       FaceLm::exocanthionR,
        FaceLm::lidLowerCentreL,    FaceLm::lidLowerCentreR,
        FaceLm::lidUpperCentreL,    FaceLm::lidUpperCentreR,
    };
}

FaceLms             getLmsEyelidExt()
{
    return cat(getLmsEyelid(),{
        FaceLm::lidLowerInnerHalfL, FaceLm::lidLowerInnerHalfR,
        FaceLm::lidLowerOuterHalfL, FaceLm::lidLowerOuterHalfR,
        FaceLm::lidUpperInnerHalfL, FaceLm::lidUpperInnerHalfR,
        FaceLm::lidUpperOuterHalfL, FaceLm::lidUpperOuterHalfR,
    });
}

FaceLms             getLmsMouth()
{
    return {
        FaceLm::mouthCornerL,       FaceLm::mouthCornerR,
        FaceLm::cristaPhiltriL,     FaceLm::cristaPhiltriR,
        FaceLm::labialeSuperius,
        FaceLm::labialeInferius,
    };
}

FaceLms                 getEyeGuideLms(Sagg LR)
{
    if (LR == Sagg::left)
        return {
            FaceLm::exocanthionL,
            FaceLm::lidLowerCentreL,
            FaceLm::lidUpperCentreL,
            FaceLm::lidLowerInnerHalfL,
            FaceLm::lidLowerOuterHalfL,
            FaceLm::lidUpperInnerHalfL,
            FaceLm::lidUpperOuterHalfL,
        };
    else
        return {
            FaceLm::exocanthionR,
            FaceLm::lidLowerCentreR,
            FaceLm::lidUpperCentreR,
            FaceLm::lidLowerInnerHalfR,
            FaceLm::lidLowerOuterHalfR,
            FaceLm::lidUpperInnerHalfR,
            FaceLm::lidUpperOuterHalfR,
        };
}

Strings             toLabels(FaceLms const & lms)
{
    FaceLmAtts const &  atts = getFaceLmAtts();
    Strings             ret; ret.reserve(lms.size());
    for (FaceLm lm : lms)
        ret.push_back(findFirst(atts,lm).label);
    return ret;
}

Strings             getLmStrsEyelid()
{
    Strings             half {
        "Endocanthion",
        "Exocanthion",
        "LidLowerCentre",
        "LidLowerInnerHalf",
        "LidLowerOuterHalf",
        "LidUpperCentre",
        "LidUpperInnerHalf",
        "LidUpperOuterHalf",
    };
    Strings             ret;
    ret.reserve(half.size()*2);
    for (char side : {'L','R'})             // do not use "LR" as it will include the null character !
        for (String const & lm : half)
            ret.push_back(lm+side);
    return ret;
}
Strings             getLmStrsMouth()
{
    return {
        "MouthCornerR",
        "CristaPhiltriR",
        "LabialeSuperius",
        "CristaPhiltriL",
        "MouthCornerL",
        "LabialeInferius",
    };
}
Strings             getLmStrsNose()
{
    return {
        "SubNasale",
        "NoseTip",
        "AlarOuterL",
        "AlarOuterR",
    };
}

static SquareF      cFaceRegion(Vec2Fs const & lms,Vec2F relShift,float relSize)
{
    Mat22F              bnds = cBounds(lms);
    Vec2F               lo = bnds.colVec(0),
                        hi = bnds.colVec(1),
                        centre = (lo+hi) * 0.5f;
    float               sz = hi[1]-lo[1];
    return {centre-relShift*sz,relSize*sz};
}
SquareF             cFaceRegionFrontal11(Vec2Fs const & lms)
{
    FGASSERT(lms.size() == 11);
    return cFaceRegion(lms,{0.8f,0.9f},1.6f);
}
SquareF             cFaceRegionProfileL9(Vec2Fs const & lms)
{
    FGASSERT(lms.size() == 9);
    return cFaceRegion(lms,{0.6f,0.8f},1.5f);
}
SquareF             cFaceRegionProfileR9(Vec2Fs const & lms)
{
    FGASSERT(lms.size() == 9);
    return cFaceRegion(lms,{0.9f,0.8f},1.5f);
}

FaceLmsV3           parseLmsV3(Vec2Fs const & pacs)
{
    FaceLmsV3           ret;
    auto                ltX = [](Vec2F l,Vec2F r){return l[0] < r[0]; };
    auto                ltY = [](Vec2F l,Vec2F r){return l[1] < r[1]; };
    Vec2Fs              ptsY = sortAll(pacs,ltY);
    if (ptsY.size() == 9) {          // profile
        bool                right = ptsY[8][0] < ptsY[7][0];        // compare chin front & jaw bottom X
        ret.lms.emplace_back(FaceLm::lipUpperFront,ptsY[5]);
        ret.lms.emplace_back(FaceLm::lipLowerFront,ptsY[6]);
        ret.lms.emplace_back(FaceLm::chinFront,ptsY[7]);
        ret.lms.emplace_back(FaceLm::jawBottom,ptsY[8]);
        Vec2Fs              sv05X = sortAll(cSubvec(ptsY,0,5),ltX);
        if (right) {
            ret.view = FaceView::right;
            ret.lms.emplace_back(FaceLm::exocanthionR,sv05X[0]);
            ret.lms.emplace_back(FaceLm::noseTip,sv05X[4]);
        }
        else {
            ret.view = FaceView::left;
            ret.lms.emplace_back(FaceLm::noseTip,sv05X[0]);
            ret.lms.emplace_back(FaceLm::exocanthionL,sv05X[4]);
        }
        Vec2Fs              sv05X13Y = sortAll(cSubvec(sv05X,1,3),ltY);
        ret.lms.emplace_back(FaceLm::sellion,sv05X13Y[0]);
        ret.lms.emplace_back(FaceLm::noseBridgeMiddle,sv05X13Y[1]);
        ret.lms.emplace_back(FaceLm::subNasale,sv05X13Y[2]);
    }
    else if (ptsY.size() == 11) {    // frontal
        ret.view = FaceView::front;
        ret.lms.emplace_back(FaceLm::chinBottom,ptsY.back());
        Vec2Fs              sv64X = sortAll(cSubvec(ptsY,6,4),ltX);
        ret.lms.emplace_back(FaceLm::jawOuterR,sv64X[0]);
        ret.lms.emplace_back(FaceLm::mouthCornerR,sv64X[1]);
        ret.lms.emplace_back(FaceLm::mouthCornerL,sv64X[2]);
        ret.lms.emplace_back(FaceLm::jawOuterL,sv64X[3]);
        Vec2Fs              sv06X = sortAll(cSubvec(ptsY,0,6),ltX);
        ret.lms.emplace_back(FaceLm::cheekboneR,sv06X[0]);
        ret.lms.emplace_back(FaceLm::cheekboneL,sv06X[5]);
        Vec2Fs              sv06X14Y = sortAll(cSubvec(sv06X,1,4),ltY);
        Vec2Fs              sv06X14Y22X = sortAll(cSubvec(sv06X14Y,2,2),ltX);
        ret.lms.emplace_back(FaceLm::alarOuterR,sv06X14Y22X[0]);
        ret.lms.emplace_back(FaceLm::alarOuterL,sv06X14Y22X[1]);
        Vec2Fs              sv06X14Y02X = sortAll(cSubvec(sv06X14Y,0,2),ltX);
        ret.lms.emplace_back(FaceLm::pupilCentreR,sv06X14Y02X[0]);
        ret.lms.emplace_back(FaceLm::pupilCentreL,sv06X14Y02X[1]);
    }
    else
        fgThrow("Not a valid number of landmarks for V3");
    return ret;
}

}

// */
