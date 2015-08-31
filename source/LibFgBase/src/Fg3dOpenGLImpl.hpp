//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Sohail Somani, Andrew Beatty
//
// DO NOT INCLUDE THIS FILE EXCEPT IN PLATFORM-SPECIFIC Fg3dOpenGLImp.cpp
//
// This code is cross-platform but is compiled in a platform-specific library
// because the Windows opengl headers are tied into windows.h.
//
// The alternative approach of using our own gl.h header files was avoided
// in case some platform versions require different calling conventions or other
// variations in the declarations.
//

#include "FgOpenGL.hpp"
#include "Fg3dOpenGL.hpp"
#include "Fg3dNormals.hpp"
#include "FgLighting.hpp"
#include "FgImageBase.hpp"
#include "FgValidVal.hpp"
#include "FgDiagnostics.hpp"
#include "FgAffine1.hpp"
#include "FgAffineCwC.hpp"

using namespace std;

static
void
drawWires(const vector<FgOglRendModel> &  rms)
{
    glDisable(GL_TEXTURE_2D);   // Disable texture mapping.
    glDisable(GL_LIGHTING);     // Draw mesh lines in constant colour.
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    glLineWidth(1.0f);

    for (size_t mm=0; mm<rms.size(); ++mm)
    {
        const Fg3dMesh &    mesh = *rms[mm].mesh;
        const FgVerts &     verts = *rms[mm].verts;
        for (size_t ss=0; ss<mesh.surfaces.size(); ss++)
        {
            if (rms[mm].rendSurfs[ss].visible)
            {
                const Fg3dSurface & surf = mesh.surfaces[ss];
                glColor3f(0.0f,0.0f,0.4f);
                glBegin(GL_TRIANGLES);
                for (uint ii=0; ii<surf.numTris(); ii++)
                {
                    FgVect3I   tri(surf.getTri(ii));
                    glVertex3fv(&verts[tri[0]][0]);
                    glVertex3fv(&verts[tri[1]][0]);
                    glVertex3fv(&verts[tri[2]][0]);
                }
                glEnd();
                glBegin(GL_QUADS);
                for (uint ii=0; ii<surf.numQuads(); ii++)
                {
                    FgVect4I   quad(surf.getQuad(ii));
                    glVertex3fv(&verts[quad[0]][0]);
                    glVertex3fv(&verts[quad[1]][0]);
                    glVertex3fv(&verts[quad[2]][0]);
                    glVertex3fv(&verts[quad[3]][0]);
                }
                glEnd();
            }
        }
    }
}

static
void
drawVerts(const vector<FgOglRendModel> &  rms)
{
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glPointSize(2.0);
    for (size_t mm=0; mm<rms.size(); ++mm) {
        const FgVerts & verts = *rms[mm].verts;
        FgVect3F    col;
        col[mm%3] = 1.0f;
        glColor3f(col[0],col[1],col[2]);
        glBegin(GL_POINTS);
        for (size_t ii=0; ii<verts.size(); ii++)
            glVertex3fv(verts[ii].dataPtr());
        glEnd();
    }
}

static
void
drawMarkedVerts(const vector<FgOglRendModel> &  rms)
{
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glColor3f(0.0f,1.0f,1.0f);
    glPointSize(5.0);
    FgVect3F       point;
    glBegin(GL_POINTS);
    for (size_t mm=0; mm<rms.size(); ++mm) {
        const FgVerts &         verts = *rms[mm].verts;
        const vector<FgMarkedVert> &    markedVerts = rms[mm].mesh->markedVerts;
        for (size_t ii=0; ii<markedVerts.size(); ii++) {
            point = verts[markedVerts[ii].idx];
            glVertex3fv(&point[0]);
        }
    }
    glEnd();
}

static
void
drawPoints(const vector<FgOglRendModel> &  rms)
{
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glColor3f(1.0f,0.0f,1.0f);
    glPointSize(5.0);
    FgVect3F       point;
    glBegin(GL_POINTS);
    for (size_t mm=0; mm<rms.size(); ++mm) {
        const Fg3dMesh &    mesh = *rms[mm].mesh;
        const FgVerts &     verts = *rms[mm].verts;
        for (uint ii=0; ii<mesh.numSurfPoints(); ii++) {
            point = mesh.getSurfPoint(verts,ii);
            glVertex3fv(&point[0]);
        }
    }
    glEnd();
}

