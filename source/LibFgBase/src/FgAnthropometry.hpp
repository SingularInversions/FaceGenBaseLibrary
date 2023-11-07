//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGANTHROPOMETRY_HPP
#define FGANTHROPOMETRY_HPP

#include "FgGeometry.hpp"

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
    noseBridgeMiddle,
    noseTip,
    subNasale,
    jawOuterL,          jawOuterR,
    mouthCornerL,       mouthCornerR,
    cristaPhiltriL,     cristaPhiltriR,
    labialeSuperius,
    labialeInferius,
    lipUpperFront,      // used in photo profile; anywhere between labialeSuperius and forward centre of upper lip
    lipLowerFront,      // used in photo profile; anywhere between labialeInferius and forward centre of lower lip
    mouthCentre,
    supramentale,
    chinFront,
    chinBottom,
    jawBottom,
};
typedef Svec<FaceLm>    FaceLms;
std::ostream &      operator<<(std::ostream &,FaceLm);
template<> Opt<FaceLm>  fromStr<FaceLm>(String const &);

struct      FaceLmAtt
{
    FaceLm              type;
    String              label;
    FaceLmAtt(FaceLm t,String const & l) : type{t}, label{l} {}

    bool                operator==(FaceLm t) const {return (type == t); }
    bool                operator==(String const & s) const {return (label == s); }
};
typedef Svec<FaceLmAtt> FaceLmAtts;

FaceLmAtts const &  getFaceLmAtts();

template<uint D>
struct      FaceLmPos                // landmark in an image
{
    FaceLm              lm;
    Mat<float,D,1>      pos;        // only positions make sense for landmark label

    FaceLmPos() {}
    FaceLmPos(FaceLm l,Mat<float,D,1> p) : lm{l}, pos{p} {}

    bool                operator==(FaceLm flm) const {return (flm==lm); }
};
template<uint D>
std::ostream &      operator<<(std::ostream & os,FaceLmPos<D> flp)
{
    return os << fgnl << flp.lm << " : " << flp.pos;
}
typedef FaceLmPos<2>        FaceLmPos2F;    // 'pos' is in image raster coordinate system (IRCS)
typedef FaceLmPos<3>        FaceLmPos3F;
typedef Svec<FaceLmPos2F>   FaceLmPos2Fs;
typedef Svec<FaceLmPos3F>   FaceLmPos3Fs;

// converts string names to FaceLm enum values, preserving order:
template<uint D>
Svec<FaceLmPos<D>>  nameVecsToFaceLmPos(Svec<NameVec<float,D>> const & nvs)
{
    Svec<FaceLmPos<D>>      ret; ret.reserve(nvs.size());
    for (NameVec<float,D> const & nv : nvs) {
        FaceLm              lm = findFirst(getFaceLmAtts(),nv.name).type;
        ret.emplace_back(lm,nv.vec);
    }
    return ret;
}

template<uint D>
Svec<NameVec<float,D>>  toNameVecs(Svec<FaceLmPos<D>> const & faceLmPoss)
{
    Svec<NameVec<float,D>>  ret; ret.reserve(faceLmPoss.size());
    for (FaceLmPos<D> const & flp : faceLmPoss) {
        String              name = findFirst(getFaceLmAtts(),flp.lm).label;
        ret.emplace_back(name,flp.pos);
    }
    return ret;
}

FaceLms             getLmsEyelid();         // 4 per eye; canthii plus upper and lower centres
FaceLms             getLmsEyelidExt();      // 8 per eye; above plus inner/outer lower/upper half points
FaceLms             getLmsMouth();          // 6; corners, cristaPhiltris, superius and inferius
FaceLms             getEyeGuideLms(Sagg LR);    // L/R lid LMS except for endocanthion
Strings             toLabels(FaceLms const &);  // preserves order
Strings             getLmStrsEyelid();      // 8 per eye; canthii plus 3 upper and lower lid points
Strings             getLmStrsMouth();       // 6 = 2 corners + 3 upper & 1 lower. In clockwise order.
Strings             getLmStrsNose();           // 4 = tip, base, outer alar L & R 
SquareF             cFaceRegionFrontal11(Vec2Fs const & lmsFrontal11);
SquareF             cFaceRegionProfileL9(Vec2Fs const & lmsProfileL9);
SquareF             cFaceRegionProfileR9(Vec2Fs const & lmsProfileR9);

struct      LmQF
{
    FaceLm          lm;
    MatUT2D         qf;     // quadratic form UT choleksy of lm error distribution (units vary):

    bool            operator==(FaceLm l) const {return l == lm; }
};
typedef Svec<LmQF>  LmQFs;

struct      FaceLmsV3
{
    FaceView            view;
    FaceLmPos2Fs        lms;
};

FaceLmsV3           parseLmsV3(Vec2Fs const & pacs);

}

#endif
