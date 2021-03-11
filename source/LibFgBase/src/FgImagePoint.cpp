//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"
#include "FgImagePoint.hpp"
#include "FgFileSystem.hpp"
#include "FgScopeGuard.hpp"
#include "FgCommand.hpp"
#include "FgParse.hpp"

using namespace std;

namespace Fg {

std::ostream &
operator<<(std::ostream & os,ImagePoint const & ip)
{
    return os << ip.label << " " << ip.posIrcs[0] << " " << ip.posIrcs[1];
}

std::ostream &
operator<<(std::ostream & os,ImagePoints const & ips)
{
    os << fgnl << "Image points:" << fgpush;
    for (ImagePoint const & ip : ips)
        os << fgnl << ip;
    return os << fgpop;
}

ImagePoints
loadImagePoints(String8 const & fname)
{
    Strings         lines = splitLines(loadRawString(fname));   // No empty lines
    ImagePoints     ret;
    for (String const & line : lines) {
        if (line[0] != '#') {                                   // comment
            Strings         objs = splitChar(line,' ');         // No empty objs
            if (objs.size() != 3)
                fgout << fgnl << "WARNING invalid line: " << line;
            ImagePoint          ip;
            ip.label = objs[0];
            bool                coordsValid = true;
            for (size_t ii=0; ii<2; ++ii) {
                Opt<float>      vo = fromStr<float>(objs[ii+1]);
                if (!vo.valid()) {
                    fgout << fgnl << "WARNING invalid number for point " << ip.label << ": " << objs[ii+1];
                    coordsValid = false;
                }
                else
                    ip.posIrcs[ii] = vo.val();
            }
            if (coordsValid)
                ret.push_back(ip);
        }
    }
    return ret;
}

void
saveImagePoints(ImagePoints const & ips,String8 const & fname)
{
    Ofstream        ofs {fname};
    ofs << "# Image labelled points in raster coordinate system (origin at center of top left pixel)";
    for (ImagePoint const & ip : ips)
        ofs << "\n" << ip.label << " " << ip.posIrcs[0] << " " << ip.posIrcs[1];
}

void
merge_(ImagePoints & lhs,ImagePoints const & rhs)
{
    for (ImagePoint const & ip : rhs)
        if (!contains(lhs,ip))
            lhs.push_back(ip);
}

}