static uint                     s_specMapName;

struct  Tri
{
    // Assume CC winding for verts & uvs:
    FgVect3F        v[3];   // verts
    FgVect3F        n[3];   // norms
    FgVect2F        u[3];   // uvs

    FgVect4F
    meanVertH() const
    {return fgAsHomogVec((v[0]+v[1]+v[2]) * 0.33333333f); }
};

static
void
drawTris(const vector<vector<Tri> > & tris)
{
    glBegin(GL_TRIANGLES);
    for (size_t ii=0; ii<tris.size(); ++ii) {
        const vector<Tri> & ts = tris[ii];
        for (size_t jj=0; jj<ts.size(); ++jj) {
            const Tri & t = ts[jj];
            // Texture coordinates are just ignored if gl texturing is off:
            for (uint kk=0; kk<3; ++kk) {
                glTexCoord2fv(&t.u[kk][0]);
                glNormal3fv  (&t.n[kk][0]);
                glVertex3fv  (&t.v[kk][0]); }
        }
    }
    glEnd();
}

static
void
insertTri(
    vector<vector<Tri> > &  tris,
    const Tri &             t,
    const FgMat44F &     prj,
    const FgAffine1F &      depToBin)
{
    FgVect4F    mean = prj * t.meanVertH();
    float       dep = mean[2] / mean[3];
    size_t      bin = size_t(depToBin * dep);
    if (bin < tris.size())
        tris[bin].push_back(t);
}

