//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Feb 28, 2005
//

#include "stdafx.h"

#include "FgMath.hpp"
#include "FgMatrixSolver.hpp"
#include "FgRandom.hpp"
#include "FgMain.hpp"

using namespace std;

void
fgSymmEigs(const FgMatrixD & rsm,FgMatrixD & val,FgMatrixD & vec)
{
    // JAMA enters an infinite loop with NaNs:
    for (size_t ii=0; ii<rsm.m_data.size(); ++ii)
        FGASSERT(boost::math::isfinite(rsm.m_data[ii]));
    uint                    dim = rsm.numRows();
    FGASSERT(rsm.numCols() == dim);
        // We use a const cast since we know 'solver' will not modify the elements, even though
        // the Array2D object holds a non-const pointer to our data.
    JAMA::Eigenvalue<double>
        solver(TNT::Array2D<double>(rsm.numRows(),rsm.numCols(),const_cast<double*>(rsm.dataPtr())));
    val.resize(dim,1);
    vec.resize(dim,dim);
    int                     idim = static_cast<int>(dim);
    for (int row=0; row<idim; row++) {
        val[row] = solver.d[row];
        for (uint col=0; col<dim; col++)
            vec.elem(row,col) = solver.V[row][col];
    }
}

// Complex division. Returns [real,imag]
static
FgVect2D
cdiv(double xr, double xi, double yr, double yi)
{
    FgVect2D    ret;
    double      r,d;
    if (abs(yr) > abs(yi)) {
        r = yi/yr;
        d = yr + r*yi;
        ret[0] = (xr + r*xi)/d;
        ret[1] = (xi - r*xr)/d;
    } else {
        r = yr/yi;
        d = yi + r*yr;
        ret[0] = (r*xr + xi)/d;
        ret[1] = (r*xi - xr)/d;
    }
    return ret;
}

