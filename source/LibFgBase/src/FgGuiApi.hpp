//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// API DESIGN:
//
// Functions creating windows with associated dataflow input node prefer to accept those
// nodes as an argument, rather than create them, for 2 reasons:
// * IPTs can be created in a number of ways (saved? default?) and we don't want to add
//   that complexity to each window creation funciton.
// * IPTs created in the function must be returned along with the window pointer, making
//   for inconvenient return-by-structures.
// * IPTs are easy to create by the client inline in the function arguments and can be
//   passed forward for assignment because the dataflow node is allocated by default construction.

#ifndef FGGUIAPIBASE_HPP
#define FGGUIAPIBASE_HPP

#include "FgDataflow.hpp"
#include "FgImageIo.hpp"

namespace Fg {

// Returns true if GUI is supported on this platform (GUI calls do nothing otherwise):
bool                isGuiSupported();

// Set up this data structure for application error handling (eg. report to server):
struct  GuiExceptHandler
{
    // set to full application name with version and bits if applicable:
    String8                 appNameVerBits;
    // Client-defined error reporting. Can be null.
    // Accepts error message, returns true if reported, false otherwise (so default dialog can be shown):
    Sfun<bool(String8)>     reportError;
};

extern GuiExceptHandler     g_guiDiagHandler;

struct  GuiBaseImpl;

typedef Sptr<GuiBaseImpl>   GuiImplPtr;
typedef Svec<GuiImplPtr>    GuiImplPtrs;

struct      GuiBase
{
    virtual ~GuiBase() {};         // Don't leak

    // Originally used the CTRP to avoid pasting a typed implementation of this in each subclass,
    // but this didn't work in a namespace since the forward declaration had to be within the
    // templated function and MSVC has a compiler bug that puts said declaration in the global
    // namespace. Plus CRTP is ugly. Then discovered that a global template function doesn't work
    // either because the specialization must be defined before use. So now each class declares
    // its specific global get instance function and instantiates this member using that:
    virtual Sptr<GuiBaseImpl>   getInstance() = 0;
};

typedef Sptr<GuiBase>       GuiPtr;
typedef Svec<GuiPtr>        GuiPtrs;
typedef Svec<GuiPtrs>       GuiPtrss;

template<class T>
Sptr<GuiBase>       guiMakePtr(T const & stackVal) {return std::make_shared<T>(stackVal); }

struct      GuiEvent
{
    void *              handle;     // OS-specific handle to event for triggering main event-driven loop
    Sfun<void()>        handler;    // Function to handle event
};

struct      GuiKeyHandle
{
    char                key;        // Only visible keys handled for now
    Sfun<void()>        handler;
};

struct      GuiOptions
{
    Svec<GuiEvent>      events;
    Svec<GuiKeyHandle>  keyHandlers;
    Sfun<void()>        onUpdate;       // Used to store to undo stack
    Vec2I               defaultSize = {1400,900};
};

template<class T>
struct      GuiVal           // Combine a window and a related node
{
    NPT<T>              valN;
    GuiPtr              win;

    GuiVal() {}
    GuiVal(const NPT<T> & v,const GuiPtr & w) : valN(v), win(w) {}
};

// Defined in OS-specific code:
void                guiStartImpl(
    NPT<String8>                titleN,
    GuiPtr                      gui,
    String8 const &             storeDir,           // directory [and prefix] in which to store state files
    GuiOptions const &          options=GuiOptions{});

// Send message to terminate GUI. Defined in OS-specific code. TODO: make it not global:
void                guiQuit();

enum struct     GuiCursor {arrow, wait, translate, grab, scale, rotate, crosshair };
std::ostream &  operator<<(std::ostream &,GuiCursor);

enum struct     GuiClick {left, middle, right };

struct          GuiClickState
{
    bool            leftDown;
    bool            middleDown;
    bool            rightDown;
    bool            shiftDown;
    bool            ctrlDown;

    GuiClickState() : leftDown{false}, middleDown{false}, rightDown{false}, shiftDown{false}, ctrlDown{false} {}
    GuiClickState(bool l,bool m,bool r,bool s,bool c) : leftDown{l}, middleDown{m}, rightDown{r}, shiftDown{s}, ctrlDown{c} {}
    FG_EQ_M5(GuiClickState,leftDown,middleDown,rightDown,shiftDown,ctrlDown)

