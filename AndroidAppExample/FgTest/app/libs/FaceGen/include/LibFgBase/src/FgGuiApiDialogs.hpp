//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Sept. 20, 2011
//

#ifndef FGGUIAPIDIALOGS_HPP
#define FGGUIAPIDIALOGS_HPP

#include "FgGuiApiBase.hpp"
#include "FgGuiApiButton.hpp"

void
fgGuiDialogMessage(
    const FgString & caption,
    const FgString & message);

// NB Windows:
// * Will sometimes return UNC path (eg. Windows Server) for network drive, rather than LFS drive letter.
// * Although only extension-matching files are shown, users can (and will) type in filenames with non-matching
//   which dialog will then accept.
FgOpt<FgString>
fgGuiDialogFileLoad(
    const FgString &            description,
    const FgStrs &              extensions,
    const string &              storeID=string());  // Remember different directories for same 'description'

FgGuiPtr
fgGuiFileLoadButton(
    const FgString &            buttonText,
    const FgString &            fileTypesDescription,
    const FgStrs &              extensions,
    const string &              storeID,
    FgDgn<FgString>             output);

// The extension should be chosen in the GUI before calling this function.
// Windows will allow the user to enter a different extension of 3 characters, but extensions of different
// character length (or no extension) will have the given extension appended:
FgOpt<FgString>
fgGuiDialogFileSave(
    const FgString &        description,
    const std::string &     extension);

FgOpt<FgString>
fgGuiDialogDirSelect();

// Arguments: true - advance progress bar, false - do not
// Return: true - user cancel, false - continue
typedef std::function<bool(bool)>             FgFnBool2Bool;

typedef std::function<void(FgFnBool2Bool)>    FgFnCallback2Void;

// Returns false if the computation was cancelled by the user, true otherwise:
bool
fgGuiDialogProgress(
    const FgString &        title,
    uint                    progressSteps,
    FgFnCallback2Void       actionProgress);

// Uses the embedded icon for the splash screen.
// Call the returned function to terminate the splash screen:
std::function<void(void)>
fgGuiDialogSplashScreen();

#endif
