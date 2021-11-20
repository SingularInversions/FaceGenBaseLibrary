//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Basic FBX ASCII export
//
// FBX CONVERTER 2013.3 Win64
//  * Use this to convert between FBX ASCII and binary
//  * Comes with several example files (click 'Add...')
//  * GUI doesn't work so use CL in 'bin' subdir
//


#include "stdafx.h"
#include "FgStdStream.hpp"
#include "FgImage.hpp"
#include "FgFileSystem.hpp"
#include "Fg3dMesh.hpp"
#include "Fg3dMeshIo.hpp"
#include "FgCommand.hpp"
#include "FgTestUtils.hpp"
#include "FgHex.hpp"
#include "FgAny.hpp"
#include "FgParse.hpp"
#include "FgSyntax.hpp"

using namespace std;

namespace Fg {

namespace {

size_t          idModel(size_t mm) {return mm*100+1; }
size_t          idGeometry(size_t mm) {return mm*100+2; }
size_t          idGeoExp(size_t mm,size_t ee) {return ee*1000+mm*100+7; }
size_t          idDeformer(size_t mm,size_t ee) {return ee*1000+mm*100+8; }
size_t          idSubdeformer(size_t mm,size_t ee) {return ee*1000+mm*100+9; }
size_t          idMaterial(size_t mm,size_t tt) {return mm*100+tt*10+3; }
size_t          idTexture(size_t mm,size_t tt) {return mm*100+tt*10+4; }
size_t          idVideo(size_t mm,size_t tt) {return mm*100+tt*10+5; }
string          nmVideo(size_t mm,size_t tt) {return "\"Video::Video"+toStr(mm)+"_"+toStr(tt)+"\""; }

}   // namespace

void
saveFbxAscii(String8 const & filename,Meshes const & meshes,String imgFormat)
{
    FGASSERT(!meshes.empty());
    Path            path(filename);
    path.ext = "fbx";
    Ofstream        ofs(path.str());
    ofs.precision(7);
    ofs <<
R"(; FBX 7.4.0 project file
; Created by FaceGen
FBXHeaderExtension:  {
    FBXHeaderVersion: 1003
    FBXVersion: 7400
}
Definitions:  {
    Version: 100
    ObjectType: "Model" {
    }
    ObjectType: "Geometry" {
    }
    ObjectType: "Material" {
    }
    ObjectType: "Texture" {
    }
    ObjectType: "Video" {
    }
    ObjectType: "Deformer" {
    }
}
Objects:  {
)";
    for (size_t mm=0; mm<meshes.size(); ++mm) {
        Mesh const &    mesh = meshes[mm];
        ofs <<
R"(    Model: )" << idModel(mm) << R"(, "Model::)" << mesh.name << R"(", "Mesh" {
        Version: 232
        Properties70:  {
            P: "ScalingMax", "Vector3D", "Vector", "",0,0,0
            P: "DefaultAttributeIndex", "int", "Integer", "",0
            P: "Lcl Translation", "Lcl Translation", "", "A",0,0,0
        }
        Shading: T
        Culling: "CullingOff"
    }
    Geometry: )" << idGeometry(mm) << R"(, "Geometry::)" << mesh.name << R"(" , "Mesh" {
        Vertices: *)" << mesh.verts.size()*3 << R"( {
            a: )";
        for (size_t vv=0; vv<mesh.verts.size(); ++vv) {
            Vec3F           v = mesh.verts[vv];
            if (vv > 0)
                ofs << ",";
            ofs << v[0] << "," << v[1] << "," << v[2];
        }
        ofs << "\n"
            "        }\n"
            "        PolygonVertexIndex: *" << mesh.numTris()*3+mesh.numQuads()*4 << " {\n"
            "            a: ";
        bool        start = true;
        for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
            Surf const & surf = mesh.surfaces[ss];
            for (size_t tt=0; tt<surf.tris.size(); ++tt) {
                Vec3UI   i = surf.tris.posInds[tt];
                if (start)
                    start = false;
                else
                    ofs << ",";
                ofs << i[0] << "," << i[1] << "," << int(~i[2]);     // bitwise negation of last index WTF
            }
            for (size_t tt=0; tt<surf.quads.size(); ++tt) {
                Vec4UI   i = surf.quads.posInds[tt];
                if (start)
                    start = false;
                else
                    ofs << ",";
                ofs << i[0] << "," << i[1] << "," << i[2] << "," << int(~i[3]);
            }
        }
        ofs << "\n"
            "        }\n"
            "        GeometryVersion: 124\n"
            "        LayerElementNormal: 0 {\n"
            "            Version: 102\n"
            "            Name: \"\"\n"
            "            MappingInformationType: \"ByVertice\"\n"
            "            ReferenceInformationType: \"Direct\"\n"
            "            Normals: *" << mesh.verts.size()*3 << " {\n"
            "                a: ";
        MeshNormals     norms = cNormals(mesh);
        for (size_t vv=0; vv<mesh.verts.size(); ++vv) {
            Vec3F    n = norms.vert[vv];
            if (vv > 0)
                ofs << ",";
            ofs << n[0] << "," << n[1] << "," << n[2];
        }
        ofs << "\n"
            "            }\n"
            "        }\n"
            "        LayerElementUV: 0 {\n"
            "            Version: 101\n"
            "            Name: \"UVs" << mm << "\"\n"
            "            MappingInformationType: \"ByPolygonVertex\"\n"
            "            ReferenceInformationType: \"IndexToDirect\"\n"
            "            UV: *" << mesh.uvs.size()*2 << " {\n"
            "                a: ";
        for (size_t uu=0; uu<mesh.uvs.size(); ++uu) {
            Vec2F    uv = mesh.uvs[uu];
            if (uu > 0)
                ofs << ",";
            ofs << uv[0] << "," << uv[1];
        }
        ofs << "\n"
            "            }\n"
            "            UVIndex: *" << mesh.numTris()*3+mesh.numQuads()*4 << " {\n"
            "                a: ";
        start = true;
        for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
            Surf const & surf = mesh.surfaces[ss];
            for (size_t tt=0; tt<surf.tris.uvInds.size(); ++tt) {
                Vec3UI   i = surf.tris.uvInds[tt];
                if (start)
                    start = false;
                else
                    ofs << ",";
                ofs << i[0] << "," << i[1] << "," << i[2];
            }
            for (size_t tt=0; tt<surf.quads.uvInds.size(); ++tt) {
                Vec4UI   i = surf.quads.uvInds[tt];
                if (start)
                    start = false;
                else
                    ofs << ",";
                ofs << i[0] << "," << i[1] << "," << i[2] << "," << i[3];
            }
        }
        ofs << "\n"
            "            }\n"
            "        }\n";
        ofs <<
            "        LayerElementMaterial: 0 {\n"
            "            Version: 101\n"
            "            Name: \"\"\n"
            "            MappingInformationType: \"ByPolygon\"\n"
            "            ReferenceInformationType: \"IndexToDirect\"\n"
            "            Materials: *" << mesh.numPolys() << " {\n"
            "                a: ";
        start = true;
        for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
            Surf const & surf = mesh.surfaces[ss];
            size_t              num = surf.tris.uvInds.size() + surf.quads.uvInds.size();
            for (size_t ii=0; ii<num; ++ii) {
                if (start)
                    start = false;
                else
                    ofs << ",";
                ofs << ss;
            }
        }
        ofs << "\n"
            "            }\n"
            "        }\n"
            "        Layer: 0 {\n"
            "            Version: 100\n"
            "            LayerElement:  {\n"
            "                Type: \"LayerElementNormal\"\n"
            "                TypedIndex: 0\n"
            "            }\n"
            "            LayerElement:  {\n"
            "                Type: \"LayerElementUV\"\n"
            "                TypedIndex: 0\n"
            "            }\n"
            "            LayerElement:  {\n"
            "                Type: \"LayerElementMaterial\"\n"
            "                TypedIndex: 0\n"
            "            }\n"
            "        }\n";
        ofs <<
            "    }\n";
        for (size_t ee=0; ee<mesh.numMorphs(); ++ee) {
            IndexedMorph  morph = mesh.getMorphAsIndexedDelta(ee);
            ofs << 
                "    Geometry: " << idGeoExp(mm,ee) << ", \"Geometry::" << morph.name << "\", \"Shape\" {\n"
                "        Version: 100\n"
                "        Indexes: *" << morph.ivs.size() << " {\n"
                "            a: ";
            for (size_t ii=0; ii<morph.ivs.size(); ++ii) {
                if (ii > 0)
                    ofs << ",";
                ofs << morph.ivs[ii].idx;
            }
            ofs << "\n"
                "        }\n"
                "        Vertices: *" << morph.ivs.size()*3 << " {\n"
                "            a: ";
            for (size_t ii=0; ii<morph.ivs.size(); ++ii) {
                if (ii > 0)
                    ofs << ",";
                Vec3F    v = morph.ivs[ii].vec;
                ofs << v[0] << "," << v[1] << "," << v[2];
            }
            ofs << "\n"
                "        }\n"
                "    }\n"
                "    Deformer: " << idDeformer(mm,ee) << ", \"Deformer::" << morph.name << "\", \"BlendShape\" {\n"
                "        Version: 100\n"
                "    }\n"
                "    Deformer: " << idSubdeformer(mm,ee) << ", \"SubDeformer::" << morph.name << "\", \"BlendShapeChannel\" {\n"
                "        Version: 100\n"
                "        DeformPercent: 0\n"
                "        FullWeights: *1 {\n"
                "            a: 10\n"
                "        }\n"
                "    }\n";
        }
    }
    for (size_t mm=0; mm<meshes.size(); ++mm) {
        Mesh const &    mesh = meshes[mm];
        for (size_t tt=0; tt<mesh.surfaces.size(); ++tt) {
            ofs <<
                "    Material: " << idMaterial(mm,tt) << ", \"Material::Material" << mm << "_" << tt << "\", \"\" {\n"
                "        Version: 102\n"
                "        ShadingModel: \"phong\"\n"
                "        MultiLayer: 0\n"
                "        Properties70:  {\n"
                "            P: \"AmbientColor\", \"Color\", \"\", \"A\",0,0,0\n"
                "            P: \"DiffuseColor\", \"Color\", \"\", \"A\",1,1,1\n"
                "            P: \"Emissive\", \"Vector3D\", \"Vector\", \"\",0,0,0\n"
                "            P: \"Ambient\", \"Vector3D\", \"Vector\", \"\",0,0,0\n"
                "            P: \"Diffuse\", \"Vector3D\", \"Vector\", \"\",1,1,1\n"
                "            P: \"Opacity\", \"double\", \"Number\", \"\",1\n"
                "        }\n"
                "    }\n";
            String8    texBaseExt = path.base + toStr(mm) + "_" + toStr(tt) + "." + imgFormat;
            if (mesh.surfaces[tt].material.albedoMap)
                saveImage(path.dir() + texBaseExt,*mesh.surfaces[tt].material.albedoMap);
            ofs <<
                "    Texture: " << idTexture(mm,tt) << ", \"Texture::Texture" << mm << "_" << tt << "\", \"TextureVideoClip\" {\n"
                "        Type: \"TextureVideoClip\"\n"
                "        Version: 202\n"
                "        TextureName: \"Texture::Texture" << mm << "_" << tt << "\"\n"
                "        Properties70:  {\n"
                "            P: \"CurrentTextureBlendMode\", \"enum\", \"\", \"\",0\n"
                "            P: \"UVSet\", \"KString\", \"\", \"\", \"UVs" << mm << "\"\n"
                "            P: \"UseMaterial\", \"bool\", \"\", \"\",1\n"
                "        }\n"
                "        Media: " << nmVideo(mm,tt) << "\n"
                "        FileName: \"" << texBaseExt << "\"\n"
                "        RelativeFilename: \"" << texBaseExt << "\"\n"
                "        ModelUVTranslation: 0,0\n"
                "        ModelUVScaling: 1,1\n"
                "        Texture_Alpha_Source: \"None\"\n"
                "        Cropping: 0,0,0,0\n"
                "    }\n";
            ofs <<
                "    Video: " << idVideo(mm,tt) << ", " << nmVideo(mm,tt) << ", \"Clip\" {\n"
                "        Type: \"Clip\"\n"
                "        Properties70:  {\n"
                "            P: \"Path\", \"KString\", \"XRefUrl\", \"\", \"" << texBaseExt << "\"\n"
                "        }\n"
                "        UseMipMap: 0\n"
                "        Filename: \"" << texBaseExt << "\"\n"
                "        RelativeFilename: \"" << texBaseExt << "\"\n"
                "    }\n";
        }
    }
    ofs <<
        "}\n"
        "Connections: {\n";
    for (size_t mm=0; mm<meshes.size(); ++mm) {
        Mesh const &    mesh = meshes[mm];
        ofs << "    C: \"OO\", " << idModel(mm) << ",0\n";
        for (size_t ee=0; ee<mesh.numMorphs(); ++ee) {
            ofs << "    C: \"OO\", " << idGeoExp(mm,ee) << "," << idSubdeformer(mm,ee) << "\n";
            ofs << "    C: \"OO\", " << idSubdeformer(mm,ee) << "," << idDeformer(mm,ee) << "\n";
            ofs << "    C: \"OO\", " << idDeformer(mm,ee) << "," << idGeometry(mm) << "\n";

        }
    }
    for (size_t mm=0; mm<meshes.size(); ++mm) {
        Mesh const &    mesh = meshes[mm];
        ofs << "    C: \"OO\", " << idGeometry(mm) << "," << idModel(mm) << "\n";
        for (size_t tt=0; tt<mesh.surfaces.size(); ++tt) {
            ofs <<
                "    C: \"OO\", " << idMaterial(mm,tt) << "," << idModel(mm) << "\n"
                "    C: \"OP\", " << idTexture(mm,tt) << "," << idMaterial(mm,tt) << ", \"DiffuseColor\"\n"
                "    C: \"OO\", " << idVideo(mm,tt) << "," << idTexture(mm,tt) << "\n";
            if (mesh.surfaces[tt].material.albedoMap && usesAlpha(*mesh.surfaces[tt].material.albedoMap))
                ofs <<
                "    C: \"OP\", " << idTexture(mm,tt) << "," << idMaterial(mm,tt) << ", \"TransparentColor\"\n";
        }
    }
    ofs << "}\n";
}
void
testSaveFbxAscii(CLArgs const & args)
{
    FGTESTDIR
    String8         dd = dataDir();
    string          rd = "base/";
    Mesh            mouth = loadTri(dd+rd+"Mouth.tri");
    mouth.surfaces[0].setAlbedoMap(loadImage(dd+rd+"MouthSmall.png"));
    Mesh            glasses = loadTri(dd+rd+"Glasses.tri");
    glasses.surfaces[0].setAlbedoMap(loadImage(dd+rd+"Glasses.tga"));
    saveFbxAscii("meshExportFbx",{mouth,glasses});
    regressFileRel("meshExportFbx.fbx","base/test/");
    regressFileRel("meshExportFbx0_0.png","base/test/");
    regressFileRel("meshExportFbx1_0.png","base/test/");
}