FgEigs
fgEigs(const FgMatrixD & mat)
{
    FgEigs              ret;
    FGASSERT(mat.ncols == mat.nrows);
    // JAMA enters an infinite loop with NaNs:
    for (uint ii=0; ii<9; ++ii)
        FGASSERT(boost::math::isfinite(mat[ii]));
    ret.real.resize(mat.ncols);
    ret.imag.resize(mat.ncols);
    ret.vecs.resize(mat.dims());
    int                 n = int(mat.ncols);
    FgDbls              &d=ret.real,    // Eigenval real components
                        &e=ret.imag,    // Eigenval imag components
                        ort(mat.ncols);
    FgMatrixD           &V=ret.vecs,    // Eigvectors as respective column vectors
                        H = mat;        // Hessenberg form
    // Reduce to Hessenberg form:
    int low = 0;
    int high = n-1;
    for (int m = low+1; m <= high-1; m++) {
        // Scale column.
        double scale = 0.0;
        for (int i = m; i <= high; i++) {
            scale = scale + abs(H.elem(i,m-1));
        }
        if (scale != 0.0) {
            // Compute Householder transformation.
            double h = 0.0;
            for (int i = high; i >= m; i--) {
                ort[i] = H.elem(i,m-1)/scale;
                h += ort[i] * ort[i];
            }
            double g = sqrt(h);
            if (ort[m] > 0) {
                g = -g;
            }
            h = h - ort[m] * g;
            ort[m] = ort[m] - g;
            // Apply Householder similarity transformation
            // H = (I-u*u'/h)*H*(I-u*u')/h)
            for (int j = m; j < n; j++) {
                double f = 0.0;
                for (int i = high; i >= m; i--) {
                    f += ort[i]*H.elem(i,j);
                }
                f = f/h;
                for (int i = m; i <= high; i++) {
                    H.elem(i,j) -= f*ort[i];
                }
            }
   
            for (int i = 0; i <= high; i++) {
                double f = 0.0;
                for (int j = high; j >= m; j--) {
                    f += ort[j]*H.elem(i,j);
                }
                f = f/h;
                for (int j = m; j <= high; j++) {
                    H.elem(i,j) -= f*ort[j];
                }
            }
            ort[m] = scale*ort[m];
            H.elem(m,m-1) = scale*g;
        }
    }
    // Accumulate transformations (Algol's ortran).
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            V.elem(i,j) = (i == j ? 1.0 : 0.0);
        }
    }
    for (int m = high-1; m >= low+1; m--) {
        if (H.elem(m,m-1) != 0.0) {
            for (int i = m+1; i <= high; i++) {
                ort[i] = H.elem(i,m-1);
            }
            for (int j = m; j <= high; j++) {
                double g = 0.0;
                for (int i = m; i <= high; i++) {
                    g += ort[i] * V.elem(i,j);
                }
                // Double division avoids possible underflow
                g = (g / ort[m]) / H.elem(m,m-1);
                for (int i = m; i <= high; i++) {
                    V.elem(i,j) += g * ort[i];
                }
            }
        }
    }


    // Reduce Hessenberg to real Schur form:
    int nn = n;
    int ntmp = nn-1;
    low = 0;
    high = nn-1;
    double eps = pow(2.0,-52.0);
    double exshift = 0.0;
    double p=0,q=0,r=0,s=0,z=0,t,w,x,y;
    // Store roots isolated by balanc and compute matrix norm
    double norm = 0.0;
    for (int i = 0; i < nn; i++) {
        if ((i < low) || (i > high)) {
            d[i] = H.elem(i,i);
            e[i] = 0.0;
        }
        for (int j = max(i-1,0); j < nn; j++) {
            norm = norm + abs(H.elem(i,j));
        }
    }
    // Outer loop over eigenvalue index
    int iter = 0;
    while (ntmp >= low) {
        // Look for single small sub-diagonal element
        int l = ntmp;
        while (l > low) {
            s = abs(H.elem(l-1,l-1)) + abs(H.elem(l,l));
            if (s == 0.0) {
                s = norm;
            }
            if (abs(H.elem(l,l-1)) < eps * s) {
                break;
            }
            l--;
        }
        // Check for convergence
        // One root found
        if (l == ntmp) {
            H.elem(ntmp,ntmp) = H.elem(ntmp,ntmp) + exshift;
            d[ntmp] = H.elem(ntmp,ntmp);
            e[ntmp] = 0.0;
            ntmp--;
            iter = 0;
            // Two roots found
        }
        else if (l == ntmp-1) {
            w = H.elem(ntmp,ntmp-1) * H.elem(ntmp-1,ntmp);
            p = (H.elem(ntmp-1,ntmp-1) - H.elem(ntmp,ntmp)) / 2.0;
            q = p * p + w;
            z = sqrt(abs(q));
            H.elem(ntmp,ntmp) = H.elem(ntmp,ntmp) + exshift;
            H.elem(ntmp-1,ntmp-1) = H.elem(ntmp-1,ntmp-1) + exshift;
            x = H.elem(ntmp,ntmp);
            // double pair
            if (q >= 0) {
                if (p >= 0) {
                    z = p + z;
                }
                else {
                    z = p - z;
                }
                d[ntmp-1] = x + z;
                d[ntmp] = d[ntmp-1];
                if (z != 0.0) {
                    d[ntmp] = x - w / z;
                }
                e[ntmp-1] = 0.0;
                e[ntmp] = 0.0;
                x = H.elem(ntmp,ntmp-1);
                s = abs(x) + abs(z);
                p = x / s;
                q = z / s;
                r = sqrt(p * p+q * q);
                p = p / r;
                q = q / r;
                // Row modification
                for (int j = ntmp-1; j < nn; j++) {
                    z = H.elem(ntmp-1,j);
                    H.elem(ntmp-1,j) = q * z + p * H.elem(ntmp,j);
                    H.elem(ntmp,j) = q * H.elem(ntmp,j) - p * z;
                }
                // Column modification
                for (int i = 0; i <= ntmp; i++) {
                    z = H.elem(i,ntmp-1);
                    H.elem(i,ntmp-1) = q * z + p * H.elem(i,ntmp);
                    H.elem(i,ntmp) = q * H.elem(i,ntmp) - p * z;
                }
                // Accumulate transformations
                for (int i = low; i <= high; i++) {
                    z = V.elem(i,ntmp-1);
                    V.elem(i,ntmp-1) = q * z + p * V.elem(i,ntmp);
                    V.elem(i,ntmp) = q * V.elem(i,ntmp) - p * z;
                }
            // Complex pair
            }
            else {
                d[ntmp-1] = x + p;
                d[ntmp] = x + p;
                e[ntmp-1] = z;
                e[ntmp] = -z;
            }
            ntmp = ntmp - 2;
            iter = 0;
            // No convergence yet
        }
        else {
            // Form shift
            x = H.elem(ntmp,ntmp);
            y = 0.0;
            w = 0.0;
            if (l < ntmp) {
                y = H.elem(ntmp-1,ntmp-1);
                w = H.elem(ntmp,ntmp-1) * H.elem(ntmp-1,ntmp);
            }
            // Wilkinson's original ad hoc shift
            if (iter == 10) {
                exshift += x;
                for (int i = low; i <= ntmp; i++) {
                    H.elem(i,i) -= x;
                }
                s = abs(H.elem(ntmp,ntmp-1)) + abs(H.elem(ntmp-1,ntmp-2));
                x = y = 0.75 * s;
                w = -0.4375 * s * s;
            }
            // MATLAB's new ad hoc shift
            if (iter == 30) {
                s = (y - x) / 2.0;
                s = s * s + w;
                if (s > 0) {
                    s = sqrt(s);
                    if (y < x) {
                        s = -s;
                    }
                    s = x - w / ((y - x) / 2.0 + s);
                    for (int i = low; i <= ntmp; i++) {
                        H.elem(i,i) -= s;
                    }
                    exshift += s;
                    x = y = w = double(0.964);
                }
            }
            iter = iter + 1;   // (Could check iteration count here.)
            // Look for two consecutive small sub-diagonal elements
            int m = ntmp-2;
            while (m >= l) {
                z = H.elem(m,m);
                r = x - z;
                s = y - z;
                p = (r * s - w) / H.elem(m+1,m) + H.elem(m,m+1);
                q = H.elem(m+1,m+1) - z - r - s;
                r = H.elem(m+2,m+1);
                s = abs(p) + abs(q) + abs(r);
                p = p / s;
                q = q / s;
                r = r / s;
                if (m == l) {
                    break;
                }
                if (abs(H.elem(m,m-1)) * (abs(q) + abs(r)) <
                    eps * (abs(p) * (abs(H.elem(m-1,m-1)) + abs(z) +
                    abs(H.elem(m+1,m+1))))) {
                        break;
                }
                m--;
            }
            for (int i = m+2; i <= ntmp; i++) {
                H.elem(i,i-2) = 0.0;
                if (i > m+2) {
                    H.elem(i,i-3) = 0.0;
                }
            }
            // Double QR step involving rows l:ntmp and columns m:ntmp
            for (int k = m; k <= ntmp-1; k++) {
                int notlast = (k != ntmp-1);
                if (k != m) {
                    p = H.elem(k,k-1);
                    q = H.elem(k+1,k-1);
                    r = (notlast ? H.elem(k+2,k-1) : 0.0);
                    x = abs(p) + abs(q) + abs(r);
                    if (x != 0.0) {
                        p = p / x;
                        q = q / x;
                        r = r / x;
                    }
                }
                if (x == 0.0) {
                    break;
                }
                s = sqrt(p * p + q * q + r * r);
                if (p < 0) {
                    s = -s;
                }
                if (s != 0) {
                    if (k != m) {
                        H.elem(k,k-1) = -s * x;
                    }
                    else if (l != m) {
                        H.elem(k,k-1) = -H.elem(k,k-1);
                    }
                    p = p + s;
                    x = p / s;
                    y = q / s;
                    z = r / s;
                    q = q / p;
                    r = r / p;
                    // Row modification
                    for (int j = k; j < nn; j++) {
                        p = H.elem(k,j) + q * H.elem(k+1,j);
                        if (notlast) {
                            p = p + r * H.elem(k+2,j);
                            H.elem(k+2,j) = H.elem(k+2,j) - p * z;
                        }
                        H.elem(k,j) = H.elem(k,j) - p * x;
                        H.elem(k+1,j) = H.elem(k+1,j) - p * y;
                    }
                    // Column modification
                    for (int i = 0; i <= min(ntmp,k+3); i++) {
                        p = x * H.elem(i,k) + y * H.elem(i,k+1);
                        if (notlast) {
                            p = p + z * H.elem(i,k+2);
                            H.elem(i,k+2) = H.elem(i,k+2) - p * r;
                        }
                        H.elem(i,k) = H.elem(i,k) - p;
                        H.elem(i,k+1) = H.elem(i,k+1) - p * q;
                    }
                    // Accumulate transformations
                    for (int i = low; i <= high; i++) {
                        p = x * V.elem(i,k) + y * V.elem(i,k+1);
                        if (notlast) {
                            p = p + z * V.elem(i,k+2);
                            V.elem(i,k+2) = V.elem(i,k+2) - p * r;
                        }
                        V.elem(i,k) = V.elem(i,k) - p;
                        V.elem(i,k+1) = V.elem(i,k+1) - p * q;
                    }
                }  // (s != 0)
            }  // k loop
        }  // check convergence
    }  // while (ntmp >= low)
    // Backsubstitute to find vectors of upper triangular form
    if (norm != 0.0) {
        for (ntmp = nn-1; ntmp >= 0; ntmp--) {
            p = d[ntmp];
            q = e[ntmp];
            // double vector
            if (q == 0) {
                int l = ntmp;
                H.elem(ntmp,ntmp) = 1.0;
                for (int i = ntmp-1; i >= 0; i--) {
                    w = H.elem(i,i) - p;
                    r = 0.0;
                    for (int j = l; j <= ntmp; j++) {
                        r = r + H.elem(i,j) * H.elem(j,ntmp);
                    }
                    if (e[i] < 0.0) {
                        z = w;
                        s = r;
                    }
                    else {
                        l = i;
                        if (e[i] == 0.0) {
                            if (w != 0.0) {
                                H.elem(i,ntmp) = -r / w;
                            }
                            else {
                                H.elem(i,ntmp) = -r / (eps * norm);
                            }
                        // Solve real equations
                        }
                        else {
                            x = H.elem(i,i+1);
                            y = H.elem(i+1,i);
                            q = (d[i] - p) * (d[i] - p) + e[i] * e[i];
                            t = (x * s - z * r) / q;
                            H.elem(i,ntmp) = t;
                            if (abs(x) > abs(z)) {
                                H.elem(i+1,ntmp) = (-r - w * t) / x;
                            }
                            else {
                                H.elem(i+1,ntmp) = (-s - y * t) / z;
                            }
                        }
                        // Overflow control
                        t = abs(H.elem(i,ntmp));
                        if ((eps * t) * t > 1) {
                            for (int j = i; j <= ntmp; j++) {
                                H.elem(j,ntmp) = H.elem(j,ntmp) / t;
                            }
                        }
                    }
                }
            // Complex vector
            }
            else if (q < 0) {
                int l = ntmp-1;
                // Last vector component imaginary so matrix is triangular
                if (abs(H.elem(ntmp,ntmp-1)) > abs(H.elem(ntmp-1,ntmp))) {
                    H.elem(ntmp-1,ntmp-1) = q / H.elem(ntmp,ntmp-1);
                    H.elem(ntmp-1,ntmp) = -(H.elem(ntmp,ntmp) - p) / H.elem(ntmp,ntmp-1);
                }
                else {
                    FgVect2D    cn = cdiv(0.0,-H.elem(ntmp-1,ntmp),H.elem(ntmp-1,ntmp-1)-p,q);
                    H.elem(ntmp-1,ntmp-1) = cn[0];
                    H.elem(ntmp-1,ntmp) = cn[1];
                }
                H.elem(ntmp,ntmp-1) = 0.0;
                H.elem(ntmp,ntmp) = 1.0;
                for (int i = ntmp-2; i >= 0; i--) {
                    double ra,sa,vr,vi;
                    ra = 0.0;
                    sa = 0.0;
                    for (int j = l; j <= ntmp; j++) {
                        ra = ra + H.elem(i,j) * H.elem(j,ntmp-1);
                        sa = sa + H.elem(i,j) * H.elem(j,ntmp);
                    }
                    w = H.elem(i,i) - p;
                    if (e[i] < 0.0) {
                        z = w;
                        r = ra;
                        s = sa;
                    } else {
                        l = i;
                        if (e[i] == 0) {
                            FgVect2D    cn = cdiv(-ra,-sa,w,q);
                            H.elem(i,ntmp-1) = cn[0];
                            H.elem(i,ntmp) = cn[1];
                        }
                        else {
                            // Solve complex equations
                            x = H.elem(i,i+1);
                            y = H.elem(i+1,i);
                            vr = (d[i] - p) * (d[i] - p) + e[i] * e[i] - q * q;
                            vi = (d[i] - p) * 2.0 * q;
                            if ((vr == 0.0) && (vi == 0.0)) {
                                vr = eps * norm * (abs(w) + abs(q) +
                                abs(x) + abs(y) + abs(z));
                            }
                            FgVect2D    cn = cdiv(x*r-z*ra+q*sa,x*s-z*sa-q*ra,vr,vi);
                            H.elem(i,ntmp-1) = cn[0];
                            H.elem(i,ntmp) = cn[1];
                            if (abs(x) > (abs(z) + abs(q))) {
                                H.elem(i+1,ntmp-1) = (-ra - w * H.elem(i,ntmp-1) + q * H.elem(i,ntmp)) / x;
                                H.elem(i+1,ntmp) = (-sa - w * H.elem(i,ntmp) - q * H.elem(i,ntmp-1)) / x;
                            }
                            else {
                                FgVect2D    cn2 = cdiv(-r-y*H.elem(i,ntmp-1),-s-y*H.elem(i,ntmp),z,q);
                                H.elem(i+1,ntmp-1) = cn2[0];
                                H.elem(i+1,ntmp) = cn2[1];
                            }
                        }
                        // Overflow control
                        t = max(abs(H.elem(i,ntmp-1)),abs(H.elem(i,ntmp)));
                        if ((eps * t) * t > 1) {
                            for (int j = i; j <= ntmp; j++) {
                                H.elem(j,ntmp-1) = H.elem(j,ntmp-1) / t;
                                H.elem(j,ntmp) = H.elem(j,ntmp) / t;
                            }
                        }
                    }
                }
            }
        }
        // Vectors of isolated roots
        for (int i = 0; i < nn; i++) {
            if (i < low || i > high) {
                for (int j = i; j < nn; j++) {
                    V.elem(i,j) = H.elem(i,j);
                }
            }
        }
        // Back transformation to get eigenvectors of original matrix
        for (int j = nn-1; j >= low; j--) {
            for (int i = low; i <= high; i++) {
                z = 0.0;
                for (int k = low; k <= min(j,high); k++) {
                    z = z + V.elem(i,k) * H.elem(k,j);
                }
                V.elem(i,j) = z;
            }
        }
    }
    return ret;
}