    size_t          toDragIndex() const {
        return
            (leftDown ? 1 : 0) +
            (middleDown ? 2 : 0) +
            (rightDown ? 4 : 0) +
            (shiftDown ? 8 : 0) +
            (ctrlDown ? 16 : 0);
    }
    bool            allButtonsUp() const {return !leftDown && !middleDown && !rightDown; }
    bool            onlyOneButton() const
    {
        return isPow2((leftDown ? 1 : 0) + (middleDown ? 2 : 0) + (rightDown ? 4 : 0));
    }
    size_t          getKeyIndex() const {return (shiftDown ? 1 : 0) + (ctrlDown ? 2 : 0); }
    size_t          getButtonIndex() const
    {
        FGASSERT(onlyOneButton());
        return leftDown ? 0 : middleDown ? 1 : 2;
    }
    size_t          getActionIndex() const {return getKeyIndex()*3 + getButtonIndex(); }
};

// Set the cursor to show the application is busy.
// Resets automatically when application becomes responsive.
void                guiBusyCursor();

struct      GuiButton;
// This function must be defined in the corresponding OS-specific implementation:
GuiImplPtr          guiGetOsImpl(GuiButton const & guiApi);

struct      GuiButton : GuiBase
{
    String8                 label;
    Sfun<void()>            action;
    bool                    stretchX;

    GuiButton(String8 const & l,Sfun<void()> const & a,bool s) : label{l}, action{a}, stretchX{s} {}

    virtual GuiImplPtr      getInstance() {return guiGetOsImpl(*this); }
};

inline GuiPtr       guiButton(String8 const & label,Sfun<void()> const & action,bool stretchX=false)
{
    return std::make_shared<GuiButton>(label,action,stretchX);
}

// This function must be defined in the corresponding OS-specific implementation:
struct      GuiCheckbox;
GuiImplPtr          guiGetOsImpl(GuiCheckbox const & guiApi);

struct      GuiCheckbox : GuiBase
{
    String8             label;
    // Will be called to get current status for display updates (must return true if box selected):
    Sfun<bool()>        getFn;
    // Will be called when user clicks on checkbox:
    Sfun<void()>        clickFn;

    GuiCheckbox() {}
    GuiCheckbox(String8 const & l,Sfun<bool()> const & g,Sfun<void()> const & c)
        : label(l), getFn(g), clickFn(c)
    {}

    virtual GuiImplPtr getInstance() {return guiGetOsImpl(*this); }
};

GuiPtr              guiCheckbox(String8 const & label,IPT<bool> const & valInp);
GuiPtr              guiCheckboxes(String8s const & labels,Svec<IPT<bool>> const & selNs);

void                guiDialogMessage(String8 const & caption,String8 const & message);

// NB Windows:
// * Will sometimes return UNC path (eg. Windows Server) for network drive, rather than LFS drive letter.
// * Although only extension-matching files are shown, users can (and will) type in filenames with non-matching
//   which dialog will then accept.
Opt<String8>        guiDialogFileLoad(
    String8 const &             description,        // eg. "Image" or "Comma separated values"
    Strings const &             extensions,         // list of (usually lower-case) extensions
    // Used in combination with 'description' to create a hash index for saving/loading last directory as default:
    String const &              storeID=String());

GuiPtr              guiLoadButton(
    String8 const &             buttonText,
    String8 const &             fileTypesDescription,
    Strings const &             extensions,
    String const &              storeID,
    IPT<String8> const &        selection);    // User load selection path placed here

// The extension should be chosen in the GUI before calling this function.
// Windows will allow the user to enter a different extension of 3 characters, but extensions of different
// character length (or no extension) will have the given extension appended:
Opt<String8>        guiDialogFileSave(
    String8 const &             description,
    String const &              extension);

Opt<String8>        guiDialogDirSelect();

// Arguments: true - advance progress bar, false - do not
// Return: true - user cancel, false - continue work
typedef Sfun<bool(bool)>            WorkerCallback;

// The worker function is passed the callback function for it to invoke at regular intervals
// to communicate progress and check for user cancel (see signature above):
typedef Sfun<void(WorkerCallback)>  WorkerFunc;

// Returns false if the computation was cancelled by the user, true otherwise:
bool                guiDialogProgress(
    String8 const &         dialogTitle,
    uint                    progressSteps,  // Number of progress steps exprected from worker callback
    WorkerFunc const &      actionProgress);

// Uses the embedded icon for the splash screen.
// Call the returned function to terminate the splash screen:
Sfun<void(void)>    guiDialogSplashScreen();

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiDynamic;
GuiImplPtr guiGetOsImpl(GuiDynamic const & guiApi);

struct      GuiDynamic : GuiBase
{
    Sfun<GuiPtr(void)>  makePane;
    DfgFPtr             updateFlag;