static
void
drawSurfaces(
    const Fg3dMesh &            mesh,
    const FgVerts &             verts,
    const Fg3dNormals &         norms,
    const vector<int> &         texNames,
    const Fg3dRenderOptions &   rend,
    bool                        xray,
    const vector<FgOglRendSurf> &   surfs)
{
    FGASSERT(mesh.surfaces.size() == surfs.size());
    // Get the modelview matrix, remove the translational component,
    // and adjust it so that it converts the result from OICS to OXCS 
    // for sphere mapping.
    FgMat44F     mvm,prj;
    glGetFloatv(GL_MODELVIEW_MATRIX,&mvm[0]);
    glGetFloatv(GL_PROJECTION_MATRIX,&prj[0]);
    mvm = mvm.transpose();
    prj = prj.transpose() * mvm;
    FgAffine3F      trans(mvm.subMatrix<3,3>(0,0));
    //FgVectF2        bnds(numeric_limits<float>::max(),numeric_limits<float>::min());
    //for (size_t ii=0; ii<verts.size(); ++ii) {
    //    FgVect4F    v = prj * fgAsHomogVec(verts[ii]);
    //    float   d = v[2] / v[3];
    //    fgSetIfLess(bnds[0],d);
    //    fgSetIfGreater(bnds[1],d); }
    //FG_HI1(bnds);
    FgAffine3F      oicsToOxcs(FgVect3F(1.0f));
    oicsToOxcs.postScale(0.5f);
    trans = oicsToOxcs * trans;
    for (uint ss=0; ss<mesh.surfaces.size(); ++ss)
    {
        if (!surfs[ss].visible)
            continue;

        if ((surfs[ss].xray && !xray) ||
            (!surfs[ss].xray && xray))
            continue;

        FgValid<uint>   texName;
        if (rend.useTexture && (ss < texNames.size()) && (texNames[ss] >= 0))
            texName = uint(texNames[ss]);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
        glEnable(GL_LIGHTING);

        const Fg3dSurface & surf = mesh.surfaces[ss];
        bool    doTex = (texName.valid() && (surf.hasUvIndices()));
        if (doTex)
        {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,GLuint(texName.val()));
            // If texture is on, we use two-pass rendering for true spectral and
            // we also use the per-object spectral surface properties. If texture
            // mode is off, all objects are rendered the same and in a single pass.
            float   alpha = xray ? 0.5f : 1.0f;
            GLfloat     white[]  = {1.0f, 1.0f, 1.0f, alpha},
                        black[]  = {0.0f, 0.0f, 0.0f, 1.0f};
            glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,white);
            glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,white);
            glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,black);
        }
        else
        {
            glDisable(GL_TEXTURE_2D);
            GLfloat     grey[]  = {0.8f, 0.8f, 0.8f, 1.0f},
                        green[] = {0.0f, 1.0f, 0.0f, 0.5f},
                        black[]  = {0.0f, 0.0f, 0.0f, 1.0f},
                        *clr = xray ? green : grey;
            glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,clr);
            glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,clr);
            glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,black);
        }

        // More bins slows things down without helping since depth sort is only approximate anyway:
        const size_t    numBins = 10000;
        FgAffine1F      depToBin(FgVectF2(1,-1),FgVectF2(0,numBins));
        vector<vector<Tri> >    tris(numBins);
        for (uint ii=0; ii<surf.numTris(); ii++) {
            Tri             tri;
            FgVect3UI       triInds = surf.getTri(ii);
            for (uint kk=0; kk<3; ++kk)
                tri.v[kk] = verts[triInds[kk]];
            if (rend.flatShaded)
                for (uint kk=0; kk<3; ++kk)
                    tri.n[kk] = norms.facet[ss].tri[ii];
            else
                for (uint kk=0; kk<3; ++kk)
                    tri.n[kk] = norms.vert[triInds[kk]];
            if (doTex) {
                FgVect3UI   texInds = surf.tris.uvInds[ii];
                for (uint kk=0; kk<3; ++kk)
                    tri.u[kk] = mesh.uvs[texInds[kk]]; }
            insertTri(tris,tri,prj,depToBin);
        }
        // Surface rasterization is done purely with triangles since some OGL drivers
        // can't handle quads:
        for (uint ii=0; ii<surf.numQuads(); ii++) {
            Tri             tri0,tri1;
            FgVect4UI       quadInds = surf.getQuad(ii);
            for (uint kk=0; kk<3; ++kk) {
                tri0.v[kk] = verts[quadInds[kk]];
                tri1.v[kk] = verts[quadInds[(kk+2)%4]]; }
            if (rend.flatShaded) {
                for (uint kk=0; kk<3; ++kk) {
                    tri0.n[kk] = norms.facet[ss].quad[ii];
                    tri1.n[kk] = norms.facet[ss].quad[ii]; }
            }
            else {
                for (uint kk=0; kk<3; ++kk) {
                    tri0.n[kk] = norms.vert[quadInds[kk]];
                    tri1.n[kk] = norms.vert[quadInds[(kk+2)%4]]; }
            }
            if (doTex) {
                FgVect4UI   texInds = surf.quads.uvInds[ii];
                for (uint kk=0; kk<3; ++kk) {
                    tri0.u[kk] = mesh.uvs[texInds[kk]];
                    tri1.u[kk] = mesh.uvs[texInds[(kk+2)%4]]; }
            }
            insertTri(tris,tri0,prj,depToBin);
            insertTri(tris,tri1,prj,depToBin);
        }

        glShadeModel(GL_SMOOTH);
        drawTris(tris);

        // Specular pass:
        if (rend.shiny)
        {
            for (size_t ii=0; ii<tris.size(); ++ii) {
                vector<Tri> & trs = tris[ii];
                for (size_t jj=0; jj<trs.size(); ++jj) {
                    Tri &       t = trs[jj];
                    for (uint kk=0; kk<3; ++kk) {
                        FgVect3F    tt = trans * t.n[kk];
                        t.u[kk] = FgVect2F(tt[0],tt[1]); }
                }
            }

            // No need to write depth buffer twice and we don't want it written in the
            // case of alpha textures:
            glDepthMask(0);
            glBlendFunc(GL_ONE,GL_ONE);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,s_specMapName);
            glColor3f(1.0f,1.0f,1.0f);
            glDisable(GL_LIGHTING);
            drawTris(tris);
            glDepthMask(1);                // Restore state
        }
        glDisable(GL_BLEND);
    }
}

