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
        {FaceLm::alarOuterL,"AlaOuterL"},                   {FaceLm::alarOuterR,"AlaOuterR"},
        {FaceLm::noseTip,"NoseTip"},                        // Pronasale
        {FaceLm::subNasale,"SubNasale"},
        {FaceLm::jawOuterL,"JawOuterL"},                    {FaceLm::jawOuterR,"JawOuterR"},
        {FaceLm::mouthCornerL,"MouthCornerL"},              {FaceLm::mouthCornerR,"MouthCornerR"},
        {FaceLm::cristaPhiltriL,"CristaPhiltriL"},          {FaceLm::cristaPhiltriR,"CristaPhiltriR"},
        {FaceLm::labialeSuperius,"LabialeSuperius"},
        {FaceLm::labialeInferius,"LabialeInferius"},
        {FaceLm::mouthCentre,"MouthCentre"},
        {FaceLm::supramentale,"Supramentale"},
        {FaceLm::chinBottom,"ChinBottom"},                  // Gnathion
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
        "AlaOuterL",
        "AlaOuterR",
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

}

// */