    virtual GuiImplPtr  getInstance() {return guiGetOsImpl(*this); }
};

inline GuiPtr       guiDynamic(Sfun<GuiPtr(void)> const & makePane,DfgFPtr const & updateFlag)
{
    GuiDynamic     d;
    d.makePane = makePane;
    d.updateFlag = updateFlag;
    return std::make_shared<GuiDynamic>(d);
}

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiGroupbox;
GuiImplPtr guiGetOsImpl(GuiGroupbox const & guiApi);

struct      GuiGroupbox : GuiBase
{
    String8             label;
    GuiPtr              contents;
    GuiGroupbox(String8 const & l,GuiPtr c) : label(l), contents(c) {}

    virtual GuiImplPtr  getInstance() {return guiGetOsImpl(*this); }
};

inline GuiPtr           guiGroupboxTr(const std::string & label,GuiPtr p) {return std::make_shared<GuiGroupbox>(fgTr(label),p); }
inline GuiPtr           guiGroupbox(String8 const & label,GuiPtr p) {return std::make_shared<GuiGroupbox>(label,p); }

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiImage;
GuiImplPtr          guiGetOsImpl(GuiImage const & guiApi);

struct      GuiImage : GuiBase
{
    struct      Disp
    {
        ImgRgba8 const *    imgPtr;
        Vec2I               offset;         // from top left of window
    };
    // Callback when image is needed for bitblt to screen.
    // Output node with image data is not sufficient since user controls input state (eg. zoom & offset)
    // need be modified when the window size changes:
    Sfun<Disp(Vec2UI)>  getImgFn;           // Input argument is display win dims in pixels
    Vec2B               wantStretch;
    NPT<Vec2UI>         minSizeN;           // Minimum display window size (in pixels)
    DfgFPtr             updateFlag;         // Display update flag when background should be filled
    DfgFPtr             updateNofill;       // Display update flag when only image pixels need to be updated

    // USER ACTION CALLBACKS:
    // don't perform an action on click down but may want to change cursor to show what drag control is enabled
    // (possibly position-dependent). Array index bits correspond (lo to hi) to L,M,R mouse buttons and shift & control
    // keys being down at time of click. Index zero is not used:
    Arr<Sfun<GuiCursor(Vec2I)>,32>  clickDownFns;       // vec arg is client window position in pixels
    // click actions take place on click up if the last click event was click down of the same button,
    // no other buttons have been clicked, and no mouse movement has occured in the interim.
    // col index is by button (0,1,2)=(L,M,R), row index by modifier: lo bit is shift on/off, hi bit is ctrl on/off:
    Arr<Sfun<void(Vec2I)>,12>      clickActionFns;      // vec arg is client window position in pixels
    // array index bits correspond (lo to hi) to L,M,R mouse buttons and shift & control keys being down
    // during mouse move. always return the no-click cursor for that position so the GUI knows what to display
    // when drag is released. If defined, the no-modifier drag should not make any changes other than returning a cursor:
    Arr<Sfun<GuiCursor(Vec2I,Vec2I)>,32> mouseMoveFns;  // vec args are mouse pos, delta in pixels

    virtual GuiImplPtr  getInstance() {return guiGetOsImpl(*this); }