FgEigsC<double,3>
fgEigs(const FgMat33D & mat)
{
    FgEigsC<double,3>    ret;
    // JAMA enters an infinite loop with NaNs:
    for (uint ii=0; ii<9; ++ii)
        FGASSERT(boost::math::isfinite(mat[ii]));
    FgVect3D            &d=ret.real,    // Eigenval real components
                        &e=ret.imag,    // Eigenval imag components
                        ort;
    FgMat33D            &V=ret.vecs,    // Eigvectors as respective column vectors
                        H = mat;        // Hessenberg form
    // Reduce to Hessenberg form:
    // Scale column.
    double scale = abs(H.elem(1,0)) + abs(H.elem(2,0));
    if (scale != 0.0) {
        // Compute Householder transformation.
        double h = 0.0;
        for (int i = 2; i >= 1; i--) {
            ort[i] = H.elem(i,0)/scale;
            h += ort[i] * ort[i];
        }
        double g = sqrt(h);
        if (ort[1] > 0)
            g = -g;
        h = h - ort[1] * g;
        ort[1] = ort[1] - g;
        // Apply Householder similarity transformation
        // H = (I-u*u'/h)*H*(I-u*u')/h)
        for (int j = 1; j < 3; j++) {
            double f = 0.0;
            for (int i = 2; i >= 1; i--)
                f += ort[i]*H.elem(i,j);
            f = f/h;
            for (int i = 1; i <= 2; i++)
                H.elem(i,j) -= f*ort[i];
        }
        for (int i = 0; i <= 2; i++) {
            double f = 0.0;
            for (int j = 2; j >= 1; j--)
                f += ort[j]*H.elem(i,j);
            f = f/h;
            for (int j = 1; j <= 2; j++)
                H.elem(i,j) -= f*ort[j];
        }
        ort[1] = scale*ort[1];
        H.elem(1,0) = scale*g;
    }
    // Accumulate transformations (Algol's ortran).
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            V.elem(i,j) = (i == j ? 1.0 : 0.0);
        }
    }
    if (H.elem(1,0) != 0.0) {
        ort[2] = H.elem(2,0);
        for (int j = 1; j <= 2; j++) {
            double g = 0.0;
            for (int i = 1; i <= 2; i++)
                g += ort[i] * V.elem(i,j);
            // Double division avoids possible underflow
            g = (g / ort[1]) / H.elem(1,0);
            for (int i = 1; i <= 2; i++)
                V.elem(i,j) += g * ort[i];
        }
    }


    // Reduce Hessenberg to real Schur form:
    int ntmp = 2;
    double eps = pow(2.0,-52.0);
    double exshift = 0.0;
    double p=0,q=0,r=0,s=0,z=0,t,w,x,y;
    // Store roots isolated by balanc and compute matrix norm
    double norm = 0.0;
    for (int i = 0; i < 3; i++) {
        for (int j = max(i-1,0); j < 3; j++) {
            norm = norm + abs(H.elem(i,j));
        }
    }
    // Outer loop over eigenvalue index
    int iter = 0;
    while (ntmp >= 0) {
        // Look for single small sub-diagonal element
        int l = ntmp;
        while (l > 0) {
            s = abs(H.elem(l-1,l-1)) + abs(H.elem(l,l));
            if (s == 0.0)
                s = norm;
            if (abs(H.elem(l,l-1)) < eps * s)
                break;
            l--;
        }
        // Check for convergence
        // One root found
        if (l == ntmp) {
            H.elem(ntmp,ntmp) = H.elem(ntmp,ntmp) + exshift;
            d[ntmp] = H.elem(ntmp,ntmp);
            e[ntmp] = 0.0;
            ntmp--;
            iter = 0;
            // Two roots found
        }
        else if (l == ntmp-1) {
            w = H.elem(ntmp,ntmp-1) * H.elem(ntmp-1,ntmp);
            p = (H.elem(ntmp-1,ntmp-1) - H.elem(ntmp,ntmp)) / 2.0;
            q = p * p + w;
            z = sqrt(abs(q));
            H.elem(ntmp,ntmp) = H.elem(ntmp,ntmp) + exshift;
            H.elem(ntmp-1,ntmp-1) = H.elem(ntmp-1,ntmp-1) + exshift;
            x = H.elem(ntmp,ntmp);
            // double pair
            if (q >= 0) {
                if (p >= 0)
                    z = p + z;
                else
                    z = p - z;
                d[ntmp-1] = x + z;
                d[ntmp] = d[ntmp-1];
                if (z != 0.0)
                    d[ntmp] = x - w / z;
                e[ntmp-1] = 0.0;
                e[ntmp] = 0.0;
                x = H.elem(ntmp,ntmp-1);
                s = abs(x) + abs(z);
                p = x / s;
                q = z / s;
                r = sqrt(p * p+q * q);
                p = p / r;
                q = q / r;
                // Row modification
                for (int j = ntmp-1; j < 3; j++) {
                    z = H.elem(ntmp-1,j);
                    H.elem(ntmp-1,j) = q * z + p * H.elem(ntmp,j);
                    H.elem(ntmp,j) = q * H.elem(ntmp,j) - p * z;
                }
                // Column modification
                for (int i = 0; i <= ntmp; i++) {
                    z = H.elem(i,ntmp-1);
                    H.elem(i,ntmp-1) = q * z + p * H.elem(i,ntmp);
                    H.elem(i,ntmp) = q * H.elem(i,ntmp) - p * z;
                }
                // Accumulate transformations
                for (int i = 0; i <= 2; i++) {
                    z = V.elem(i,ntmp-1);
                    V.elem(i,ntmp-1) = q * z + p * V.elem(i,ntmp);
                    V.elem(i,ntmp) = q * V.elem(i,ntmp) - p * z;
                }
            // Complex pair
            }
            else {
                d[ntmp-1] = x + p;
                d[ntmp] = x + p;
                e[ntmp-1] = z;
                e[ntmp] = -z;
            }
            ntmp = ntmp - 2;
            iter = 0;
            // No convergence yet
        }
        else {
            // Form shift
            x = H.elem(ntmp,ntmp);
            y = 0.0;
            w = 0.0;
            if (l < ntmp) {
                y = H.elem(ntmp-1,ntmp-1);
                w = H.elem(ntmp,ntmp-1) * H.elem(ntmp-1,ntmp);
            }
            // Wilkinson's original ad hoc shift
            if (iter == 10) {
                exshift += x;
                for (int i = 0; i <= ntmp; i++)
                    H.elem(i,i) -= x;
                s = abs(H.elem(ntmp,ntmp-1)) + abs(H.elem(ntmp-1,ntmp-2));
                x = y = 0.75 * s;
                w = -0.4375 * s * s;
            }
            // MATLAB's new ad hoc shift
            if (iter == 30) {
                s = (y - x) / 2.0;
                s = s * s + w;
                if (s > 0) {
                    s = sqrt(s);
                    if (y < x)
                        s = -s;
                    s = x - w / ((y - x) / 2.0 + s);
                    for (int i = 0; i <= ntmp; i++)
                        H.elem(i,i) -= s;
                    exshift += s;
                    x = y = w = double(0.964);
                }
            }
            ++iter; // (Could check iteration count here.)
            // Look for two consecutive small sub-diagonal elements
            int m = ntmp-2;
            while (m >= l) {
                z = H.elem(m,m);
                r = x - z;
                s = y - z;
                p = (r * s - w) / H.elem(m+1,m) + H.elem(m,m+1);
                q = H.elem(m+1,m+1) - z - r - s;
                r = H.elem(m+2,m+1);
                s = abs(p) + abs(q) + abs(r);
                p = p / s;
                q = q / s;
                r = r / s;
                if (m == l)
                    break;
                if (abs(H.elem(m,m-1)) * (abs(q) + abs(r)) <
                    eps * (abs(p) * (abs(H.elem(m-1,m-1)) + abs(z) +
                    abs(H.elem(m+1,m+1))))) {
                        break;
                }
                m--;
            }
            for (int i = m+2; i <= ntmp; i++) {
                H.elem(i,i-2) = 0.0;
                if (i > m+2)
                    H.elem(i,i-3) = 0.0;
            }
            // Double QR step involving rows l:ntmp and columns m:ntmp
            for (int k = m; k <= ntmp-1; k++) {
                int notlast = (k != ntmp-1);
                if (k != m) {
                    p = H.elem(k,k-1);
                    q = H.elem(k+1,k-1);
                    r = (notlast ? H.elem(k+2,k-1) : 0.0);
                    x = abs(p) + abs(q) + abs(r);
                    if (x != 0.0) {
                        p = p / x;
                        q = q / x;
                        r = r / x;
                    }
                }
                if (x == 0.0)
                    break;
                s = sqrt(p * p + q * q + r * r);
                if (p < 0)
                    s = -s;
                if (s != 0) {
                    if (k != m)
                        H.elem(k,k-1) = -s * x;
                    else if (l != m)
                        H.elem(k,k-1) = -H.elem(k,k-1);
                    p = p + s;
                    x = p / s;
                    y = q / s;
                    z = r / s;
                    q = q / p;
                    r = r / p;
                    // Row modification
                    for (int j = k; j < 3; j++) {
                        p = H.elem(k,j) + q * H.elem(k+1,j);
                        if (notlast) {
                            p = p + r * H.elem(k+2,j);
                            H.elem(k+2,j) = H.elem(k+2,j) - p * z;
                        }
                        H.elem(k,j) = H.elem(k,j) - p * x;
                        H.elem(k+1,j) = H.elem(k+1,j) - p * y;
                    }
                    // Column modification
                    for (int i = 0; i <= min(ntmp,k+3); i++) {
                        p = x * H.elem(i,k) + y * H.elem(i,k+1);
                        if (notlast) {
                            p = p + z * H.elem(i,k+2);
                            H.elem(i,k+2) = H.elem(i,k+2) - p * r;
                        }
                        H.elem(i,k) = H.elem(i,k) - p;
                        H.elem(i,k+1) = H.elem(i,k+1) - p * q;
                    }
                    // Accumulate transformations
                    for (int i = 0; i <= 2; i++) {
                        p = x * V.elem(i,k) + y * V.elem(i,k+1);
                        if (notlast) {
                            p = p + z * V.elem(i,k+2);
                            V.elem(i,k+2) = V.elem(i,k+2) - p * r;
                        }
                        V.elem(i,k) = V.elem(i,k) - p;
                        V.elem(i,k+1) = V.elem(i,k+1) - p * q;
                    }
                }  // (s != 0)
            }  // k loop
        }  // check convergence
    }  // while (ntmp >= 0)
    // Backsubstitute to find vectors of upper triangular form
    if (norm != 0.0) {
        for (ntmp = 2; ntmp >= 0; ntmp--) {
            p = d[ntmp];
            q = e[ntmp];
            // double vector
            if (q == 0) {
                int l = ntmp;
                H.elem(ntmp,ntmp) = 1.0;
                for (int i = ntmp-1; i >= 0; i--) {
                    w = H.elem(i,i) - p;
                    r = 0.0;
                    for (int j = l; j <= ntmp; j++)
                        r = r + H.elem(i,j) * H.elem(j,ntmp);
                    if (e[i] < 0.0) {
                        z = w;
                        s = r;
                    }
                    else {
                        l = i;
                        if (e[i] == 0.0) {
                            if (w != 0.0)
                                H.elem(i,ntmp) = -r / w;
                            else
                                H.elem(i,ntmp) = -r / (eps * norm);
                        }
                        // Solve real equations
                        else {
                            x = H.elem(i,i+1);
                            y = H.elem(i+1,i);
                            q = (d[i] - p) * (d[i] - p) + e[i] * e[i];
                            t = (x * s - z * r) / q;
                            H.elem(i,ntmp) = t;
                            if (abs(x) > abs(z))
                                H.elem(i+1,ntmp) = (-r - w * t) / x;
                            else
                                H.elem(i+1,ntmp) = (-s - y * t) / z;
                        }
                        // Overflow control
                        t = abs(H.elem(i,ntmp));
                        if ((eps * t) * t > 1) {
                            for (int j = i; j <= ntmp; j++)
                                H.elem(j,ntmp) = H.elem(j,ntmp) / t;
                        }
                    }
                }
            }
            // Complex vector
            else if (q < 0) {
                int l = ntmp-1;
                // Last vector component imaginary so matrix is triangular
                if (abs(H.elem(ntmp,ntmp-1)) > abs(H.elem(ntmp-1,ntmp))) {
                    H.elem(ntmp-1,ntmp-1) = q / H.elem(ntmp,ntmp-1);
                    H.elem(ntmp-1,ntmp) = -(H.elem(ntmp,ntmp) - p) / H.elem(ntmp,ntmp-1);
                }
                else {
                    FgVect2D    cn = cdiv(0.0,-H.elem(ntmp-1,ntmp),H.elem(ntmp-1,ntmp-1)-p,q);
                    H.elem(ntmp-1,ntmp-1) = cn[0];
                    H.elem(ntmp-1,ntmp) = cn[1];
                }
                H.elem(ntmp,ntmp-1) = 0.0;
                H.elem(ntmp,ntmp) = 1.0;
                for (int i = ntmp-2; i >= 0; i--) {
                    double ra,sa,vr,vi;
                    ra = 0.0;
                    sa = 0.0;
                    for (int j = l; j <= ntmp; j++) {
                        ra = ra + H.elem(i,j) * H.elem(j,ntmp-1);
                        sa = sa + H.elem(i,j) * H.elem(j,ntmp);
                    }
                    w = H.elem(i,i) - p;
                    if (e[i] < 0.0) {
                        z = w;
                        r = ra;
                        s = sa;
                    }
                    else {
                        l = i;
                        if (e[i] == 0) {
                            FgVect2D    cn = cdiv(-ra,-sa,w,q);
                            H.elem(i,ntmp-1) = cn[0];
                            H.elem(i,ntmp) = cn[1];
                        }
                        else {
                            // Solve complex equations
                            x = H.elem(i,i+1);
                            y = H.elem(i+1,i);
                            vr = (d[i] - p) * (d[i] - p) + e[i] * e[i] - q * q;
                            vi = (d[i] - p) * 2.0 * q;
                            if ((vr == 0.0) && (vi == 0.0)) {
                                vr = eps * norm * (abs(w) + abs(q) +
                                abs(x) + abs(y) + abs(z));
                            }
                            FgVect2D    cn = cdiv(x*r-z*ra+q*sa,x*s-z*sa-q*ra,vr,vi);
                            H.elem(i,ntmp-1) = cn[0];
                            H.elem(i,ntmp) = cn[1];
                            if (abs(x) > (abs(z) + abs(q))) {
                                H.elem(i+1,ntmp-1) = (-ra - w * H.elem(i,ntmp-1) + q * H.elem(i,ntmp)) / x;
                                H.elem(i+1,ntmp) = (-sa - w * H.elem(i,ntmp) - q * H.elem(i,ntmp-1)) / x;
                            }
                            else {
                                FgVect2D    cn2 = cdiv(-r-y*H.elem(i,ntmp-1),-s-y*H.elem(i,ntmp),z,q);
                                H.elem(i+1,ntmp-1) = cn2[0];
                                H.elem(i+1,ntmp) = cn2[1];
                            }
                        }
                        // Overflow control
                        t = max(abs(H.elem(i,ntmp-1)),abs(H.elem(i,ntmp)));
                        if ((eps * t) * t > 1) {
                            for (int j = i; j <= ntmp; j++) {
                                H.elem(j,ntmp-1) = H.elem(j,ntmp-1) / t;
                                H.elem(j,ntmp) = H.elem(j,ntmp) / t;
                            }
                        }
                    }
                }
            }
        }
        // Back transformation to get eigenvectors of original matrix
        for (int j = 2; j >= 0; j--) {
            for (int i = 0; i <= 2; i++) {
                z = 0.0;
                for (int k = 0; k <= min(j,2); k++)
                    z = z + V.elem(i,k) * H.elem(k,j);
                V.elem(i,j) = z;
            }
        }
    }
    return ret;
}