String      zlibInflate(String const & compressed,size_t sz);

namespace {

template<typename T>
Any         readBin(Ifstream & ifs)
{
    T           v = ifs.readb<T>();
    //fgout << v << " ";
    return Any{v};
}
template<typename T>
Any         readBinArray(Ifstream & ifs)
{
    size_t          arrayLen = ifs.readb<int32>(),
                    encoding = ifs.readb<int32>(),
                    compressedLen = ifs.readb<int32>(),
                    byteLen = arrayLen * sizeof(T);
    //fgout << arrayLen << " ";
    if (arrayLen == 0)              // just in case
        return Any{Svec<T>{}};
    if (compressedLen == 0)
        fgThrow("FBX file corrupt; compressedLen == 0");
    String          data;
    if (encoding == 0)              // not compressed:
        data = ifs.readChars(byteLen);
    else                            // compressed:
        data = zlibInflate(ifs.readChars(compressedLen),byteLen);
    Svec<T>         arr (arrayLen);
    memcpy(&arr[0],&data[0],byteLen);
    return Any{arr};
}
template<typename T>
void        writeBinArray_(Any const & in,String & out)
{
    Svec<T> const &     arr = in.as<Svec<T>>();
    srlzRaw_(uint32(arr.size()),out);
    srlzRaw_(uint32{0},out);                    // uncompressed encoding
    srlzRaw_(uint32(arr.size()*sizeof(T)),out); // 'compressed length' just byte length
    srlzRaw_(arr,out);                          // Svec partial specialization
}

// the node record 'PropertyListLen' allows you to skip all properties without parsing,
// but there is no way to skip individual properties
struct      Property
{
    Any             data;           // type given above
    char            typeChar;       // the FBX type character code
};

struct      RecordRaw
{
    uint32                  endOffset,          // from the input file, not relevent to output
                            numProperties;
    String                  name;
    Svec<Property>          propertyList;
    Svec<Sptr<RecordRaw>>   subs;

