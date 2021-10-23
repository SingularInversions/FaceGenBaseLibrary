//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGuiApiImage.hpp"
#include "FgImage.hpp"

using namespace std;


namespace Fg {

GuiPtr
guiImage(NPT<ImgRgba8> imageN)
{
    GuiImage                gi;
    gi.updateFlag = makeUpdateFlag(imageN);
    gi.updateNofill = gi.updateFlag;
    gi.wantStretch = Vec2B{false,false};
    gi.minSizeN = link1<ImgRgba8,Vec2UI>(imageN,[](ImgRgba8 const & img){return img.dims();});
    auto                    getImgFn = [imageN](Vec2UI)
    {
        return GuiImage::Disp {&imageN.cref(),Vec2I{0}};
    };
    gi.getImgFn = getImgFn;
    return guiMakePtr(gi);
}

GuiPtr
guiImage(NPT<ImgRgba8> const & imageN,Sfun<void(Vec2F)> const & onClick)
{
    GuiImage                gi;
    gi.updateFlag = makeUpdateFlag(imageN);
    gi.updateNofill = gi.updateFlag;
    gi.wantStretch = Vec2B{false,false};
    gi.minSizeN = link1<ImgRgba8,Vec2UI>(imageN,[](ImgRgba8 const & img){return img.dims();});
    auto                    clickFn = [imageN,onClick](Vec2I pos)
    {
        Vec2D           iucs = cIrcsToIucsXf(imageN.cref().dims()) * Vec2D(pos);
        onClick(Vec2F(iucs));
    };
    gi.clickLeft = clickFn;
    auto                    getImgFn = [imageN](Vec2UI)
    {
        return GuiImage::Disp {&imageN.cref(),Vec2I{0}};
    };
    gi.getImgFn = getImgFn;
    return guiMakePtr(gi);
}

GuiImg
guiImageCtrls(
    NPT<ImgRgba8> const &       imageN,
    IPT<Vec2Fs> const &         ptsIucsN,
    bool                        expertMode,
    Sfun<void(Vec2F)> const &   onClick)
{
    OPT<ImgRgba8s>          mipmapN = link1<ImgRgba8,ImgRgba8s>(imageN,cMipmap<Rgba8>);
    // offset of image centre from view area centre in original image pixels:
    IPT<Vec2F>              offsetN {Vec2F{0,0}};
    // zoom level is log base 2 of the linear scale:
    IPT<int>                zoomN {0};
    // scale of display image relative to original:
    OPT<float>              scaleN = link1<int,float>(zoomN,exp2f);
    // need a copy of the output image on the head for the return value pointer:
    IPT<ImgRgba8>           imgOutputN;
    auto                    imgDispFn = [=](Vec2UI winSize)
    {
        // Strategy is to create an image exactly the size of the display window and fill
        // background with a color or pattern:
        ImgRgba8s const &   mipmap = mipmapN.cref();
        Vec2F               origSize (mipmap[0].dims());
        int                 zoom = zoomN.val();
        uint                mipLevel = uint(cMax(0,-zoom));
        ImgRgba8 const &    imgOrig = mipmap[mipLevel];
        // pixel scale of output image relative to input:
        float               scale = exp2f(zoom+mipLevel);
        float               invScale = 1.0 / scale;
        // TODO: clamp offset bounds:
        Mat22I              origBounds = catHoriz(Vec2I{0},Vec2I(imgOrig.dims()));
        AffineEw2F          originToWinCentre {Vec2F(1),-Vec2F(winSize)/2},
                            scaleToOrig {Vec2F(invScale),Vec2F(0)},
                            applyOffset {Vec2F(1),origSize/2-offsetN.val()},
                            winToOrig = applyOffset * scaleToOrig * originToWinCentre;
        ImgRgba8 &          imgOutput = imgOutputN.ref();
        Rgba8               background {128,128,128,255};
        imgOutput.resize(winSize);
        if (expertMode || (zoomN.val() < 1)) {                      // sample by rounding
            auto            fn = [=](Vec2UI ircs)
            {
                Vec2F           crdf = winToOrig * Vec2F(ircs);
                Vec2I           crd =  Vec2I(mapFloor(crdf));
                if (isInBounds(origBounds,crd))
                    return imgOrig[Vec2UI(crd)];
                else
                    return background;
            };
            mapCall_(imgOutput,fn,true);
        }
        else {                                                      // sample by lerping
            auto            fn = [=](Vec2UI ircs)
            {
                Vec2F           crdf = winToOrig * Vec2F(ircs);
                // TODO: why is sampleAlpha in IUCS ?
                RgbaF       clr = sampleAlpha(imgOrig,mapDiv(crdf,origSize));
                if (clr.alpha() > 127.0f) {
                    clr *= (clr.alpha() / 255.0f);
                    return Rgba8(clr);
                }
                else
                    return background;
            };
            mapCall_(imgOutput,fn,true);
        }
        return GuiImage::Disp{&imgOutput,{0,0}};
    };
    auto                    zoomInFn = [=]()
    {
        int &               zoom = zoomN.ref();
        int                 zoomMax = expertMode ? 4 : 2;
        zoom = cMin(zoom+1,zoomMax);
    };
    auto                    zoomOutFn = [=]()
    {
        int &               zoom = zoomN.ref();
        float               relSize = cMaxElem(imageN.cref().dims()) / 512.0f;  // typical window size
        int                 zoomMin = int(floor(-4.0f - log2f(relSize)));
        zoom = cMax(zoom-1,zoomMin);
    };
    IPT<int>                zoomAccN {0};
    auto                    dragRightFn = [=](Vec2I delta)
    {
        int                 zoomAcc = zoomAccN.val() + delta[1];
        if (abs(zoomAcc) > 40) {
            if (zoomAcc > 0)
                zoomInFn();
            else
                zoomOutFn();
            zoomAcc = 0;
        }
        zoomAccN.set(zoomAcc);
    };
    auto                    clickLeftFn = [=](Vec2I pos)
    {
        ImgRgba8 const &    img = imageN.cref();
        float               scale = scaleN.val();
        Vec2F               imgPosf = Vec2F(pos) / scale - offsetN.val();
        Vec2I               imgPosi = Vec2I(mapFloor(imgPosf));
        if (isInBounds(Mat22I(0,img.width(),0,img.height()),imgPosi)) {
            Vec2D           iucs = cIrcsToIucsXf(img.dims()) * Vec2D(imgPosi);
            onClick(Vec2F(iucs));
        }
    };
    GuiImage                gi;
    gi.getImgFn = imgDispFn;
    gi.wantStretch = Vec2B{true,true};
    gi.minSizeN = makeIPT(Vec2UI{100});
    gi.updateFlag = makeUpdateFlag(imageN,zoomN);
    // offset never needs background repaint since only images that more than cover the view area
    // can be translated. Image smaller than the view area are fixed in the centre:
    gi.updateNofill = makeUpdateFlag(offsetN,ptsIucsN);
    gi.dragLeft = [=](Vec2I delta) {offsetN.ref() += Vec2F(delta) / scaleN.val(); };
    gi.dragRight = dragRightFn;
    if (onClick)
        gi.clickLeft = clickLeftFn;
    return {guiMakePtr(gi),zoomInFn,zoomOutFn};
}

}

// */
