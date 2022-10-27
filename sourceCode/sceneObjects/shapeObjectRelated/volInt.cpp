/*******************************************************
*                                                      *
*  volInt.c                                            *
*                                                      *
*  This code computes volume integrals needed for      *
*  determining mass properties of polyhedral bodies.   *
*                                                      *
*  For more information, see the accompanying README   *
*  file, and the paper                                 *
*                                                      *
*  Brian Mirtich, "Fast and Accurate Computation of    *
*  Polyhedral Mass Properties," journal of graphics    *
*  tools, volume 1, number 1, 1996.                    *
*                                                      *
*  This source code is public domain, and may be used  *
*  in any way, shape or form, free of charge.          *
*                                                      *
*  Copyright 1995 by Brian Mirtich                     *
*                                                      *
*  mirtich@cs.berkeley.edu                             *
*  http://www.cs.berkeley.edu/~mirtich                 *
*                                                      *
*******************************************************/

/*
    Revision history

    26 Jan 1996 Program creation.

     3 Aug 1996 Corrected bug arising when polyhedron density
            is not 1.0.  Changes confined to function main().
            Thanks to Zoran Popovic for catching this one.

    27 May 1997     Corrected sign error in translation of inertia
                    product terms to center of mass frame.  Changes 
            confined to function main().  Thanks to 
            Chris Hecker.

    28 July 2015 Code adapted by Coppelia Robotics AG

*/


#include "volInt.h"
#include <math.h>

int CVolInt::A;   // alpha
int CVolInt::B;   // beta
int CVolInt::C;   // gamma

// projection integrals
double CVolInt::P1;
double CVolInt::Pa;
double CVolInt::Pb;
double CVolInt::Paa;
double CVolInt::Pab;
double CVolInt::Pbb;
double CVolInt::Paaa;
double CVolInt::Paab;
double CVolInt::Pabb;
double CVolInt::Pbbb;

// face integrals
double CVolInt::Fa;
double CVolInt::Fb;
double CVolInt::Fc;
double CVolInt::Faa;
double CVolInt::Fbb;
double CVolInt::Fcc;
double CVolInt::Faaa;
double CVolInt::Fbbb;
double CVolInt::Fccc;
double CVolInt::Faab;
double CVolInt::Fbbc;
double CVolInt::Fcca;

// volume integrals
double CVolInt::T0;
double CVolInt::T1[3];
double CVolInt::T2[3];
double CVolInt::TP[3];

/*
   ============================================================================
   compute mass properties
   ============================================================================
*/
/* compute various integrations over projection of face */
void CVolInt::compProjectionIntegrals(FACE *f)
{
  double a0, a1, da;
  double b0, b1, db;
  double a0_2, a0_3, a0_4, b0_2, b0_3, b0_4;
  double a1_2, a1_3, b1_2, b1_3;
  double C1, Ca, Caa, Caaa, Cb, Cbb, Cbbb;
  double Cab, Kab, Caab, Kaab, Cabb, Kabb;
  int i;

  P1 = Pa = Pb = Paa = Pab = Pbb = Paaa = Paab = Pabb = Pbbb = 0.0;

  for (i = 0; i < 3; i++) {
    a0 = f->poly->verts[3*f->vertsInd[i]+A];
    b0 = f->poly->verts[3*f->vertsInd[i]+B];
    a1 = f->poly->verts[3*f->vertsInd[(i+1) % 3]+A];
    b1 = f->poly->verts[3*f->vertsInd[(i+1) % 3]+B];
    da = a1 - a0;
    db = b1 - b0;
    a0_2 = a0 * a0; a0_3 = a0_2 * a0; a0_4 = a0_3 * a0;
    b0_2 = b0 * b0; b0_3 = b0_2 * b0; b0_4 = b0_3 * b0;
    a1_2 = a1 * a1; a1_3 = a1_2 * a1; 
    b1_2 = b1 * b1; b1_3 = b1_2 * b1;

    C1 = a1 + a0;
    Ca = a1*C1 + a0_2; Caa = a1*Ca + a0_3; Caaa = a1*Caa + a0_4;
    Cb = b1*(b1 + b0) + b0_2; Cbb = b1*Cb + b0_3; Cbbb = b1*Cbb + b0_4;
    Cab = 3*a1_2 + 2*a1*a0 + a0_2; Kab = a1_2 + 2*a1*a0 + 3*a0_2;
    Caab = a0*Cab + 4*a1_3; Kaab = a1*Kab + 4*a0_3;
    Cabb = 4*b1_3 + 3*b1_2*b0 + 2*b1*b0_2 + b0_3;
    Kabb = b1_3 + 2*b1_2*b0 + 3*b1*b0_2 + 4*b0_3;

    P1 += db*C1;
    Pa += db*Ca;
    Paa += db*Caa;
    Paaa += db*Caaa;
    Pb += da*Cb;
    Pbb += da*Cbb;
    Pbbb += da*Cbbb;
    Pab += db*(b1*Cab + b0*Kab);
    Paab += db*(b1*Caab + b0*Kaab);
    Pabb += da*(a1*Cabb + a0*Kabb);
  }

  P1 /= 2.0;
  Pa /= 6.0;
  Paa /= 12.0;
  Paaa /= 20.0;
  Pb /= -6.0;
  Pbb /= -12.0;
  Pbbb /= -20.0;
  Pab /= 24.0;
  Paab /= 60.0;
  Pabb /= -60.0;
}