    RecordRaw(uint32 e,uint32 n) : endOffset{e}, numProperties{n} {}

    Any const &
    getPropertyData(size_t idx,char typeChar) const
    {
        if (idx >= propertyList.size())
            fgThrow("FBX record does not have enough properties",name+":"+toStr(idx));
        Property const &    prop = propertyList[idx];
        if (prop.typeChar != typeChar)
            fgThrow("FBX property not of expected type",name+":"+toStr(idx)+":"+typeChar+"!="+prop.typeChar);
        return prop.data;
    }

    Any &
    getPropertyData(size_t idx,char typeChar)
    {
        if (idx >= propertyList.size())
            fgThrow("FBX record does not have enough properties",name+":"+toStr(idx));
        Property &          prop = propertyList[idx];
        if (prop.typeChar != typeChar)
            fgThrow("FBX property not of expected type",name+":"+toStr(idx)+":"+typeChar+"!="+prop.typeChar);
        return prop.data;
    }
};

// because 'endOffset' values are absolute, and because vertex arrays are compressed, any change
// in vertex positions will involve changing every subsequent 'endOffset', thus we must parse
// every record (instead of just keeping all subs as a raw block) in order to be able to re-write
// them with updated 'endOffset' values. Luckily this is not true of the property list.
Sptr<RecordRaw>
readBinRecord(Ifstream & ifs)
{
    Sptr<RecordRaw>     ret;
    // read in the data shared by sentinel & actual records:
    uint32              endOffset = ifs.readb<uint32>(),
                        numProperties = ifs.readb<uint32>(),
                        propertyListLen = ifs.readb<uint32>();
    size_t              nameLen = ifs.readb<uchar>();
    if (endOffset == 0)          // sentinel record, no property list or subs, end of current list.
        return ret;
    // actual record:
    ret = make_shared<RecordRaw>(endOffset,numProperties);
    if (nameLen > 0)                // istream does not accept 0 size reads
        ret->name = ifs.readChars(nameLen);
    //fgout << fgnl << ret->name << " ";
    size_t              propertyStartIdx = ifs.tellg();
    for (uint pp=0; pp<numProperties; ++pp) {     // read property list
        Any                 data;
        char                type = ifs.get();
        //fgout << type << ": ";
        if (type == 'Y')
            data = readBin<int16>(ifs);
        else if (type == 'C')
            data = readBin<uchar>(ifs);
        else if (type == 'I')
            data = readBin<int32>(ifs);
        else if (type == 'F')
            data = readBin<float>(ifs);
        else if (type == 'D')
            data = readBin<double>(ifs);
        else if (type == 'L')
            data = readBin<int64>(ifs);
        else if (type == 'f')
            data = readBinArray<float>(ifs);
        else if (type == 'd')
            data = readBinArray<double>(ifs);
        else if (type == 'l')
            data = readBinArray<int64>(ifs);
        else if (type == 'i')
            data = readBinArray<int32>(ifs);
        else if (type == 'b')
            data = readBinArray<uchar>(ifs);
        else if (type == 'S') {
            size_t          len = ifs.readb<uint32>();      // can be empty
            String          str = ifs.readChars(len);
            //fgout << str << " ";
            data = str;
        }
        else if (type == 'R') {
            size_t          len = ifs.readb<uint32>();
            //fgout << len << " ";
            data = ifs.readChars(len);
        }
        else    // can't recover, no way to skip single property w/o parsing
            fgThrow("FBX unknown property type",type);
        ret->propertyList.push_back(Property{data,type});
    }
    if ((size_t(ifs.tellg()) - propertyStartIdx) != propertyListLen)
        fgThrow("FBX invalid property list length",ret->name+":"+toStr(propertyListLen));
    PushIndent          pind;
    // while there is space not accounted for by the 13 terminating nulls (sentinel record):
    while (ifs.tellg()+13LL < endOffset)
        ret->subs.push_back(readBinRecord(ifs));
    // this is necessary because some implementations occasionally add a null record to an empty
    // list so we can never be sure if there is a null record for an empty list:
    ifs.seekg(endOffset);
    return ret;
}
void
writeBinRecord_(Sptr<RecordRaw> const & recPtr,String & out)
{
    String const        sentinel (13,'\0');
    RecordRaw const &   rec = *recPtr;
    size_t              offsetIdx = out.size();
    srlzRaw_(rec.endOffset,out);
    String              plOut;
    for (Property const & prop : rec.propertyList) {
        char                type = prop.typeChar;
        srlzRaw_(type,plOut);
        if (type == 'Y')
            srlzRaw_(prop.data.as<int16>(),plOut);
        else if (type == 'C')
            srlzRaw_(prop.data.as<uchar>(),plOut);
        else if (type == 'I')
            srlzRaw_(prop.data.as<int32>(),plOut);
        else if (type == 'F')
            srlzRaw_(prop.data.as<float>(),plOut);
        else if (type == 'D')
            srlzRaw_(prop.data.as<double>(),plOut);
        else if (type == 'L')
            srlzRaw_(prop.data.as<int64>(),plOut);
        else if (type == 'f')
            writeBinArray_<float>(prop.data,plOut);
        else if (type == 'd')
            writeBinArray_<double>(prop.data,plOut);
        else if (type == 'l')
            writeBinArray_<int64>(prop.data,plOut);
        else if (type == 'i')
            writeBinArray_<int32>(prop.data,plOut);
        else if (type == 'b')
            writeBinArray_<uchar>(prop.data,plOut);
        else if (type == 'S') {
            String const &  str = prop.data.as<String>();       // can be empty
            srlzRaw_(uint32(str.size()),plOut);
            plOut += prop.data.as<String>();
        }
        else if (type == 'R') {
            String const &  str = prop.data.as<String>();
            srlzRaw_(uint32(str.size()),plOut);
            plOut += prop.data.as<String>();
        }
        else
            FGASSERT_FALSE;
    }
    srlzRaw_(rec.numProperties,out);
    srlzRaw_(uint32(plOut.size()),out);
    srlzRaw_(uchar(rec.name.size()),out);
    out += rec.name;
    out += plOut;
    for (auto const & record : rec.subs)
        writeBinRecord_(record,out);
    // sentinels only for non-empty record lists (although some implementations occasionally
    // insert a sentinel for an empty list, I think just at the first sub-level):
    if (!rec.subs.empty())
        out += sentinel;
    // reach back and update the absolute next record position:
    srlzRawOverwrite_(uint32(out.size()),out,offsetIdx);
}

struct      FbxBin
{
    String                  header;     // everything up to version below
    uint32                  version;
    Svec<Sptr<RecordRaw>>   records;
    String                  footer;     // includes the null record at end
};

FbxBin
loadFbxBinRaw(String8 const & filename)
{
    Ifstream        ifs {filename};
    FbxBin          ret;
    ret.header = ifs.readChars(23);
    if (!beginsWith(ret.header,"Kaydara FBX Binary"))
        fgThrow("file is not a Kaydara FBX binary",filename);
    ret.version = ifs.readb<uint32>();
    //fgout << "Version: " << ret.version;
    // the top level is just a NestedList of records with no header:
    while(Sptr<RecordRaw> rp = readBinRecord(ifs))      // will return null ptr for terminating null record
        ret.records.push_back(rp);
    for (;;) {
        char        ch = ifs.readb<char>();
        if (ifs.eof())                                  // flag gets set after reading past end
            break;
        ret.footer.append(1,ch);
    }
    //fgout << fgnl << ret.records.size() << " top level records read"
    //    << fgnl << ret.footer.size() << " footer bytes";
    return ret;
}

void
saveFbxBinRaw(String8 const & filename,FbxBin const & fbx)
{
    String const        sentinel (13,'\0');
    String              blob = fbx.header;
    srlzRaw_(fbx.version,blob);
    for (auto const & record : fbx.records)
        writeBinRecord_(record,blob);
    blob += sentinel;
    blob += fbx.footer;
    saveRaw(blob,filename,false);
}

void
testFbxBin(CLArgs const & args)
{
    if (isAutomated(args))
        return;
    Syntax              syn {args,
        "<in>.fbx <out>.fbx"
    };
    FbxBin              fbx = loadFbxBinRaw(syn.next());
    saveFbxBinRaw(syn.next(),fbx);
}

}

