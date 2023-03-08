//
// Copyright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"
#include "FgGui.hpp"
#include "FgCommand.hpp"
#include "FgTime.hpp"
#include "FgImageIo.hpp"

using namespace std;

namespace Fg {

GuiVal<ImgFormat>   guiImgFormatSelector(ImgFormats const & imgFormats,String8 const & store)
{
    ImgFormatsInfo      imgFormatInfo = getImgFormatsInfo();
    String8s            imgFormatDescs;     // descriptions
    for (ImgFormat fmt : imgFormats)
        imgFormatDescs.push_back(findFirst(imgFormatInfo,fmt).description);
    IPT<size_t>         imgFormatIdxN;      // user selection
    if (store.empty())
        imgFormatIdxN = makeIPT<size_t>(0);
    else
        imgFormatIdxN = makeSavedIPTEub<size_t>(0,store+"ImgFormat",imgFormats.size());
    auto                imgFormatFn = [=](size_t const & idx){return imgFormats[idx]; };
    OPT<ImgFormat>      imgFormatN = link1<size_t,ImgFormat>(imgFormatIdxN,imgFormatFn);
    GuiPtr              imgFormatSelW = guiRadio(imgFormatDescs,imgFormatIdxN);
    return {imgFormatN,imgFormatSelW};
}

void                testGuiDialogSplashScreen(CLArgs const &)
{
    std::function<void(void)>     f = guiDialogSplashScreen();
    fgout << fgnl << "Splash screen displayed, waiting 3 seconds ... \n";
    sleepSeconds(3);
    f();
}

void                testGui2(CLArgs const &)
{
    String8                 store = getDirUserAppDataLocalFaceGen({"base","testm gui2"});
    GuiPtr                  checkboxes;
    {
        String8s                labels {"Box 1","Box 2",};
        Svec<IPT<bool>>         states = {makeIPT(false),makeIPT(true)};
        checkboxes = guiCheckboxes(labels,states);
    }
    GuiPtr    img;
    {
        IPT<ImgRgba8>    imgN = makeIPT(loadImage(dataDir()+"base/trees.jpg"));
        IPT<Vec2Fs>      ptsN = makeIPT(svec(Vec2F(0.5,0.5)));
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
        Svec<IPT<double> >  valNs = genSvec<IPT<double> >(labs.size(),[](size_t){return makeIPT<double>(0.0); });
        sliders = guiSliderBank(valNs,labs,VecD2(-1,1),0.1);
    }
    GuiPtr        scroll;
    {
        string          text1,text2;
        for (size_t ii=0; ii<100; ++ii)
            text1 += "Tab 1 Line " + toStrDigits(ii,3) + "\n";
        for (size_t ii=0; ii<10; ++ii)
            text2 += "Tab 2 Line " + toStrDigits(ii,3) + "\n";
        GuiTabDef        tab1 = guiTab("Tab 1",guiSplitScroll(svec(guiText(text1)))),
                        tab2 = guiTab("Tab 2",guiSplitScroll(svec(guiText(text2))));
        scroll = guiTabs(svec(tab1,tab2));
    }
    GuiPtr      left = guiSplitV({checkboxes,radio}),
                tab1 = guiSplitH({left,img}),
                tab2 = guiSplitV({txt,sliders}),
                win = guiTabs(svec(guiTab("Tab1",tab1),guiTab("Tab2",tab2),guiTab("Scroll",scroll)));
    guiStartImpl(makeIPT<String8>("GUI2 testm"),win,getDirUserAppDataLocalFaceGen({"Base","GUI2 testm"}));
}

}

// */
