//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Nov 29, 2011
//

#include "stdafx.h"
#include "FgStdVector.hpp"
#include "FgOut.hpp"
#include "FgException.hpp"
#include "FgFileSystem.hpp"
#include "FgCommand.hpp"
#include "FgMetaFormat.hpp"
#include "FgCons.hpp"

using namespace std;

static
vector<string>
glob(const string & dir)
{
    vector<string>      ret;
    FgDirectoryContents dc = fgDirectoryContents(dir);
    for (size_t ii=0; ii<dc.filenames.size(); ++ii) {
        string      fn = dc.filenames[ii].as_utf8_string();
        FgPath      p(fn);
        string      ext = p.ext.ascii(),
                    base = p.base.ascii();
        if (((ext == "cpp") || (ext == "c") || (ext == "hpp") || (ext == "h")) &&
            (base[0] != '_'))
            ret.push_back(fn);
    }
    return ret;
}

FgConsSrcGroup::FgConsSrcGroup(const string & baseDir,const string & relDir)
    : dir(relDir), files(glob(baseDir+relDir))
    {}

FgConsProj::FgConsProj(
    const string &          name_,
    const string &          srcBaseDir_,
    const vector<string> &  incDirs_,
    const vector<string> &  defs_,
    const vector<string> &  lnkDeps_,
    uint                    warn_)
    :
    name(name_),
    srcBaseDir(srcBaseDir_),
    srcGroups(fgSvec(FgConsSrcGroup(name_+'/'+srcBaseDir_,""))),
    incDirs(incDirs_),
    defs(defs_),
    lnkDeps(lnkDeps_),
    warn(warn_)
{}

void
FgConsSolution::addDll(
    const string &          name,
    const vector<string> &  incDirs,
    const vector<string> &  lnkDeps,
    const vector<string> &  defs)
{
    if (!fgExists(name))
        fgThrow("Unable to find directory",name);
    FgConsProj  proj(name,"",incDirs,defs,lnkDeps);
    proj.dll = true;
    projects.push_back(proj);
}

void
FgConsSolution::addLib(
    const string &          name,
    const vector<string> &  incDirs,
    const vector<string> &  defs)
{
    if (!fgExists(name))
        fgThrow("Unable to find directory",name);
    FgConsSrcGroup           srcs;
    srcs.files = glob(name+'/');
    FgConsProj         proj(name,"",incDirs,defs,vector<string>());
    proj.srcGroups = fgSvec(srcs);
    projects.push_back(proj);
}

void
FgConsSolution::addLib(
    const string &          name,
    const string &          srcBaseDir,
    const vector<string> &  srcDirs,    // Leave empty for default
    const vector<string> &  incDirs,
    const vector<string> &  defs,
    uint                    warn)
{
    if (!fgExists(name))
        fgThrow("Unable to find directory",name);
    FgConsProj         proj;
    proj.name = name;
    proj.srcBaseDir = srcBaseDir;
    proj.incDirs = incDirs;
    proj.defs = defs;
    proj.warn = warn;
    if (srcDirs.size() == 0)
        proj.srcGroups.push_back(FgConsSrcGroup(name+'/'+srcBaseDir,""));
    else
        for (size_t ii=0; ii<srcDirs.size(); ++ii)
            proj.srcGroups.push_back(FgConsSrcGroup(name+'/'+srcBaseDir,srcDirs[ii]));
    projects.push_back(proj);
}

void
FgConsSolution::addApp(
    const string &          name,
    const vector<string> &  incDirs,
    const vector<string> &  lnkDeps,
    const vector<string> &  defs)
{
    if (!fgExists(name))
        fgThrow("Unable to find directory",name);
    FgConsProj  proj(name,string(),incDirs,defs,lnkDeps);
    proj.app = true;
    projects.push_back(proj);
}