    GuiImage() {}
    explicit GuiImage(NPT<ImgRgba8> imageN);        // fixed size image GUI
};

// Fixed size image with no controls:
inline GuiPtr       guiImage(NPT<ImgRgba8> imageN) {return std::make_shared<GuiImage>(imageN); }

// Fixed size image that can be clicked on for custom action:
GuiPtr              guiImage(NPT<ImgRgba8> imageN,Sfun<void(Vec2F)>);   // Arg is IUCS image coordinate of click

struct      GuiImg
{
    GuiPtr              win;
    Sfun<void(void)>    zoomIn;
    Sfun<void(void)>    zoomOut;
};
// Zoom-able (by powers of 2), shift-able, click-able image of variable size:
GuiImg              guiImageCtrls(
    NPT<ImgRgba8> const &       imageN,             // source image for display
    // User-selected points in IUCS will be overlaid on image.
    // NB These are NOT corrected for non-power-of-2 pixel truncation:
    IPT<Vec2Fs> const &         ptsIucsN,
    // if true, image zoom goes up to 8x for <=2K images.
    // if false, image zoom goes up to 4x for <=2K images.
    bool                        expertMode=false,
    // If defined, this is called when the user ctrl-clicks on the image.
    // The first argument is the coordinate in IUCS. The second argument are the image dimensions at the
    // current viewing zoom level. The latter can be useful for determining accidental near double-clicks.
    // Note that unless the image is a power of 2 in size, small discrepencies exist between the coord
    // in different SLs, but they are sub-pixel:
    Sfun<void(Vec2F,Vec2UI)> const & onCtrlClick=nullptr);

// radio selection dialog of given formats by their default descriptions:
GuiVal<ImgFormat>   guiImgFormatSelector(
    ImgFormats const &  imgFormats,     // ordered list of formats for radio selection
    String8 const &     store={});      // leave empty to avoid storing user setting

// This function must be defined in the corresponding OS-specific implementation:
struct      GuiRadio;
GuiImplPtr          guiGetOsImpl(GuiRadio const & guiApi);

struct      GuiRadio : GuiBase
{
    String8s            labels;
    Sfun<uint()>        getFn;      // Will be called for display updates. Must return current selection.
    Sfun<void(uint)>    setFn;      // Will be called when user makes a selection.

    virtual GuiImplPtr  getInstance() {return guiGetOsImpl(*this); }
};

// If desired output is the index:
GuiPtr              guiRadio(String8s const & labels,IPT<size_t> idxN);
// If desired output is the label:
GuiVal<String8>     guiRadioLabel(String8s const & labels,IPT<size_t> idxN);

// This function must be defined in the corresponding OS-specific implementation:
struct      GuiSelect;
GuiImplPtr          guiGetOsImpl(GuiSelect const & guiApi);

struct      GuiSelect : GuiBase
{
    GuiPtrs             wins;
    NPTF<size_t>        selection;

    GuiSelect(GuiPtrs const & w,NPT<size_t> const & s) : wins(w), selection(s) {}

    virtual GuiImplPtr  getInstance() {return guiGetOsImpl(*this); }
};

inline GuiPtr           guiSelect(NPT<size_t> select,GuiPtrs const & wins)
{
    return std::make_shared<GuiSelect>(wins,select);
}

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiSpacer;
GuiImplPtr guiGetOsImpl(GuiSpacer const & guiApi);

struct      GuiSpacer : GuiBase
{
    Vec2UI              size;       // One dim can be zero

    virtual GuiImplPtr  getInstance() {return guiGetOsImpl(*this); }
};

inline GuiPtr       guiSpacer(uint wid,uint hgt)
{
    GuiSpacer  ret;
    ret.size = Vec2UI(wid,hgt);
    return std::make_shared<GuiSpacer>(ret);
}

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiSplit;
GuiImplPtr          guiGetOsImpl(GuiSplit const & guiApi);

// Algorithmically proportioned split window with all contents viewable:
struct      GuiSplit : GuiBase
{
    Img<GuiPtr>         panes;

    virtual GuiImplPtr  getInstance() {return guiGetOsImpl(*this); }
};

// Checks for case of 1x1 image and just returns the single GuiPtr in that case:
GuiPtr              guiSplit(Img<GuiPtr> const & panes);
// Horizontal array of panes:
inline GuiPtr       guiSplitH(GuiPtrs const & panes) {return guiSplit(Img<GuiPtr>{panes.size(),1,panes}); }
// Vertical array of panes:
inline GuiPtr       guiSplitV(GuiPtrs const & panes) {return guiSplit(Img<GuiPtr>{1,panes.size(),panes}); }

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiSplitAdj;
GuiImplPtr          guiGetOsImpl(GuiSplitAdj const & guiApi);

// Adjustable split dual window with central divider:
struct  GuiSplitAdj : GuiBase
{
    bool                horiz;
    GuiPtr              pane0;
    GuiPtr              pane1;

