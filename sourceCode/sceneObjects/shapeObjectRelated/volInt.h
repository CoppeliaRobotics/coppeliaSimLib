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

// Code adapted by Coppelia Robotics AG

#pragma once

#include <vector>
#include "3Vector.h"
#include "3X3Matrix.h"

#define SQR(x) ((x)*(x))
#define CUBE(x) ((x)*(x)*(x))

typedef struct {
  double norm[3];
  double w;
  int vertsInd[3];
  struct polyhedron *poly;
} FACE;

typedef struct polyhedron {
  int numVerts, numFaces;
  std::vector<double> verts;
  std::vector<FACE> faces;
} POLYHEDRON;


//FULLY STATIC CLASS
class CVolInt
{
public:
    static double getMassCenterOfMassAndInertiaTensor(const double* vertices,int ptCnt,const int* indices,int triCnt,double density,C3Vector& centerOfMass,C3X3Matrix& inertiaTensor);

private:
    static void compProjectionIntegrals(FACE *f);
    static void compFaceIntegrals(FACE *f);
    static void compVolumeIntegrals(POLYHEDRON *p);

    static int A;   // alpha
    static int B;   // beta
    static int C;   // gamma

    // projection integrals
    static double P1, Pa, Pb, Paa, Pab, Pbb, Paaa, Paab, Pabb, Pbbb;

    // face integrals
    static double Fa, Fb, Fc, Faa, Fbb, Fcc, Faaa, Fbbb, Fccc, Faab, Fbbc, Fcca;

    // volume integrals
    static double T0, T1[3], T2[3], TP[3];
};
