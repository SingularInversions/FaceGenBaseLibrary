//
// Copyright (c) 2023 Singular Inversions Inc.
//
// Authors:     Andrew Beatty
// Created:     Feb 27, 2012
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

struct      FaceLmAtt
{
    FaceLm              type;
    String              label;

    bool                operator==(FaceLm t) const {return (type == t); }
    bool                operator==(String const & s) const {return (label == s); }
};
typedef Svec<FaceLmAtt> FaceLmAtts;

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


}

// */
