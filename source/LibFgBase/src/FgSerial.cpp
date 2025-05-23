//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgSerial.hpp"
#include "MurmurHash2.h"
#include "FgCommand.hpp"
#include "FgParse.hpp"
#include "FgMath.hpp"

using namespace std;

namespace Fg {

uint64              treeHash(Uint64s const & hashes)
{
    FGASSERT(hashes.size() < scast<size_t>(lims<int>::max() / 8));
    int                     len = scast<int>(hashes.size() * 8);
    string                  msg; msg.reserve(len);
    for (uint64 hash : hashes)
        msg.append(reinterpret_cast<char *>(&hash),8);
    // std::hash is not deterministic (across standard libraries or CPU architectures) so cannot be used here.
    // Don't use MurmurHash3 as it gives different results on x86 and x86_64 for its only hash larger than
    // 32 (which happens to be 128). The see was chosen at random.
    return MurmurHash64A(reinterpret_cast<void const *>(msg.data()),len,0x0B779664AC6C80E1ULL);
}

void                testHash(CLArgs const &)
{
    uint64                  in0 = 0x00C89C66E406A689ULL,        // Chosen at random
                            in1 = 0xE46B92AF98E1D9CCULL,        // "
                            out0 = treeHash({in0,in1}),
                            out1 = treeHash({in1,in0});
    fgout
        << fgnl << "Hash of 2 values not symmetric: "
        << toHexString(out0) << " != " << toHexString(out1);
    // Test determinism on all platforms:
    FGASSERT(out0 == 0x960D9C0EDFDE4928ULL);
    FGASSERT(out1 == 0x888931C0760EFC53ULL);
}

Strings             splitCommas(String const & csvs)
{
    return splitChar(csvs,',');
}

String              reflectToTxt(std::any const & node,String const & indent)
{
    size_t constexpr    maxLen = 512ULL;
    FGASSERT(node.has_value());
    if (node.type() == typeid(String)) {
        String              str = any_cast<String>(node);
        if (contains(str,' '))  // use quotes because there are spaces. TODO: add support for including quotes in a string.
            return "\"" + str + "\"";
        else
            return str;
    }
    else if (node.type() == typeid(RflArray)) {
        RflArray            arr = any_cast<RflArray>(node);
        Strings             strs;
        for (any const & elem : arr.elems)
            strs.push_back(reflectToTxt(elem,indent+"  "));
        size_t              sz {0};
        for (String const & ms : strs)
            sz += ms.size();
        if (sz < maxLen)
            return "[ " + cat(strs," ") + " ]";
        else
            return "[ " + cat(strs,indent) + indent + "]";
    }
    else if (node.type() == typeid(RflStruct)) {
        RflStruct           arr = any_cast<RflStruct>(node);
        String              indent2 = indent+"  ";
        String              ret = indent + "{ ";
        for (RflMember const & memb : arr.members) {
            String              val = reflectToTxt(memb.object,indent2);
            if (memb.name.size() + val.size() < maxLen)
                ret += indent2 + memb.name + ": " + val;
            else
                ret += indent2 + memb.name + ":" + indent2 + val;
        }
        return ret + indent + "}";
    }
    else
        fgThrow("reflectToTxt unexpected type",node.type().name());
    return {};
}

String              reflectToText(std::any const & node)
{
    return reflectToTxt(node,"\n");
}

std::any            stringsToReflect(Strings const & tokens,size_t & cnt)
{
    FGASSERT(!tokens.empty());
    String const &      tok = tokens[cnt++];        // can be empty if it's a string
    if (tok == "[") {
        RflArray            arr;
        while (tokens[cnt] != "]")
            arr.elems.push_back(stringsToReflect(tokens,cnt));
        ++cnt;
        return arr;
    }
    else if (tok == "{") {
        RflStruct           strct;
        while (tokens[cnt] != "}") {
            ++cnt;          // must increment here since evaluation order below not specified:
            RflMember           memb {
                any_cast<String>(tokens[cnt-1]),
                stringsToReflect(tokens,cnt)
            };
            strct.members.push_back(memb);
        }
        ++cnt;
        return strct;
    }
    else {              // must be builtin, keep as string:
        return tok;
    }
}

std::any            textToReflect(String const & txt)
{
    FGASSERT(!txt.empty());
    size_t              cnt {0};
    return stringsToReflect(splitWhitespace(txt),cnt);
}

void                dsrlzSizet_(Bytes const & ser,size_t & pos,size_t & val)
{
        uint64              sz;
        dsrlzRaw_(ser,pos,sz);
#ifndef FG_64
        FGASSERT(sz <= lims<size_t>::max());
#endif
        val = scast<size_t>(sz);
}

Bytes               stringToBytes(String const & str)
{
    size_t              S = str.size();
    Bytes               ret (S);
    memcpy(&ret[0],&str[0],S);
    return ret;
}

String              bytesToString(Bytes const & ser)
{
    size_t              S = ser.size();
    String              ret; ret.resize(S);
    memcpy(&ret[0],&ser[0],S);
    return ret;
}

// encode booleans as uchar with values 0 or 1:
void                srlz_(bool v,Bytes & s)
{
    uchar           b = v ? 1 : 0;
    srlzRaw_(b,s);
}
void                dsrlz_(Bytes const & s,size_t & p,bool & v)
{
    uchar           b;
    dsrlzRaw_(s,p,b);
    v = (b == 1);
}
void                dsrlz_(Bytes const & s,size_t & p,long & v)
{
    int64           t;
    dsrlzRaw_(s,p,t);
    FGASSERT(t >= std::numeric_limits<long>::lowest());
    FGASSERT(t <= std::numeric_limits<long>::max());
    v = static_cast<long>(t);
}
void                dsrlz_(Bytes const & s,size_t & p,unsigned long & v)
{
    uint64          t;
    dsrlzRaw_(s,p,t);
    FGASSERT(t <= std::numeric_limits<unsigned long>::max());
    v = static_cast<unsigned long>(t);
}
void                srlz_(String const & str,Bytes & ser)
{
    size_t              S = str.size();
    srlzSizet_(S,ser);
    if (S > 0) {
        size_t              B = ser.size();
        ser.resize(B+S);
        memcpy(&ser[B],&str[0],S);
    }
}
void                dsrlz_(Bytes const & ser,size_t & off,String & str)
{
    size_t              S;
    dsrlzSizet_(ser,off,S);
    str.resize(S);
    if (S > 0) {
        FGASSERT(off+S <= ser.size());
        memcpy(&str[0],&ser[off],S);
        off += S;
    }
}

namespace {

template<class T>
void                testSerialBinT(T val)
{
    FGASSERT1(val == dsrlz<T>(srlz(val)),typeid(T).name());
}

void                testSerialBin(CLArgs const &)
{
    testSerialBinT<int>(-42);
    testSerialBinT<uint>(42);
    testSerialBinT<long>(-42);
    testSerialBinT<unsigned long>(42);
    testSerialBinT<long long>(-42);
    testSerialBinT<unsigned long long>(42);
    testSerialBinT<float>(pi);
    testSerialBinT<double>(pi);
    testSerialBinT<String>("forty two");
    testSerialBinT<Arr<String,2>>({"forty two","purple haze"});
    testSerialBinT<Strings>({"forty two","purple haze"});
    
    struct          S {
        int             a;
        String          b;
        FG_SER(a,b)
        bool            operator==(S const & r) const {return ((a==r.a) && (b==r.b)); }
    };
    S               s {-42,"hello"};
    testSerialBinT(s);
}

void                testTypenames(CLArgs const &)
{
    Strings         tns;
    int             a = 5;
    double          b = 3.14;
    typeNames_(tns,a,b);
    fgout << fgnl << tns;
}

void                testReflect(CLArgs const &)
{
    struct      B
    {
        bool            hired;
        Uint64s         codes;
        FG_SER(hired,codes)
        FG_EQ_M2(B,hired,codes)
    };
    struct      A
    {
        String          name;
        size_t          age;
        float           weight;
        Arr3UI          dims;
        B               status;
        FG_SER(name,age,weight,dims,status)
        FG_EQ_M5(A,name,age,weight,dims,status)
    };
    Svec<A>             data {
        {"John",42,212.7,{6,2,1},{true,{}}},
        {"Mary",27,123.4,{5,1,2},{false,{23947324,20345534}}},
        {"11",12,110,{4,1,1},{true,{1130046}}},
    };
    any                 node = toReflect(data);
    String              text = reflectToText(node);
    fgout << fgnl << splitLines(text);
    Svec<A>             test0;
    fromReflect_(node,test0);
    FGASSERT(test0 == data);
    Svec<A>             test1;
    fromReflect_(textToReflect(text),test1);
    fgout << fgnl << splitLines(reflectToText(toReflect(test1)));
    FGASSERT(test1 == data);
}

void                testRefl2(CLArgs const &)
{
    struct              A
    {
        int             sval;
        Svec<uint>      uvals;
        FG_MNAMES(sval,uvals)
        FG_TOREFL(sval,uvals)
        FG_FRMRFL(sval,uvals)

        bool            operator==(A const & r) const {return ((sval==r.sval) && (uvals==r.uvals)); }
    };
    A               a {-42,{3,5,8}};
    String          ser = srlzText(a);
    fgout << fgnl << ser;
    A               b = dsrlzText<A>(ser);
    FGASSERT(a==b);
}

}

void                testSerial(CLArgs const & args)
{
    Cmds            cmds {
        {testSerialBin,"binary","binary serialization / deserialization"},
        {testReflect,"reflect","object reflection to std::any name-value tree"},
        {testRefl2,"refl2",""},
        {testTypenames,"type","type name as string"},
    };
    doMenu(args,cmds,true);
}

}
