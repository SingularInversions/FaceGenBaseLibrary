//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// * Terminology: load/save is used for whole files, read/write is used for streams.

#ifndef FGMETAFORMAT_HPP
#define FGMETAFORMAT_HPP

#include "FgSerial.hpp"
#include "FgFile.hpp"
#include "FgFileSystem.hpp"

namespace Fg {

template<class T>
void                saveMessage(T const & val,String8 const & filename)
{
    saveRaw(toMessage(val),filename);
}
template<class T>
void                loadMessage_(String8 const & filename,T & val)
{
    try {
        fromMessage_(loadRaw(filename),val);
    }
    catch (FgException & e) {
        e.addContext("while loading file",filename.m_str);
        throw;
    }
    catch (std::exception const & e) {
        throw FgException {{
            {e.what(),""},
            {"while loading file",filename.m_str},
        }};
    }
    catch (...) {
        throw FgException {{
            {"UNKNOWN",""},
            {"while loading file",filename.m_str},
        }};
    }
}
template<class T>
T                   loadMessage(String8 const & filename)
{
    T                   ret;
    loadMessage_(filename,ret);
    return ret;
}
template<class T>
void                saveMessageExplicit(T const & val,String8 const & filename)
{
    saveRaw(toMessageExplicit(val),filename);
}
template<class T>
T                   loadMessageExplicit(String8 const & filename)
{
    try {
        return fromMessageExplicit<T>(loadRaw(filename));
    }
    catch (FgException & e) {
        e.addContext("while loading file",filename.m_str);
        throw;
    }
    catch (std::exception const & e) {
        throw FgException {{
            {e.what(),""},
            {"while loading file",filename.m_str},
        }};
    }
    catch (...) {
        throw FgException {{
            {"UNKNOWN",""},
            {"while loading file",filename.m_str},
        }};
    }
}

}

#endif