static
void
testSymmEigenProblem(uint dim)
{
    FgMatrixD  mat(dim,dim);
    for (uint ii=0; ii<dim; ii++) {
        for (uint jj=ii; jj<dim; jj++) {
            mat.elem(ii,jj) = fgRand();
            mat.elem(jj,ii) = mat.elem(ii,jj);
        }
    }
    FgMatrixD  eigVals,eigVecs;
    //clock_t     clk = clock();
    fgSymmEigs(mat,eigVals,eigVecs);
    //float       time = ((float)clock() - (float)clk) / (float)CLOCKS_PER_SEC;
    FgMatrixD  eigValMat(dim,dim);
    eigValMat.setZero();
    for (uint ii=0; ii<dim; ii++)
        eigValMat.elem(ii,ii) = eigVals[ii];
    FgMatrixD  recon = eigVecs * eigValMat * eigVecs.transpose();
    double      residual = 0.0;
    for (uint ii=0; ii<dim; ii++)
        for (uint jj=ii; jj<dim; jj++)
            residual += abs(recon.elem(ii,jj) - mat.elem(ii,jj));
    // Now we can't expect matrix diagonalization to be accurate to machine
    // precision. In fact, depending on how extreme the eigenvalues are, the precision
    // can be arbitrarily poor. Experiments with random matrices show that the 
    // average absolute error increases slightly faster than the SQUARE of the dimension.
    // We need a fudge factor of 5 by the time we get to 300x300.
    double      tol = (dim*dim) * 5.0 * numeric_limits<double>::epsilon();
    //HI_MOM4(dim,residual,tol,time);
    FGASSERT(residual < tol);
}

static
void
testAsymEigs()
{
    fgout << fgnl << fgEigs(fgMatRotateAxis(1.0,FgVect3D(1,1,1)));
}

void
fgMatrixSolverTest(const FgArgs &)
{
    fgRandSeedRepeatable();
    testSymmEigenProblem(10);
    testSymmEigenProblem(30);       // Larger than this is too slow in debug compile.
    testAsymEigs();
//    testSymmEigenProblem(100);
//    testSymmEigenProblem(1000);
}