string
fgOglGetInfo()
{
    FGASSERT(glGetError() == 0);

    const char *oglVendor, *oglRenderer, *oglVersion;
    oglVendor = (const char *)glGetString(GL_VENDOR);
    oglRenderer = (const char *)glGetString(GL_RENDERER);
    oglVersion = (const char *)glGetString(GL_VERSION);

    std::string oglInfo(oglVendor);
    oglInfo += ", " + std::string(oglRenderer);
    oglInfo += ", " + std::string(oglVersion);
    oglInfo += "\n";

    FGASSERT(glGetError() == 0);

    return oglInfo;
}

void
fgOglSetup()
{
    FGASSERT(glGetError() == 0);

    // Set up OGL rendering preferences:

    GLfloat     blackLight[] = {0.0,  0.0,  0.0,  1.0f};

    glEnable(GL_POLYGON_OFFSET_FILL);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT,blackLight);  // No global ambient.
    // Calculate spectral reflection approximating viewer at infinity (default):
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,0);

    glEnable(GL_DEPTH_TEST);

    // Enable rendering of both sides of each polygon as the default by
    // enabling proper lighting calculations for the back (clockwise) side.
    glDisable(GL_CULL_FACE);        // The OGL default
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE); // NOT the OGL default

    glReadBuffer(GL_BACK);          // Default but just in case, for glAccum().
    glDepthFunc(GL_LEQUAL);         // (default LESS) this allows the second render pass to work.

    // We probably don't need this anymore:
    glPixelStorei(GL_UNPACK_ALIGNMENT,8);

    // Repeat rather than clamp to avoid use of the border
    // color since some OGL 1.1 drivers (such as ATI) always
    // use the border color.
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);

    // We use trilinear texturing for quality. To force the use
    // of a single texture map we'd have to keep track of the view
    // window size in pixels.
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

    s_specMapName = fgOglTextureAdd(FgImgRgbaUb(128,128));

    FGASSERT(glGetError() == 0);
}

void
fgOglTextureUpdate(
    uint                name,
    const FgImgRgbaUb & img)
{
    FGASSERT(glGetError() == 0);

    // OGL requires power of 2 dimensioned images, and expects them stored bottom to top:
    FgImgRgbaUb     oglImg;
    fgPower2Ceil(img,oglImg);
    fgImgFlipVertical(oglImg);
    vector<FgImgRgbaUb>     mipmap = fgMipMap(oglImg);

    glEnable(GL_TEXTURE_2D);
    // Set this texture "name" as the current 2D texture "target":
    glBindTexture(GL_TEXTURE_2D,name);

    // The next line increments jj to the first pyramid level small enough
    // to be supported by this implementation of OGL. Note that we do not
    // attempt to ensure the texture fits in remaining GPU memory, just that it
    // is of a supported size.
    GLint tmp;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE,&tmp);
    // In order to avoid running out of memory (which has happened here since I gues the max
    // texture size can be such that it takes most memory, and resulting in NVIDIA driver
    // shutting down the application!) reduce by one pow2:
    uint        oglTexMax = static_cast<uint>(tmp/2);
    uint        dimMax = std::max(oglImg.width(),oglImg.height()),
                jj;
    for (jj=0; dimMax>oglTexMax; jj++) dimMax/=2;

    // Load into GPU:
    for (uint ii=jj; ii<=mipmap.size(); ii++) {
        const FgImgRgbaUb  *img;
        if (ii == 0)
            img = &oglImg;
        else
            img = &(mipmap[ii-1]);
        glTexImage2D(GL_TEXTURE_2D,
                     ii-jj,                 // Mipmap level being specified.
                     4,                     // 4 Channels
                     img->width(),
                     img->height(),
                     0,                     // No border colour
                     GL_RGBA,               // Channel order
                     GL_UNSIGNED_BYTE,      // Channel type
                     img->dataPtr());
    }

    // Note that although glTexSubImage2D can be used to update an already loaded
    // texture image more efficiently, this didn't work properly on some machines -
    // It would not update the texture if too much texture memory was being used 
    // (NT4, Gauss) or it would randomly corrupt all the textures (XP, beta tester).

    FGASSERT(glGetError() == 0);
}

uint
fgOglTextureAdd(
    const FgImgRgbaUb & img)
{
    FGASSERT(glGetError() == 0);
    glEnable(GL_TEXTURE_2D);
    uint name;
    glGenTextures(1,&name);
    fgOglTextureUpdate(name,img);
    return name;
}

