//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGGUIAPIDIALOGS_HPP
#define FGGUIAPIDIALOGS_HPP

#include "FgGuiApiBase.hpp"
#include "FgGuiApiButton.hpp"

namespace Fg {

void
guiDialogMessage(
    String8 const & caption,
    String8 const & message);

// NB Windows:
// * Will sometimes return UNC path (eg. Windows Server) for network drive, rather than LFS drive letter.
// * Although only extension-matching files are shown, users can (and will) type in filenames with non-matching
//   which dialog will then accept.
Opt<String8>
guiDialogFileLoad(
    String8 const &             description,        // eg. "Image" or "Comma separated values"
    Strings const &             extensions,         // list of (usually lower-case) extensions
    // Used in combination with 'description' to create a hash index for saving/loading last directory as default:
    String const &              storeID=String());

GuiPtr
guiLoadButton(
    String8 const &             buttonText,
    String8 const &             fileTypesDescription,
    Strings const &             extensions,
    String const &              storeID,
    IPT<String8> const &        selection);    // User load selection path placed here

// The extension should be chosen in the GUI before calling this function.
// Windows will allow the user to enter a different extension of 3 characters, but extensions of different
// character length (or no extension) will have the given extension appended:
Opt<String8>
guiDialogFileSave(
    String8 const &             description,
    String const &              extension);

Opt<String8>
guiDialogDirSelect();

// Arguments: true - advance progress bar, false - do not
// Return: true - user cancel, false - continue work
typedef Sfun<bool(bool)>            WorkerCallback;

// The worker function is passed the callback function for it to invoke at regular intervals
// to communicate progress and check for user cancel (see signature above):
typedef Sfun<void(WorkerCallback)>  WorkerFunc;

// Returns false if the computation was cancelled by the user, true otherwise:
bool
guiDialogProgress(
    String8 const &         dialogTitle,
    uint                    progressSteps,  // Number of progress steps exprected from worker callback
    WorkerFunc              actionProgress);

// Uses the embedded icon for the splash screen.
// Call the returned function to terminate the splash screen:
Sfun<void(void)>
guiDialogSplashScreen();

}

#endif
