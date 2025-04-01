//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGuiApi.hpp"
#include "FgCommand.hpp"
#include "FgTime.hpp"
#include "FgFileSystem.hpp"
#include "FgBuild.hpp"

using namespace std;

namespace Fg {

bool                isGuiSupported()
{
#ifdef _WIN32
    return true;
#else
    return false;
#endif
}

std::ostream &      operator<<(std::ostream & os,GuiCursor cursor)
{
    static Svec<pair<GuiCursor,String>> toString {
        {GuiCursor::arrow,"arrow"},
        {GuiCursor::wait,"wait"},
        {GuiCursor::translate,"translate"},
        {GuiCursor::grab,"grab"},
        {GuiCursor::scale,"scale"},
        {GuiCursor::rotate,"rotate"},
        {GuiCursor::crosshair,"crosshair"},
    };
    return os << lookupFirstL(toString,cursor);
}

GuiExceptHandler        g_guiDiagHandler;

namespace {

void                testGuiDialogSplashScreen(CLArgs const & args)
{
    if (!isAutomated(args)) {
        std::function<void(void)>     f = guiDialogSplashScreen();
        fgout << fgnl << "Splash screen displayed, waiting 3 seconds ... \n";
        sleepSeconds(3);
        f();
    }
}

void                testGuiCombo(CLArgs const & args)
{
    String8                 store = getDirUserAppDataLocalFaceGen({"base","testGuiCombo"});
    GuiPtr                  checkboxes;
    {
        String8s                labels {"Box 1","Box 2",};
        Svec<IPT<bool>>         states = {makeIPT(false),makeIPT(true)};
        checkboxes = guiCheckboxes(labels,states);
    }
    GuiPtr    img;
    {
        IPT<ImgRgba8>       imgN = makeIPT(loadImage(dataDir()+"base/Jane.jpg"));
        IPT<NameVec2Fs>     ptsN {{NameVec2F{"",Vec2F{0.5,0.5}}}};
        img = guiImageCtrls(imgN,ptsN).win;
    }
    GuiPtr    radio;
    {
        String8s           strs = {"Choice 1","Choice 2"};
        IPT<size_t>         selN(0);
        radio = guiRadio(strs,selN);
    }
    GuiPtr    txt;
    {
        GuiPtr      t1 = guiText("Hello world."),
                    t2 = guiTextEdit(makeIPT(String8("Edit me"))),
                    t3 = guiTextEditFixed(makeIPT(3.14),VecD2(-9.0,9.0)),
                    t4 = guiTextEditFloat(makeIPT(2.73),VecD2(-9.0,9.0),6);
        txt = guiSplitV({t1,t2,t3,t4});
    }
    GuiPtr    sliders;
    {
        String8s            labs = {"Slider 1","Slider 2"};
        Svec<IPT<double> >  valNs = genSvec(labs.size(),[](size_t){return makeIPT<double>(0.0); });
        sliders = guiSplitScroll(guiSliders(valNs,labs,VecD2(-1,1),0.1));
    }
    GuiPtr        scroll;
    {
        string          text1,text2;
        for (size_t ii=0; ii<100; ++ii)
            text1 += "Tab 1 Line " + toStrDigits(ii,3) + "\n";
        for (size_t ii=0; ii<10; ++ii)
            text2 += "Tab 2 Line " + toStrDigits(ii,3) + "\n";
        GuiTabDef       tab1 {"Tab 1",guiSplitScroll({guiText(text1)})},
                        tab2 {"Tab 2",guiSplitScroll({guiText(text2)})};
        scroll = guiTabs({tab1,tab2});
    }
    GuiPtr          left = guiSplitV({checkboxes,radio}),
                    win = guiTabs({
        {"Tab1",guiSplitH({left,img})},
        {"Tab2",guiSplitV({txt,sliders})},
        {"Scroll",scroll}
    });
    if (!isAutomated(args))
        guiStartImpl(makeIPT<String8>("GUI2 testm"),win,getDirUserAppDataLocalFaceGen({"Base","GUI2 testm"}));
}

void                testGuiDialogFilesLoad(CLArgs const & args)
{
    // can't call 'guiDialogFilesLoad' directly since COM needs to be initialized:
    auto                selFilesFn = []()
    {
        String8s            strs = guiDialogFilesLoad("Test selecting txt filenames",{"txt"});
        for (String8 const & str : strs)
            fgout << fgnl << str;
    };
    if (!isAutomated(args))
        guiStartImpl(
            IPT<String8>("test multi-file select"),
            guiButton("Select text files",selFilesFn),
            getDirUserAppDataLocalFaceGen({"testm","files"}));
}

void                testSliders(CLArgs const & args)
{
    size_t constexpr    S = 16;
    Svec<IPT<double>>   valNs = genSvec(S,[](size_t ii){return makeIPT(ii/8.0);});
    String8s            labels = genSvec(S,[](size_t ii){return String8{"label "}+ toStr(ii);});
    labels.back() += " (long label)";
    VecD2               range {-3,3};
    double              tickSpacing {1};
    Img<GuiPtr>         arr0 = guiSliders(cTail(valNs,8),cTail(labels,8),range,tickSpacing),    // fewer elements so we can reduce vert size
                        arr1 = guiSliders(valNs,labels,range,tickSpacing);  // duplicate required for update flag uniqueness
    GuiPtr              win0 = guiSplit(arr0),
                        win1 = guiSplitScroll(arr1),
                        win = guiSplitH({win0,win1});
    if (!isAutomated(args))
        guiStartImpl(makeIPT<String8>("GUI test sliders"),win,getDirUserAppDataLocalFaceGen({"SDK","GuiTestSliders"}));
}

}

void                testFontRender(CLArgs const &);
void                testGuiImageMark(CLArgs const &);

void                testGui(CLArgs const & args)
{
    Cmds            cmds {
        {testFontRender,"font","dispay a roman alphabet font atlas for a TTF file"},
    };
    if (getCurrentBuildOS() == BuildOS::win) {
        cat_(cmds,
            {
                {testGuiCombo,"combo"},
                {testGuiImageMark,"image"},
                {testGuiDialogFilesLoad,"files","files select dialog"},
                {testSliders,"sliders"},
                {testGuiDialogSplashScreen,"splash"},
            }
        );
    }
    doMenu(args,cmds,true);
}

}

// */