void CVolInt::compFaceIntegrals(FACE *f)
{
  double *n, w;
  double k1, k2, k3, k4;

  compProjectionIntegrals(f);

  w = f->w;
  n = f->norm;
  k1 = 1 / n[C]; k2 = k1 * k1; k3 = k2 * k1; k4 = k3 * k1;

  Fa = k1 * Pa;
  Fb = k1 * Pb;
  Fc = -k2 * (n[A]*Pa + n[B]*Pb + w*P1);

  Faa = k1 * Paa;
  Fbb = k1 * Pbb;
  Fcc = k3 * (SQR(n[A])*Paa + 2*n[A]*n[B]*Pab + SQR(n[B])*Pbb
     + w*(2*(n[A]*Pa + n[B]*Pb) + w*P1));

  Faaa = k1 * Paaa;
  Fbbb = k1 * Pbbb;
  Fccc = -k4 * (CUBE(n[A])*Paaa + 3*SQR(n[A])*n[B]*Paab 
       + 3*n[A]*SQR(n[B])*Pabb + CUBE(n[B])*Pbbb
       + 3*w*(SQR(n[A])*Paa + 2*n[A]*n[B]*Pab + SQR(n[B])*Pbb)
       + w*w*(3*(n[A]*Pa + n[B]*Pb) + w*P1));

  Faab = k1 * Paab;
  Fbbc = -k2 * (n[A]*Pabb + n[B]*Pbbb + w*Pbb);
  Fcca = k3 * (SQR(n[A])*Paaa + 2*n[A]*n[B]*Paab + SQR(n[B])*Pabb
     + w*(2*(n[A]*Paa + n[B]*Pab) + w*Pa));
}

void CVolInt::compVolumeIntegrals(POLYHEDRON *p)
{
  FACE *f;
  double nx, ny, nz;
  int i;

  T0 = T1[0] = T1[1] = T1[2]
     = T2[0] = T2[1] = T2[2]
     = TP[0] = TP[1] = TP[2] = 0;

  for (i = 0; i < p->numFaces; i++) {

    f = &p->faces[i];

    nx = fabs(f->norm[0]);
    ny = fabs(f->norm[1]);
    nz = fabs(f->norm[2]);
    if (nx > ny && nx > nz) C = 0;
    else C = (ny > nz) ? 1 : 2;
    A = (C + 1) % 3;
    B = (A + 1) % 3;

    compFaceIntegrals(f);

    T0 += f->norm[0] * ((A == 0) ? Fa : ((B == 0) ? Fb : Fc));

    T1[A] += f->norm[A] * Faa;
    T1[B] += f->norm[B] * Fbb;
    T1[C] += f->norm[C] * Fcc;
    T2[A] += f->norm[A] * Faaa;
    T2[B] += f->norm[B] * Fbbb;
    T2[C] += f->norm[C] * Fccc;
    TP[A] += f->norm[A] * Faab;
    TP[B] += f->norm[B] * Fbbc;
    TP[C] += f->norm[C] * Fcca;
  }

  T1[0] /= 2; T1[1] /= 2; T1[2] /= 2;
  T2[0] /= 3; T2[1] /= 3; T2[2] /= 3;
  TP[0] /= 2; TP[1] /= 2; TP[2] /= 2;
}