static
void
rendSurfaces(
    const vector<FgOglRendModel> &  rms,
    Fg3dRenderOptions               rend,
    bool                            transparentPass)
{
    for (size_t ii=0; ii<rms.size(); ++ii) {
        const Fg3dMesh &    mesh = *(rms[ii].mesh);
        const FgVerts &     verts = *(rms[ii].verts);
        const Fg3dNormals & norms = *(rms[ii].norms);
        drawSurfaces(mesh,verts,norms,*(rms[ii].texNames),rend,transparentPass,rms[ii].rendSurfs);
    }
}

void
fgOglSetLighting(const FgLighting & lt)
{
    FGASSERT(glGetError() == 0);
    GLint       glLight[] = {GL_LIGHT0,GL_LIGHT1,GL_LIGHT2,GL_LIGHT3};
    glEnable(GL_LIGHTING);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();               // Lights are transformed by the current MVM
    FgVect4F    amb = fgConcatVert(lt.m_ambient,1.0f);
    glLightfv(glLight[0],GL_AMBIENT,amb.dataPtr());
    for (uint ll=0; (ll<lt.m_lights.size()) && (ll < 4); ll++)
    {
        const FgLight & lgt = lt.m_lights[ll];
        glEnable(glLight[ll]);
        FgVect4F        pos = fgConcatVert(lgt.m_direction,0.0f);
        glLightfv(glLight[ll],GL_POSITION,pos.dataPtr());
        FgVect4F        clr = fgConcatVert(lgt.m_colour,1.0f);
        glLightfv(glLight[ll],GL_DIFFUSE,clr.dataPtr());
    }
    fgOglTextureUpdate(s_specMapName,lt.createSpecularMap());
    FGASSERT(glGetError() == 0);
}

void
fgOglRender(
    vector<FgOglRendModel>          rms,
    FgMat44F                     oglMvm, // MVM in column-major layout.
    FgVect6D                        frustum,
    const Fg3dRenderOptions &       rend)
{
    FGASSERT(glGetError() == 0);

    if (rend.twoSided)
        glDisable(GL_CULL_FACE);        // The OGL default
    else {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(frustum[0],
              frustum[1],
              frustum[2],
              frustum[3],
              frustum[4],
              frustum[5]);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glLoadMatrixf(oglMvm.dataPtr());
    glClearColor(rend.backgroundColor[0],rend.backgroundColor[1],rend.backgroundColor[2],1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Using offsets creates cracks in facet rendering so only use when necessary:
    bool    useOffsets = (rend.wireframe || rend.markedVerts || rend.surfPoints);

    glEnable(GL_LIGHTING);
    if (rend.facets) {
        if (useOffsets)
            glPolygonOffset(1.0,1.0);   // Move the surface behind the wireframe.
        rendSurfaces(rms,rend,false);   // Opaque pass
        if (useOffsets)
            glPolygonOffset(0.0,0.0);
    }
    if (rend.wireframe) {
        glPolygonOffset(1.0,1.0);
        drawWires(rms);
        glPolygonOffset(0.0,0.0);
    }
    if (rend.allVerts)
        drawVerts(rms);
    if (rend.markedVerts)
        drawMarkedVerts(rms);
    if (rend.surfPoints)
        drawPoints(rms);
    if (rend.facets){
        if (useOffsets)
            glPolygonOffset(1.0,1.0);
        rendSurfaces(rms,rend,true);      // Transparent pass
        if (useOffsets)
            glPolygonOffset(0.0,0.0);
    }

    FGASSERT(glGetError() == 0);
}

FgMat44F
fgOglTransform()
{
    FgMat44F     mvm,prj;
    glGetFloatv(GL_PROJECTION_MATRIX,&prj[0]);
    glGetFloatv(GL_MODELVIEW_MATRIX,&mvm[0]);
    return prj.transpose() * mvm.transpose();
}

void
fgOglTexRelease(uint texName)
{
    FGASSERT(glGetError() == 0);
    glEnable(GL_TEXTURE_2D);
    glDeleteTextures(1,&texName);
    FGASSERT(glGetError() == 0);
}

// */