Meshes              loadFbx(String8 const & filename)
{
    Meshes          ret;
    FbxBin          fb = loadFbxBinRaw(filename);
    for (Sptr<RecordRaw> const & rp0 : fb.records) {
        if (rp0->name == "Objects") {
            for (Sptr<RecordRaw> const & rp1 : rp0->subs) {
                if (rp1->name == "Geometry") {
                    String          p2s = rp1->getPropertyData(2,'S').as<String>();
                    if (p2s == "Mesh") {
                        Mesh            mesh;
                        // the name is the initial c-string within a longer description:
                        mesh.name = splitChar(rp1->getPropertyData(1,'S').as<String>(),'\0')[0];
                        mesh.surfaces.resize(1);
                        // only the vertex indices include polygon size information so we need to keep
                        // this for parsing the UV indices:
                        Uints           polySizes;
                        // RCC export seems structured for only 1 surface per vertex list, but Reallusion
                        // uses UV domains to separate:
                        Surf            surf = mesh.surfaces[0];
                        for (Sptr<RecordRaw> const & rp2 : rp1->subs) {
                            if (rp2->name == "Vertices") {
                                Doubles const & dbls = rp2->getPropertyData(0,'d').as<Doubles>();
                                for (size_t ii=0; ii<dbls.size()/3; ++ii) {
                                    mesh.verts.emplace_back(
                                        scast<float>(dbls[ii*3]),
                                        scast<float>(dbls[ii*3+1]),
                                        scast<float>(dbls[ii*3+2])
                                    );
                                }
                            }
                            else if (rp2->name == "PolygonVertexIndex") {
                                Ints const &        ints = rp2->getPropertyData(0,'i').as<Ints>();
                                VArray<int,4>       poly;
                                for (int ii : ints) {
                                    if (ii < 0) {       // last index of a poly signified by XOR'd value
                                        int         jj = ~ii;
                                        if (poly.size() == 2) {
                                            surf.tris.posInds.emplace_back(poly[0],poly[1],jj);
                                            polySizes.push_back(3);
                                        }
                                        else if (poly.size() == 3) {
                                            surf.quads.posInds.emplace_back(poly[0],poly[1],poly[2],jj);
                                            polySizes.push_back(4);
                                        }
                                        else
                                            fgout << fgnl << "WARNING: unhandled poly of size " << poly.size()+1;
                                        poly.clear();
                                    }
                                    else
                                        poly.add(ii);
                                }
                            }
                            else if (rp2->name == "LayerElementUV") {
                                for (Sptr<RecordRaw> const & rp3 : rp2->subs) {
                                    if (rp3->name == "UV") {
                                        Doubles const & dbls = rp3->getPropertyData(0,'d').as<Doubles>();
                                        for (size_t ii=0; ii<dbls.size()/2; ++ii) {
                                            mesh.uvs.emplace_back(
                                                scast<float>(dbls[ii*2]),
                                                scast<float>(dbls[ii*2+1])
                                            );
                                        }
                                    }
                                    else if (rp3->name == "UVIndex") {
                                        if (!(surf.tris.uvInds.empty() && surf.quads.uvInds.empty()))
                                            fgout << fgnl << "WARNING: duplicate UV indices ignored for " << mesh.name;
                                        else {
                                            Ints const &    ints = rp3->getPropertyData(0,'i').as<Ints>();
                                            if (ints.size() == cSum(polySizes)) {
                                                size_t          ii {0};
                                                for (uint sz : polySizes) {
                                                    if (sz == 3)
                                                        surf.tris.uvInds.emplace_back(ints[ii],ints[ii+1],ints[ii+2]);
                                                    else
                                                        surf.quads.uvInds.emplace_back(ints[ii],ints[ii+1],ints[ii+2],ints[ii+3]);
                                                    ii += sz;
                                                }
                                            }
                                            else
                                                fgout << fgnl << "WARNING: UV indices ignored due to size mismatch: "
                                                    << ints.size() << " != " << cSum(polySizes);
                                        }
                                    }
                                }
                            }
                        }
                        mesh.surfaces = splitByUvDomain_(surf,mesh.uvs);
                        ret.push_back(mesh);
                    }
                }
            }
        }
    }
    return ret;
}

