//
// Copyright (c) 2023 Singular Inversions Inc.
//
// Authors:     Andrew Beatty
// Created:     Dec 27, 2012
//

#ifndef FGANTHROPOMETRY_HPP
#define FGANTHROPOMETRY_HPP

#include "FgMatrixC.hpp"

namespace Fg {

enum struct         Sagg { left=0, right=1 };   // subject's saggital left or right half (not viewer's)
inline size_t constexpr saggSize() {return 2; }
inline size_t       toIdx(Sagg s) {return (s==Sagg::left) ? 0 : 1; }
inline Sagg         toSagg(size_t idx) {FGASSERT(idx<2); return (idx==0) ? Sagg::left : Sagg::right; }
inline char         toChar(Sagg s) {return (s==Sagg::left) ? 'L' : 'R'; }
inline Arr<Sagg,2> constexpr getSaggVals() {return {Sagg::left,Sagg::right}; }

enum struct Gender { female, male };
Svec<Pair<Gender,char> >    cGenderCharList();  // maps to { f, m } resp.

// 'Other' inludes mixed:
enum struct RaceGroup { african, caucasian, eAsian, sAsian, other };
Svec<Pair<RaceGroup,char> > cRaceCharList();    // maps to { a, c, e, s, o } resp.

enum struct FaceView { front=0, left=1, right=2, };
typedef Svec<FaceView>  FaceViews;
std::ostream &      operator<<(std::ostream &,FaceView);

enum struct     FaceLm {
    endocanthionL,      endocanthionR,
    exocanthionL,       exocanthionR,
    lidLowerCentreL,    lidLowerCentreR,
    lidUpperCentreL,    lidUpperCentreR,
    lidLowerInnerHalfL, lidLowerInnerHalfR,
    lidLowerOuterHalfL, lidLowerOuterHalfR,
    lidUpperInnerHalfL, lidUpperInnerHalfR,
    lidUpperOuterHalfL, lidUpperOuterHalfR,
    pupilCentreL,       pupilCentreR,
    sellion,
    cheekboneL,         cheekboneR,
    alarOuterL,         alarOuterR,
    noseTip,
    subNasale,
    mouthCornerL,       mouthCornerR,
    cristaPhiltriL,     cristaPhiltriR,
    labialeSuperius,
    labialeInferius,
    mouthCentre,
    supramentale,
    chinBottom,
};
typedef Svec<FaceLm>    FaceLms;
std::ostream &      operator<<(std::ostream &,FaceLm);
template<> Opt<FaceLm>  fromStr<FaceLm>(String const &);

struct      FaceLm2D                // landmark in an image
{
    Vec2F               pos;        // position in image in image raster coordinate system (IRCS)
    FaceLm              lm;

    FaceLm2D() {}
    FaceLm2D(Vec2F p,FaceLm l) : pos{p}, lm{l} {}

    bool                operator==(FaceLm flm) const {return (flm==lm); }
};
typedef Svec<FaceLm2D>  FaceLms2D;

FaceLms                 getLmsEyelid();         // 4 per eye; canthii plus upper and lower centres
FaceLms                 getLmsEyelidExt();      // 8 per eye; above plus inner/outer lower/upper half points
FaceLms                 getLmsMouth();          // 6; corners, cristaPhiltris, superius and inferius
FaceLms                 getEyeGuideLms(Sagg LR);    // L/R lid LMS except for endocanthion

Strings                 toLabels(FaceLms const &);  // preserves order

Strings                 getLmStrsEyelid();      // 8 per eye; canthii plus 3 upper and lower lid points
Strings                 getLmStrsMouth();       // 6 = 2 corners + 3 upper & 1 lower. In clockwise order.
Strings                 getLmStrsNose();           // 4 = tip, base, outer alar L & R 

}

#endif