float CVolInt::getMassCenterOfMassAndInertiaTensor(const float* vertices,int ptCnt,const int* indices,int triCnt,float density,C3Vector& centerOfMass,C3X3Matrix& inertiaTensor)
{ // returned inertia tensor is divided by the mass!
    POLYHEDRON p;
    for (int i=0;i<ptCnt*3;i++)
        p.verts.push_back((double)vertices[i]);
    p.numVerts=ptCnt;
    p.numFaces=triCnt;
    for (int i=0;i<triCnt;i++)
    {
        FACE f;
        f.poly=&p;
        f.vertsInd[0]=indices[3*i+0];
        f.vertsInd[1]=indices[3*i+1];
        f.vertsInd[2]=indices[3*i+2];

        // compute face normal and offset w from the 3 vertices
        double dx1 = p.verts[3*f.vertsInd[1]+0] - p.verts[3*f.vertsInd[0]+0];
        double dy1 = p.verts[3*f.vertsInd[1]+1] - p.verts[3*f.vertsInd[0]+1];
        double dz1 = p.verts[3*f.vertsInd[1]+2] - p.verts[3*f.vertsInd[0]+2];
        double dx2 = p.verts[3*f.vertsInd[2]+0] - p.verts[3*f.vertsInd[1]+0];
        double dy2 = p.verts[3*f.vertsInd[2]+1] - p.verts[3*f.vertsInd[1]+1];
        double dz2 = p.verts[3*f.vertsInd[2]+2] - p.verts[3*f.vertsInd[1]+2];
        double nx = dy1 * dz2 - dy2 * dz1;
        double ny = dz1 * dx2 - dz2 * dx1;
        double nz = dx1 * dy2 - dx2 * dy1;
        double len = sqrt(nx * nx + ny * ny + nz * nz);
        f.norm[0] = nx / len;
        f.norm[1] = ny / len;
        f.norm[2] = nz / len;
        f.w = - f.norm[0] * p.verts[3*f.vertsInd[0]+0]
               - f.norm[1] * p.verts[3*f.vertsInd[0]+1]
               - f.norm[2] * p.verts[3*f.vertsInd[0]+2];

        p.faces.push_back(f);
    }

    double J[3][3];         /* inertia tensor */


    compVolumeIntegrals(&p);

    double _density=(double)density;
    double mass=_density*T0;

    double r[3];
    r[0]=T1[0]/T0;
    r[1]=T1[1]/T0;
    r[2]=T1[2]/T0;
    centerOfMass.setData((float)r[0],(float)r[1],(float)r[2]);

    // compute inertia tensor
    J[0][0]=_density*(T2[1]+T2[2]);
    J[1][1]=_density*(T2[2]+T2[0]);
    J[2][2]=_density*(T2[0]+T2[1]);
    J[0][1]=J[1][0]=-_density*TP[0];
    J[1][2]=J[2][1]=-_density*TP[1];
    J[2][0]=J[0][2]=-_density*TP[2];

    // translate inertia tensor to center of mass
    J[0][0]-=mass*(r[1]*r[1]+r[2]*r[2]);
    J[1][1]-=mass*(r[2]*r[2]+r[0]*r[0]);
    J[2][2]-=mass*(r[0]*r[0]+r[1]*r[1]);
    J[0][1]=J[1][0]+=mass*r[0]*r[1];
    J[1][2]=J[2][1]+=mass*r[1]*r[2];
    J[2][0]=J[0][2]+=mass*r[2]*r[0];

    inertiaTensor.axis[0](0)=float(J[0][0]/mass);
    inertiaTensor.axis[1](0)=float(J[0][1]/mass);
    inertiaTensor.axis[2](0)=float(J[0][2]/mass);
    inertiaTensor.axis[0](1)=float(J[1][0]/mass);
    inertiaTensor.axis[1](1)=float(J[1][1]/mass);
    inertiaTensor.axis[2](1)=float(J[1][2]/mass);
    inertiaTensor.axis[0](2)=float(J[2][0]/mass);
    inertiaTensor.axis[1](2)=float(J[2][1]/mass);
    inertiaTensor.axis[2](2)=float(J[2][2]/mass);

/*
    printf("\nT1 =   %+20.6f\n\n", T0);

    printf("Tx =   %+20.6f\n", T1[0]);
    printf("Ty =   %+20.6f\n", T1[1]);
    printf("Tz =   %+20.6f\n\n", T1[2]);

    printf("Txx =  %+20.6f\n", T2[0]);
    printf("Tyy =  %+20.6f\n", T2[1]);
    printf("Tzz =  %+20.6f\n\n", T2[2]);

    printf("Txy =  %+20.6f\n", TP[0]);
    printf("Tyz =  %+20.6f\n", TP[1]);
    printf("Tzx =  %+20.6f\n\n", TP[2]);


    printf("inertia tensor with origin at c.o.m. :\n");
    printf("%+15.6f  %+15.6f  %+15.6f\n", J[0][0], J[0][1], J[0][2]);
    printf("%+15.6f  %+15.6f  %+15.6f\n", J[1][0], J[1][1], J[1][2]);
    printf("%+15.6f  %+15.6f  %+15.6f\n\n", J[2][0], J[2][1], J[2][2]);
//*/
    return((float)mass);
}