    GuiSplitAdj(bool h,GuiPtr p0,GuiPtr p1) : horiz(h), pane0(p0), pane1(p1) {}

    virtual GuiImplPtr  getInstance() {return guiGetOsImpl(*this); }
};

inline GuiPtr       guiSplitAdj(bool horiz,GuiPtr p0,GuiPtr p1) {return std::make_shared<GuiSplitAdj>(horiz,p0,p1); }

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiSplitScroll;
GuiImplPtr          guiGetOsImpl(GuiSplitScroll const & guiApi);

// Vertically scrollable split window (panes thickness is fixed to minimum):
struct      GuiSplitScroll : GuiBase
{
    DfgFPtr                 updateFlag;     // Has the panes info been updated ?
    // This function must not depend on the same guigraph node depended on by its children or
    // the windows will be destroyed and recreated with each child update and thus not work:
    Sfun<GuiPtrs(void)>     getPanes;
    Vec2UI                  minSize;        // Of client area (not including scroll bar)
    uint                    spacing;        // Insert this spacing above each sub-win

    GuiSplitScroll() : minSize(300,300), spacing(0) {}

    virtual GuiImplPtr      getInstance() {return guiGetOsImpl(*this); }
};

// all of the below are vertical scrolls:
GuiPtr              guiSplitScroll(GuiPtrs const & panes,uint spacing=0);
GuiPtr              guiSplitScroll(Sfun<GuiPtrs(void)> const & getPanes);
GuiPtr              guiSplitScroll(Img<GuiPtr> const & panes);

GuiPtr              guiSplitScroll(
    DfgFPtr const &                 updateNodeIdx,  // Must be unique to this object
    Sfun<GuiPtrs(void)> const &     getPanes,
    uint                            spacing=0);

struct      GuiTickLabel
{
    double          pos;
    String8         label;

    GuiTickLabel() {}
    GuiTickLabel(double p,String8 l) : pos(p), label(l) {}
};

typedef Svec<GuiTickLabel> GuiTickLabels;

// This function must be defined in the corresponding OS-specific implementation:
struct      GuiSlider;
GuiImplPtr          guiGetOsImpl(GuiSlider const & guiApi);

struct      GuiSlider : GuiBase
{
    DfgFPtr             updateFlag;
    // getInput is required 1. to allow for restoring from serialization and 2. to allow
    // for dependent sliders (eg. FaceGen linear controls). It is also then used for the
    // initialization value:
    Sfun<double(void)>  getInput;
    Sfun<void(double)>  setOutput;
    String8             label;          // Can be empty
    VecD2               range;
    double              tickSpacing;
    GuiTickLabels       tickLabels;     // Can be empty
    GuiTickLabels       tockLabels;     // ". On other side from ticks
    // Set this to larger values if your tick / tock labels overflow the edges:
    uint                edgePadding = 5;

    virtual GuiImplPtr  getInstance() {return guiGetOsImpl(*this); }
};
typedef Svec<GuiSlider> GuiSliders;

GuiPtr              guiSlider(
    IPT<double>         valN,
    String8             label,               // Can be empty
    VecD2               range,
    double              tickSpacing,
    GuiTickLabels const & tl = GuiTickLabels(),
    GuiTickLabels const & ul = GuiTickLabels(),
    uint                edgePadding=5,
    bool                editBox=false);     // Numerical edit box on right, clipped to slider range, 2 fractional digits.

// vertical array of windows with labels on left, sliders on right:
Img<GuiPtr>         guiSliders(
    Svec<IPT<double> > const & valNs,
    String8s const &        labels,     // Must be same size as valNs
    VecD2                   range,
    double                  tickSpacing,
    GuiTickLabels const &   tickLabels=GuiTickLabels{});

// Array of panes with labels on left, sliders on right:
inline GuiPtr       guiSliderBank(
    Svec<IPT<double> > const & valNs,
    String8s const &        labels,     // Must be same size as valNs
    VecD2                   range,
    double                  tickSpacing,
    GuiTickLabels const &   tickLabels=GuiTickLabels{})
{return guiSplit(guiSliders(valNs,labels,range,tickSpacing,tickLabels)); }

// Use to auto create labels for above. Labels will have numbers appended:
String8s            numberedLabels(String8 const & baseLabel,size_t num);

// Generate equispaced tick labels:
GuiTickLabels       guiTickLabels(VecD2 range,double spacing,double basePos);

struct  GuiTabDef
{
    String8         label;
    GuiPtr          win;
    uint            padLeft;        // pixels ...
    uint            padRight;
    uint            padTop;
    uint            padBottom;