FgConsBase
fgConsBase(bool win,bool nix)
{
    FgConsBase      ret;
    // Build platform solutions in current directory:
    FgConsSolution  & sln = ret.sln;
    sln.win = win;
    FGASSERT(win || nix);
    vector<string>  & defs = ret.defs;
    vector<string>  srcDirs =
        fgSvec<string>(
            "filesystem/src/",
            "serialization/src/",
            "system/src/");
    // This stops boost from automatically flagging the compiler to link to it's default library names.
    // Without this you'll see link errors looking for libs like libboost_filesystem-vc90-mt-gd-1_48.lib:
    defs.push_back("BOOST_ALL_NO_LIB");
    // Suppress command-line warning if the compiler version is more recent than this boost version recognizes:
    defs.push_back("BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE");
    FgStrs          boostDefs = defs;
    if (win) {
        boostDefs.push_back("_CRT_SECURE_NO_DEPRECATE=1");
        boostDefs.push_back("_SCL_SECURE_NO_DEPRECATE=1");
    }
    sln.addLib("LibTpBoost","boost_1_67_0/libs/",srcDirs,fgSvec<string>("boost_1_67_0/"),boostDefs,2);
    vector<string> & incMain = ret.incs,
                   & lnkMain = ret.lnks;
    incMain.push_back("../LibTpBoost/boost_1_67_0/");
    lnkMain.push_back("LibTpBoost");
    // This library is set up such that you run a config script to adapt the source code to
    // the platform (eg. generate config.h and remove other-platform .c files). So a bit of work
    // is required to make it properly source-compatible cross-platform:
    sln.addLib(
        "LibJpegIjg6b","",
        fgSvec<string>(""),
        vector<string>(),
        vector<string>(),
        2);
    incMain.push_back("../LibJpegIjg6b/");
    lnkMain.push_back("LibJpegIjg6b");
    //sln.addLib(
    //    "LibTpTiff",
    //    "tiff-4.0.3/libtiff/",
    //    fgSvec<string>(""),
    //    vector<string>(),
    //    vector<string>(),2);
    //incMain.push_back("../LibTpTiff/tiff-4.0.3/libtiff/");
    //lnkMain.push_back("LibTpTiff");
    // ImageMagick is a PITA.
    // to read floating point TIFF (but breaks gcc bmp read) add the following to 'magic-config.h':
    // #define MAGICKCORE_HDRI_SUPPORT
    FgConsProj     imgk(
        "LibImageMagickCore",
        "ImageMagick-6.6.2/",
        fgSvec(
            string("ImageMagick-6.6.2/"),
            string("ImageMagick-6.6.2/bzlib/"),
            string("ImageMagick-6.6.2/jp2/src/libjasper/include/"),
            string("ImageMagick-6.6.2/png/"),
            string("ImageMagick-6.6.2/tiff/libtiff/"),
            string("ImageMagick-6.6.2/zlib/"),
            string("../LibJpegIjg6b/")),
        fgSvec<string>(
            "_MAGICKLIB",
            "TIFF_PLATFORM_CONSOLE"),
        vector<string>(),
        0);
    imgk.addSrcGroup("bzlib/");
    imgk.addSrcGroup("coders/");
    imgk.addSrcGroup("jp2/src/libjasper/base/");
    imgk.addSrcGroup("jp2/src/libjasper/bmp/");
    imgk.addSrcGroup("jp2/src/libjasper/jp2/");
    imgk.addSrcGroup("jp2/src/libjasper/jpc/");
    imgk.addSrcGroup("jp2/src/libjasper/jpg/");
    imgk.addSrcGroup("jp2/src/libjasper/mif/");
    imgk.addSrcGroup("jp2/src/libjasper/pgx/");
    imgk.addSrcGroup("jp2/src/libjasper/pnm/");
    imgk.addSrcGroup("jp2/src/libjasper/ras/");
    imgk.addSrcGroup("magick/");
    imgk.addSrcGroup("png/");
    imgk.addSrcGroup("zlib/");
    vector<string>  imtf;
    imtf.push_back("tif_aux.c");
    imtf.push_back("tif_close.c");
    imtf.push_back("tif_codec.c");
    imtf.push_back("tif_color.c");
    imtf.push_back("tif_compress.c");
    imtf.push_back("tif_dir.c");
    imtf.push_back("tif_dirinfo.c");
    imtf.push_back("tif_dirread.c");
    imtf.push_back("tif_dirwrite.c");
    imtf.push_back("tif_dumpmode.c");
    imtf.push_back("tif_error.c");
    imtf.push_back("tif_extension.c");
    imtf.push_back("tif_fax3.c");
    imtf.push_back("tif_fax3sm.c");
    imtf.push_back("tif_flush.c");
    imtf.push_back("tif_getimage.c");
    imtf.push_back("tif_jpeg.c");
    imtf.push_back("tif_luv.c");
    imtf.push_back("tif_lzw.c");
    imtf.push_back("tif_next.c");
    imtf.push_back("tif_ojpeg.c");
    imtf.push_back("tif_open.c");
    imtf.push_back("tif_packbits.c");
    imtf.push_back("tif_pixarlog.c");
    imtf.push_back("tif_predict.c");
    imtf.push_back("tif_print.c");
    imtf.push_back("tif_read.c");
    imtf.push_back("tif_strip.c");
    imtf.push_back("tif_swab.c");
    imtf.push_back("tif_thunder.c");
    imtf.push_back("tif_tile.c");
    imtf.push_back("tif_version.c");
    imtf.push_back("tif_warning.c");
    if (nix)
        imtf.push_back("tif_unix.c");
    if (win)
        imtf.push_back("tif_win32.c");
    imtf.push_back("tif_write.c");
    imtf.push_back("tif_zip.c");
    imgk.srcGroups.push_back(FgConsSrcGroup("tiff/libtiff/",imtf));
    sln.projects.push_back(imgk);
    incMain.push_back("../LibImageMagickCore/ImageMagick-6.6.2/");
    lnkMain.push_back("LibImageMagickCore");
    incMain.push_back("../LibUTF-8/");
    FgConsProj      libBase;
    libBase.name = "LibFgBase";
    libBase.srcBaseDir = "src/";
    libBase.incDirs = incMain;
    libBase.defs = defs;
    libBase.warn = 4;
    libBase.srcGroups.push_back(FgConsSrcGroup("LibFgBase/src/",""));
    if (nix) {
        libBase.srcGroups.push_back(FgConsSrcGroup("LibFgBase/src/","nix/"));
        libBase.incDirs.push_back("../LibFgBase/src/");
    }
    sln.projects.push_back(libBase);
    incMain.push_back("../LibFgBase/src/");
    if (win) {
        sln.addLib("LibFgWin",incMain,defs);
        lnkMain.push_back("LibFgWin");
    }
    // This is down here because libraries must be linked in the right order for gcc:
    lnkMain.push_back("LibFgBase");
    if (fgExists("fgbl"))
        sln.addApp("fgbl",incMain,lnkMain,defs);
    return ret;
}

void
fgCmdCons(const FgArgs &)
{
    // Allow this func to be called from repo or repo/source:
    FgPushDir       pd;
    if (fgExists("source"))
        pd.push("source");
    FgConsSolution  sln = fgConsBase(true,false).sln;
    fgConsVs201x(sln);
    sln = fgConsBase(false,true).sln;
    fgConsMakefiles(sln);
}
