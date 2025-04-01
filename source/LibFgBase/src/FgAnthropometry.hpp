//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGANTHROPOMETRY_HPP
#define FGANTHROPOMETRY_HPP

#include "FgGeometry.hpp"

namespace Fg {

// scoped enum indexed array
template<class T,class E>
struct      ArrE
{
    Arr<T,static_cast<size_t>(E::size)> arr;
    FG_SER(arr)

    T const &           operator[](E e) const
    {
        size_t          idx = scast<size_t>(e);
        FGASSERT(idx < arr.size());
        return arr[idx];
    }
    T &                 operator[](E e)
    {
        size_t          idx = scast<size_t>(e);
        FGASSERT(idx < arr.size());
        return arr[idx];
    }
};

// get all scoped enum values as long as they use default enumeration and end with the symbol 'size':
template<class E>
Svec<E>             getAllE()
{
    return genSvec(scast<size_t>(E::size),[](size_t ii){return scast<E>(ii); });
}

enum struct         Sagg { left, right, size };   // subject's saggital left or right half (not viewer's)
inline size_t       toIdx(Sagg s) {return (s==Sagg::left) ? 0 : 1; }
inline Sagg         toSagg(size_t idx) {FGASSERT(idx<2); return (idx==0) ? Sagg::left : Sagg::right; }
inline char         toChar(Sagg s) {return (s==Sagg::left) ? 'L' : 'R'; }
inline Arr<Sagg,2> constexpr cLeftRight() {return {Sagg::left,Sagg::right}; }

enum struct Gender { female, male, size };
typedef Svec<Gender>    Genders;
inline char         toChar(Gender g) {return (g==Gender::female) ? 'F' : 'M'; }
Svec<Pair<Gender,char> >    cGenderCharList();  // maps to { f, m } resp.
std::ostream &      operator<<(std::ostream &,Gender);

// 'Other' inludes mixed:
enum struct RaceGroup { african, caucasian, eAsian, sAsian, other, size };
typedef Svec<RaceGroup>     RaceGroups;
Svec<Pair<RaceGroup,char> > cRaceCharList();    // maps to { a, c, e, s, o } resp.
std::ostream &      operator<<(std::ostream &,RaceGroup);

enum struct FaceView { front, left, right, size };
typedef Svec<FaceView>  FaceViews;
std::ostream &      operator<<(std::ostream &,FaceView);
char                toChar(FaceView);           // 'F', 'L', 'R'
FaceView            toFaceView(char);           // "

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
    earJoinUpperL,      earJoinUpperR,
    earJoinLowerL,      earJoinLowerR,
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
std::ostream &          operator<<(std::ostream &,FaceLm);
template<> Opt<FaceLm>  fromStr<FaceLm>(String const &);
inline FaceLms          toFaceLms(Strings const & names)     // throws if any of the names are not recognized
{
    return mapCall(names,[](String const & n){return fromStr<FaceLm>(n).value(); });
}

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

template<size_t D>
struct      FaceLmPos                // landmark in an image
{
    FaceLm              lm;
    Mat<float,D,1>      pos;        // only positions make sense for landmark label

    FaceLmPos() {}
    FaceLmPos(FaceLm l,Mat<float,D,1> p) : lm{l}, pos{p} {}

    bool                operator==(FaceLm flm) const {return (flm==lm); }
};
template<size_t D>
std::ostream &      operator<<(std::ostream & os,FaceLmPos<D> flp)
{
    return os << fgnl << flp.lm << " : " << flp.pos;
}
typedef FaceLmPos<2>        FaceLmPos2F;    // 'pos' is in image raster coordinate system (IRCS)
typedef FaceLmPos<3>        FaceLmPos3F;
typedef Svec<FaceLmPos2F>   FaceLmPos2Fs;
typedef Svec<FaceLmPos3F>   FaceLmPos3Fs;

// converts known string face LM names to FaceLm enum values, preserving order. Ignores unknown.
template<size_t D>
Svec<FaceLmPos<D>>  toFaceLmPoss(Svec<NameVec<float,D>> const & nvs)
{
    FaceLmAtts const &  flas = getFaceLmAtts();
    Svec<FaceLmPos<D>>  ret;
    for (NameVec<float,D> const & nv : nvs) {
        size_t          idx = findFirstIdx(flas,nv.name);
        if (idx < flas.size())
            ret.emplace_back(flas[idx].type,nv.vec);
    }
    return ret;
}

template<size_t D>
NameVec<float,D>    toNameVec(FaceLmPos<D> const & faceLmPos)
{
    String              name = findFirst(getFaceLmAtts(),faceLmPos.lm).label;
    return {name,faceLmPos.pos};
}

template<size_t D>
Svec<NameVec<float,D>>  toNameVecs(Svec<FaceLmPos<D>> const & faceLmPoss)
{
    return mapCall(faceLmPoss,toNameVec<D>);
}

FaceLms             getLmsEyelid4();            // 4 per eyelid; canthii plus upper and lower centres
FaceLms             getLmsEyelid8();            // 8 per eye; above plus inner/outer lower/upper half points
FaceLms             getLmsMouth6();             // 6; corners, cristaPhiltris, superius and inferius
Strings             toLabels(FaceLms const &);  // preserves order
Strings             getLmStrsEyelid();          // 8 per eye; canthii plus 3 upper and lower lid points
Strings             getLmStrsMouth();           // 6 = 2 corners + 3 upper & 1 lower. In clockwise order.
Strings             getLmStrsNose();            // 4 = tip, base, outer alar L & R 
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

struct      ViewLmPoss
{
    FaceView            view;
    FaceLmPos2Fs        lms;
};

}

#endif