void
injectVertsFbxBin(String8 const & inFile,Vec3Fs const & verts,String8 const & outFile)
{
    Floats          flts = flatten(verts);
    FbxBin          fbx = loadFbxBinRaw(inFile);
    size_t          cnt {0};
    for (Sptr<RecordRaw> & rp0 : fbx.records) {
        if (rp0->name == "Objects") {
            for (Sptr<RecordRaw> & rp1 : rp0->subs) {
                if (rp1->name == "Geometry") {
                    String          p2s = rp1->getPropertyData(2,'S').as<String>();
                    if (p2s == "Mesh") {
                        for (Sptr<RecordRaw> & rp2 : rp1->subs) {
                            if (rp2->name == "Vertices") {
                                Doubles &       dbls = rp2->getPropertyData(0,'d').ref<Doubles>();
                                if (flts.size()-cnt < dbls.size())
                                    fgThrow("inject verts FBX too few vertices",verts.size());
                                for (size_t ii=0; ii<dbls.size(); ++ii)
                                    dbls[ii] = flts[cnt++];
                            }
                        }
                    }
                }
            }
        }
    }
    saveFbxBinRaw(outFile,fbx);
    if (cnt < flts.size())
        fgout << fgnl << "WARNING: inject verts FBX too many vertices: " << verts.size();
}

void
testSaveFbx(CLArgs const & args)
{
    Cmds            cmds {
        {testSaveFbxAscii,"ascii",""},
        {testFbxBin,"bin","round-trip FBX binary test"},
    };
    doMenu(args,cmds,true);
}

}

// */