    GuiTabDef() : padLeft(1), padRight(1), padTop(1), padBottom(1) {}
    GuiTabDef(String8 const & l,GuiPtr w) : label(l), win(w), padLeft(1), padRight(1), padTop(1), padBottom(1) {}
    GuiTabDef(String8 const & l,bool spacer,GuiPtr w)
    :   label(l), win(w),
        padLeft(spacer ? 5 : 1), padRight(spacer ? 5 : 1),
        padTop(spacer ? 10 : 1), padBottom(1)
    {}
};
typedef Svec<GuiTabDef>  GuiTabDefs;

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiTabs;
GuiImplPtr guiGetOsImpl(GuiTabs const & guiApi);

struct      GuiTabs : GuiBase
{
    GuiTabDefs          tabs;
    GuiTabs(GuiTabDefs const & t) : tabs{t} {}

    virtual GuiImplPtr  getInstance() {return guiGetOsImpl(*this); }
};

inline GuiPtr       guiTabs(GuiTabDefs const & tabs)
{
    FGASSERT(!tabs.empty());
    return std::make_shared<GuiTabs>(tabs);
}

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiText;
GuiImplPtr guiGetOsImpl(GuiText const & guiApi);

struct  GuiText : GuiBase
{
    NPT<String8>        content;
    // Usually set to true for dynamic text:
    Vec2B               wantStretch = Vec2B(false);
    // Used to specify a fixed min width for 2D layouts (eg. label - slider lists). Zero ignores.
    uint                minWidth = 0;
    // Given in lines. When you expect overflow from one line, reserve more:
    uint                minHeight = 1;
    // Set this to false to avoid bug in Win10 RichEdit that causes copy operations from this richedit
    // to hang on paste (in any other context) until this main window regains focus. Note that
    // newlines and hyptertext links are not supported with RichEdit:
    bool                rich = true;

    virtual GuiImplPtr  getInstance() {return guiGetOsImpl(*this); }
};

// Assumes dynamic text and sets 'wantStretch' to true:
GuiPtr              guiText(NPT<String8> node,uint minWidth=0,bool rich=true);
GuiPtr              guiTextLines(NPT<String8> node,uint minHeight,bool wantStretchVert=false);
GuiPtr              guiText(String8 text,uint minWidth=0,bool rich=true);

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiTextEdit;
GuiImplPtr guiGetOsImpl(GuiTextEdit const & guiApi);

struct  GuiTextEdit : GuiBase
{
    DfgFPtr                 updateFlag;
    Sfun<String8(void)>     getInput;
    Sfun<void(String8)>     setOutput;
    uint                    minWidth;
    bool                    wantStretch;    // Width only.

    virtual GuiImplPtr      getInstance() {return guiGetOsImpl(*this); }
};

// String text edit box:
GuiPtr              guiTextEdit(IPT<String8> t,bool wantStretch=true);
// Fixed-point numerical text edit box with given number of fractional digits and clamp values:
GuiPtr              guiTextEditFixed(
    DfgFPtr                 updateFlag,     // Must be unique to each function call
    Sfun<double(void)>      getVal,
    Sfun<void(double)>      setVal,
    VecD2                   bounds,
    uint                    numFraction);
// Fixed-point numerical text edit box with specified fractional digits, clips output values to bounds:
GuiPtr              guiTextEditFixed(IPT<double> valN,VecD2 bounds,uint numFraction=2);
// Flaoting-point numerical text edit box with given number of digits and clamp values:
GuiPtr              guiTextEditFloat(
    DfgFPtr                 updateFlag,     // Must be unique to each function call
    Sfun<double()>          getVal,
    Sfun<void(double)>      setVal,
    VecD2                   bounds,
    uint                    numDigits);
// Fixed-point numerical text edit box with specified fractional digits, clips output values to bounds:
GuiPtr              guiTextEditFloat(IPT<double> valN,VecD2 bounds,uint numDigits);

}

#endif
