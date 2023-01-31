#include <vector>
#include "algos.h"

C7Vector CAlgos::getMeshBoundingBoxPose(const std::vector<double>& vert,const std::vector<int>& ind,bool alignedWithMainAxis)
{
    C7Vector tr;
    tr.setIdentity();
    std::vector<double> vertices(vert);

    if (alignedWithMainAxis)
    {
        tr=getMainAxis(vertices,ind);
        C7Vector transfInverse(tr.getInverse());

        for (int i=0;i<vertices.size()/3;i++)
        {
            C3Vector tmp(vertices.data()+3*i);
            tmp=transfInverse*tmp;
            vertices[3*i+0]=tmp(0);
            vertices[3*i+1]=tmp(1);
            vertices[3*i+2]=tmp(2);
        }
    }

    // Compute the bounding box for these vertices
    C3Vector bbMin;
    C3Vector bbMax;
    for (size_t i=0;i<vertices.size()/3;i++)
    {
        C3Vector curr(vertices.data()+3*i);
        if (i==0)
        {
            bbMin=curr;
            bbMax=curr;
        }
        else
        {
            bbMin.keepMin(curr);
            bbMax.keepMax(curr);
        }
    }
    C7Vector translation;
    translation.setIdentity();
    translation.X=(bbMin+bbMax)*0.5;
    tr=tr*translation;
    return(tr);
}

C4X4Matrix CAlgos::getMainAxis(const std::vector<double>& vertices,const std::vector<int>& indices)
{ // Returned frame's axes are as follow: x: smalles axis and z: biggest axis.

    /* // Use following code to generate spherical orientation patterns:
    // ********************************************************************
    // *********************** KEEEEEEEEP!!!! *****************************
    // ********************************************************************
    static bool firstPass=true;
    if (firstPass)
    {
        firstPass=false;
        ogl::setColor(1.0,1.0,0.0,GL_EMISSION);
        C4Vector v;
        v.setIdentity();
        const int divNb[4]={1,6,13,19};//,25,31,38,44,50,57};
        for (int i=0;i<div;i++)
        {
            C4Vector savedV(v);
            C4Vector rot(piValT2/double(divNb[i]),C3Vector::unitZVector);
            for (int j=0;j<divNb[i];j++)
            {
                C4Vector vi(v.getInverse());
                printf("%ff,%ff,%ff,%ff,\n",vi(0),vi(1),vi(2),vi(3));
                C3Vector w(v*C3Vector::unitZVector);
                v=rot*v;
            }
            v=savedV;
            double aV=piValue*0.5/(double(div)-0.5);
            double aV2=aV*0.5/(double(div)-0.5);
            double aV3=aV2*0.5/(double(div)-0.5);
            double aV4=aV3*0.5/(double(div)-0.5);
            C4Vector rot2(aV,C3Vector::unitXVector); // use aV, aV2, aV3 or aV4 depending on the desired spacing
            v=rot2*v;
        }
    }
//*/

/* // Use following code to generate circular orientation patterns:
    // ********************************************************************
    // *********************** KEEEEEEEEP!!!! *****************************
    // ********************************************************************
    static bool firstPass=true;
    if (firstPass)
    {
        firstPass=false;
        ogl::setColor(1.0,1.0,0.0,GL_EMISSION);
        C4Vector v;
        v.setIdentity();
        double aV=piValue*0.5/double(div+1);
        double aV2=aV/double(div+1);
        double aV3=aV2/double(div+1);
        double aV4=aV3/double(div+1);
        double aV5=aV4/double(div+1);
        double aV6=aV5/double(div+1);
        double usedAv=aV;// use aV, aV2, aV3, aV4, aV5 or aV6 depending on the desired spacing
        C4Vector rot(usedAv,C3Vector::unitZVector);
        C4Vector rotNeg(-usedAv*double(div)*0.5,C3Vector::unitZVector);
        C4Vector or(rotNeg);
        for (int i=0;i<div+1;i++)
        {
            C4Vector vi(or.getInverse());
            printf("%ff,%ff,%ff,%ff,\n",vi(0),vi(1),vi(2),vi(3));
            C3Vector w(or*C3Vector::unitXVector);
            or=rot*or;
        }
    }
*/
/* // Use following code to display generated spherical orientation patterns:
    // ********************************************************************
    // *********************** KEEEEEEEEP!!!! *****************************
    // ********************************************************************
        ogl::setColor(1.0,1.0,0.0,GL_EMISSION);
        for (int i=0;i<39;i++)
        {
            C3Vector w(((C4Vector*)orientationDataPrecision0)[i].getInverse()*C3Vector::unitZVector);
            ogl::setColor(1.0,1.0,0.0,GL_DIFFUSE);

            C3Vector w2(((C4Vector*)orientationDataPrecision1)[i].getInverse()*C3Vector::unitZVector);
            ogl::setColor(0.0,1.0,1.0,GL_DIFFUSE);

            C3Vector w3(((C4Vector*)orientationDataPrecision2)[i].getInverse()*C3Vector::unitZVector);
            ogl::setColor(0.1,0.0,1.0,GL_DIFFUSE);

            C3Vector w4(((C4Vector*)orientationDataPrecision3)[i].getInverse()*C3Vector::unitZVector);
            ogl::setColor(0.1,0.0,1.0,GL_DIFFUSE);

        }
//*/

/* // Use following code to display generated circular orientation patterns:
    // ********************************************************************
    // *********************** KEEEEEEEEP!!!! *****************************
    // ********************************************************************
        ogl::setColor(1.0,1.0,0.0,GL_EMISSION);
        for (int i=0;i<5;i++)
        {
            C3Vector w(((C4Vector*)orientation2DataPrecision0)[i].getInverse()*C3Vector::unitXVector*2.0);
            ogl::setColor(1.0,1.0,0.0,GL_DIFFUSE);

            C3Vector w2(((C4Vector*)orientation2DataPrecision1)[i].getInverse()*C3Vector::unitXVector*1.5);
            ogl::setColor(0.0,1.0,1.0,GL_DIFFUSE);

            C3Vector w3(((C4Vector*)orientation2DataPrecision2)[i].getInverse()*C3Vector::unitXVector*1.25);
            ogl::setColor(0.1,0.0,1.0,GL_DIFFUSE);

            C3Vector w4(((C4Vector*)orientation2DataPrecision3)[i].getInverse()*C3Vector::unitXVector);
            ogl::setColor(0.1,0.0,1.0,GL_DIFFUSE);

            C3Vector w5(((C4Vector*)orientation2DataPrecision4)[i].getInverse()*C3Vector::unitXVector);
            ogl::setColor(0.1,0.0,1.0,GL_DIFFUSE);

        }
//*/

    std::vector<C3Vector> vert;
    for (size_t i=0;i<vertices.size()/3;i++)
        vert.push_back(C3Vector(vertices.data()+3*i+0));

    // Following are 4 sets of spherical orientation patterns. Patterns are same, scale is different.
    static const double orientationDataPrecision0[39*4]=
    { // 25.71 deg. precision
        1.000000,0.000000,0.000000,0.000000,
        0.974928,-0.222521,0.000000,0.000000,
        0.844312,-0.192709,-0.111260,-0.487464,
        0.487464,-0.111260,-0.192709,-0.844312,
        -0.000000,0.000000,-0.222521,-0.974928,
        -0.487464,0.111260,-0.192709,-0.844312,
        -0.844312,0.192709,-0.111260,-0.487464,
        0.900969,-0.433884,0.000000,0.000000,
        0.874788,-0.421276,-0.103835,-0.215616,
        0.797768,-0.384185,-0.201636,-0.418701,
        0.674385,-0.324767,-0.287718,-0.597453,
        0.511809,-0.246474,-0.357079,-0.741483,
        0.319488,-0.153857,-0.405688,-0.842421,
        0.108600,-0.052299,-0.430720,-0.894400,
        -0.108600,0.052299,-0.430720,-0.894400,
        -0.319488,0.153857,-0.405688,-0.842421,
        -0.511809,0.246474,-0.357079,-0.741483,
        -0.674385,0.324767,-0.287718,-0.597453,
        -0.797769,0.384185,-0.201636,-0.418701,
        -0.874789,0.421276,-0.103835,-0.215616,
        0.781831,-0.623490,0.000000,0.000000,
        0.771168,-0.614986,-0.102623,-0.128685,
        0.739470,-0.589707,-0.202447,-0.253860,
        0.687600,-0.548343,-0.296748,-0.372111,
        0.616975,-0.492021,-0.382955,-0.480211,
        0.529520,-0.422278,-0.458716,-0.575212,
        0.427621,-0.341017,-0.521965,-0.654523,
        0.314058,-0.250453,-0.570975,-0.715981,
        0.191928,-0.153058,-0.604411,-0.757908,
        0.064563,-0.051487,-0.621360,-0.779161,
        -0.064563,0.051487,-0.621360,-0.779161,
        -0.191928,0.153058,-0.604411,-0.757908,
        -0.314058,0.250453,-0.570975,-0.715981,
        -0.427621,0.341017,-0.521965,-0.654523,
        -0.529520,0.422278,-0.458717,-0.575212,
        -0.616975,0.492021,-0.382956,-0.480211,
        -0.687600,0.548343,-0.296748,-0.372111,
        -0.739470,0.589708,-0.202447,-0.253860,
        -0.771169,0.614986,-0.102623,-0.128685
    };

    static const double orientationDataPrecision1[39*4]=
    { // 3.67 deg. precision
        1.000000,0.000000,0.000000,0.000000,
        0.999486,-0.032052,0.000000,0.000000,
        0.865580,-0.027757,-0.016026,-0.499743,
        0.499743,-0.016026,-0.027757,-0.865580,
        -0.000000,0.000000,-0.032052,-0.999486,
        -0.499743,0.016026,-0.027757,-0.865580,
        -0.865580,0.027757,-0.016026,-0.499743,
        0.997945,-0.064070,0.000000,0.000000,
        0.968947,-0.062208,-0.015333,-0.238824,
        0.883637,-0.056731,-0.029775,-0.463768,
        0.746973,-0.047957,-0.042486,-0.661760,
        0.566898,-0.036396,-0.052729,-0.821293,
        0.353876,-0.022720,-0.059907,-0.933095,
        0.120289,-0.007723,-0.063603,-0.990669,
        -0.120289,0.007723,-0.063603,-0.990669,
        -0.353876,0.022720,-0.059907,-0.933095,
        -0.566898,0.036396,-0.052729,-0.821293,
        -0.746973,0.047957,-0.042486,-0.661760,
        -0.883637,0.056731,-0.029775,-0.463768,
        -0.968947,0.062208,-0.015333,-0.238824,
        0.995379,-0.096023,0.000000,0.000000,
        0.981803,-0.094713,-0.015805,-0.163834,
        0.941447,-0.090820,-0.031179,-0.323199,
        0.875410,-0.084450,-0.045702,-0.473748,
        0.785494,-0.075776,-0.058979,-0.611374,
        0.674152,-0.065035,-0.070646,-0.732324,
        0.544421,-0.052520,-0.080387,-0.833298,
        0.399839,-0.038572,-0.087935,-0.911542,
        0.244351,-0.023572,-0.093085,-0.964921,
        0.082198,-0.007930,-0.095695,-0.991980,
        -0.082198,0.007930,-0.095695,-0.991980,
        -0.244351,0.023572,-0.093085,-0.964921,
        -0.399839,0.038572,-0.087935,-0.911542,
        -0.544421,0.052520,-0.080387,-0.833298,
        -0.674152,0.065035,-0.070646,-0.732324,
        -0.785494,0.075776,-0.058979,-0.611375,
        -0.875410,0.084450,-0.045702,-0.473748,
        -0.941447,0.090820,-0.031179,-0.323199,
        -0.981804,0.094713,-0.015805,-0.163834
    };

    static const double orientationDataPrecision2[39*4]=
    { // 0.52 deg. precision
        1.000000,0.000000,0.000000,0.000000,
        0.999990,-0.004580,0.000000,0.000000,
        0.866016,-0.003966,-0.002290,-0.499995,
        0.499995,-0.002290,-0.003966,-0.866016,
        -0.000000,0.000000,-0.004580,-0.999990,
        -0.499995,0.002290,-0.003966,-0.866016,
        -0.866016,0.003966,-0.002290,-0.499995,
        0.999958,-0.009159,0.000000,0.000000,
        0.970901,-0.008893,-0.002192,-0.239306,
        0.885419,-0.008110,-0.004256,-0.464704,
        0.748479,-0.006856,-0.006074,-0.663095,
        0.568041,-0.005203,-0.007538,-0.822949,
        0.354590,-0.003248,-0.008564,-0.934977,
        0.120532,-0.001104,-0.009092,-0.992667,
        -0.120532,0.001104,-0.009092,-0.992667,
        -0.354590,0.003248,-0.008564,-0.934977,
        -0.568041,0.005203,-0.007538,-0.822949,
        -0.748480,0.006856,-0.006074,-0.663095,
        -0.885419,0.008110,-0.004256,-0.464704,
        -0.970901,0.008893,-0.002192,-0.239306,
        0.999906,-0.013738,0.000000,0.000000,
        0.986268,-0.013551,-0.002261,-0.164579,
        0.945728,-0.012994,-0.004461,-0.324669,
        0.879391,-0.012082,-0.006539,-0.475902,
        0.789066,-0.010841,-0.008438,-0.614155,
        0.677218,-0.009305,-0.010108,-0.735655,
        0.546897,-0.007514,-0.011501,-0.837088,
        0.401658,-0.005519,-0.012581,-0.915687,
        0.245462,-0.003373,-0.013318,-0.969309,
        0.082572,-0.001135,-0.013691,-0.996491,
        -0.082572,0.001135,-0.013691,-0.996491,
        -0.245462,0.003373,-0.013318,-0.969309,
        -0.401658,0.005519,-0.012581,-0.915687,
        -0.546897,0.007514,-0.011501,-0.837088,
        -0.677218,0.009305,-0.010108,-0.735655,
        -0.789066,0.010841,-0.008438,-0.614155,
        -0.879391,0.012082,-0.006539,-0.475903,
        -0.945728,0.012994,-0.004461,-0.324669,
        -0.986269,0.013551,-0.002261,-0.164579
    };

    static const double orientationDataPrecision3[39*4]=
    { // 0.074 deg. precision
        1.000000,0.000000,0.000000,0.000000,
        1.000000,-0.000654,0.000000,0.000000,
        0.866025,-0.000567,-0.000327,-0.500000,
        0.500000,-0.000327,-0.000567,-0.866025,
        -0.000000,0.000000,-0.000654,-1.000000,
        -0.500000,0.000327,-0.000567,-0.866025,
        -0.866025,0.000567,-0.000327,-0.500000,
        0.999999,-0.001308,0.000000,0.000000,
        0.970941,-0.001270,-0.000313,-0.239315,
        0.885455,-0.001159,-0.000608,-0.464723,
        0.748510,-0.000979,-0.000868,-0.663122,
        0.568064,-0.000743,-0.001077,-0.822983,
        0.354605,-0.000464,-0.001223,-0.935015,
        0.120537,-0.000158,-0.001299,-0.992708,
        -0.120537,0.000158,-0.001299,-0.992708,
        -0.354605,0.000464,-0.001223,-0.935015,
        -0.568064,0.000743,-0.001077,-0.822983,
        -0.748510,0.000979,-0.000868,-0.663122,
        -0.885455,0.001159,-0.000608,-0.464723,
        -0.970941,0.001270,-0.000313,-0.239315,
        0.999998,-0.001963,0.000000,0.000000,
        0.986359,-0.001936,-0.000323,-0.164594,
        0.945815,-0.001856,-0.000637,-0.324699,
        0.879472,-0.001726,-0.000934,-0.475946,
        0.789139,-0.001549,-0.001206,-0.614211,
        0.677280,-0.001329,-0.001444,-0.735722,
        0.546947,-0.001073,-0.001643,-0.837165,
        0.401695,-0.000788,-0.001797,-0.915772,
        0.245485,-0.000482,-0.001903,-0.969398,
        0.082579,-0.000162,-0.001956,-0.996583,
        -0.082579,0.000162,-0.001956,-0.996583,
        -0.245485,0.000482,-0.001903,-0.969398,
        -0.401695,0.000788,-0.001797,-0.915772,
        -0.546947,0.001073,-0.001643,-0.837165,
        -0.677280,0.001329,-0.001444,-0.735723,
        -0.789139,0.001549,-0.001206,-0.614212,
        -0.879472,0.001726,-0.000934,-0.475947,
        -0.945816,0.001856,-0.000637,-0.324699,
        -0.986360,0.001936,-0.000323,-0.164594
    };

    // Following are 5 sets of circular orientation patterns. Patterns are same, scale is different.
    static const double orientation2DataPrecision0[5*4]=
    { // 18 deg. precision
        0.951057,0.000000,0.000000,0.309017,
        0.987688,0.000000,0.000000,0.156434,
        1.000000,0.000000,0.000000,-0.000000,
        0.987688,0.000000,0.000000,-0.156434,
        0.951057,0.000000,0.000000,-0.309017
    };
    static const double orientation2DataPrecision1[5*4]=
    { // 3.6 deg. precision
        0.998027,0.000000,0.000000,0.062791,
        0.999507,0.000000,0.000000,0.031411,
        1.000000,0.000000,0.000000,-0.000000,
        0.999506,0.000000,0.000000,-0.031411,
        0.998027,0.000000,0.000000,-0.062791
    };
    static const double orientation2DataPrecision2[5*4]=
    { // 0.72 deg. precision
        0.999921,0.000000,0.000000,0.012566,
        0.999980,0.000000,0.000000,0.006283,
        1.000000,0.000000,0.000000,0.000000,
        0.999980,0.000000,0.000000,-0.006283,
        0.999921,0.000000,0.000000,-0.012566
    };
    static const double orientation2DataPrecision3[5*4]=
    { // 0.144 deg. precision
        0.999997,0.000000,0.000000,0.002513,
        0.999999,0.000000,0.000000,0.001257,
        1.000000,0.000000,0.000000,0.000000,
        0.999999,0.000000,0.000000,-0.001257,
        0.999997,0.000000,0.000000,-0.002513
    };
    static const double orientation2DataPrecision4[5*4]=
    { // 0.0288 deg. precision
        1.000000,0.000000,0.000000,0.000503,
        1.000000,0.000000,0.000000,0.000251,
        1.000000,0.000000,0.000000,0.000000,
        1.000000,0.000000,0.000000,-0.000251,
        1.000000,0.000000,0.000000,-0.000503
    };

    // First pass (roughest) (spherical):
    C4Vector bestSmallestDirection;
    double smallestDimension=999999999.0;
    for (int i=0;i<39;i++)
    {
        double minMax[2]={+999999999.0,-999999999.0};
        for (size_t j=0;j<vert.size();j++)
        {
            C3Vector w(((C4Vector*)orientationDataPrecision0)[i]*vert[j]);
            if (w(2)<minMax[0])
                minMax[0]=w(2);
            if (w(2)>minMax[1])
                minMax[1]=w(2);
            if (minMax[1]-minMax[0]>smallestDimension)
                break;
        }
        double currSmallest=minMax[1]-minMax[0];
        if (currSmallest<smallestDimension)
        {
            smallestDimension=currSmallest;
            bestSmallestDirection=((C4Vector*)orientationDataPrecision0)[i].getInverse();
        }
    }

    // Second pass (spherical):
    C4Vector bestSmallestDirectionPrevious(bestSmallestDirection);
    smallestDimension=999999999.0;
    for (int i=0;i<39;i++)
    {
        double minMax[2]={+999999999.0,-999999999.0};
        C4Vector currentOrientation((bestSmallestDirectionPrevious*((C4Vector*)orientationDataPrecision1)[i].getInverse()).getInverse());
        for (size_t j=0;j<vert.size();j++)
        {
            C3Vector w(currentOrientation*vert[j]);
            if (w(2)<minMax[0])
                minMax[0]=w(2);
            if (w(2)>minMax[1])
                minMax[1]=w(2);
            if (minMax[1]-minMax[0]>smallestDimension)
                break;
        }
        double currSmallest=minMax[1]-minMax[0];
        if (currSmallest<smallestDimension)
        {
            smallestDimension=currSmallest;
            bestSmallestDirection=currentOrientation.getInverse();
        }
    }

    // Third pass (spherical):
    bestSmallestDirectionPrevious=bestSmallestDirection;
    smallestDimension=999999999.0;
    for (int i=0;i<39;i++)
    {
        double minMax[2]={+999999999.0,-999999999.0};
        C4Vector currentOrientation((bestSmallestDirectionPrevious*((C4Vector*)orientationDataPrecision2)[i].getInverse()).getInverse());
        for (size_t j=0;j<vert.size();j++)
        {
            C3Vector w(currentOrientation*vert[j]);
            if (w(2)<minMax[0])
                minMax[0]=w(2);
            if (w(2)>minMax[1])
                minMax[1]=w(2);
            if (minMax[1]-minMax[0]>smallestDimension)
                break;
        }
        double currSmallest=minMax[1]-minMax[0];
        if (currSmallest<smallestDimension)
        {
            smallestDimension=currSmallest;
            bestSmallestDirection=currentOrientation.getInverse();
        }
    }

    // Forth and last pass (most precise one) (spherical):
    bestSmallestDirectionPrevious=bestSmallestDirection;
    smallestDimension=999999999.0;
    for (int i=0;i<39;i++)
    {
        double minMax[2]={+999999999.0,-999999999.0};
        C4Vector currentOrientation((bestSmallestDirectionPrevious*((C4Vector*)orientationDataPrecision3)[i].getInverse()).getInverse());
        for (size_t j=0;j<vert.size();j++)
        {
            C3Vector w(currentOrientation*vert[j]);
            if (w(2)<minMax[0])
                minMax[0]=w(2);
            if (w(2)>minMax[1])
                minMax[1]=w(2);
            if (minMax[1]-minMax[0]>smallestDimension)
                break;
        }
        double currSmallest=minMax[1]-minMax[0];
        if (currSmallest<smallestDimension)
        {
            smallestDimension=currSmallest;
            bestSmallestDirection=currentOrientation.getInverse();
        }
    }

    // We have found the axis with the smallest dimension (z of bestSmallestDirection), we now search for the two other axes:
    bool xAxisIsLargerThanYAxis=false;

    // First and roughest pass (circular):
    bestSmallestDirectionPrevious=bestSmallestDirection;
    double smallestArea=999999999999999999.0;
    for (int i=0;i<5;i++)
    {
        double minMaxX[2]={+999999999.0,-999999999.0};
        double minMaxY[2]={+999999999.0,-999999999.0};
        C4Vector currentOrientation((bestSmallestDirectionPrevious*((C4Vector*)orientation2DataPrecision0)[i].getInverse()).getInverse());
        for (size_t j=0;j<vert.size();j++)
        {
            C3Vector w(currentOrientation*vert[j]);
            if (w(0)<minMaxX[0])
                minMaxX[0]=w(0);
            if (w(0)>minMaxX[1])
                minMaxX[1]=w(0);
            if (w(1)<minMaxY[0])
                minMaxY[0]=w(1);
            if (w(1)>minMaxY[1])
                minMaxY[1]=w(1);
        }
        double currSmallest=(minMaxX[1]-minMaxX[0])*(minMaxY[1]-minMaxY[0]);
        xAxisIsLargerThanYAxis=(minMaxX[1]-minMaxX[0]>minMaxY[1]-minMaxY[0]);
        if (currSmallest<smallestArea)
        {
            smallestArea=currSmallest;
            bestSmallestDirection=currentOrientation.getInverse();
        }
    }

    // Second pass (circular):
    bestSmallestDirectionPrevious=bestSmallestDirection;
    smallestArea=999999999999999999.0;
    for (int i=0;i<5;i++)
    {
        double minMaxX[2]={+999999999.0,-999999999.0};
        double minMaxY[2]={+999999999.0,-999999999.0};
        C4Vector currentOrientation((bestSmallestDirectionPrevious*((C4Vector*)orientation2DataPrecision1)[i].getInverse()).getInverse());
        for (size_t j=0;j<vert.size();j++)
        {
            C3Vector w(currentOrientation*vert[j]);
            if (w(0)<minMaxX[0])
                minMaxX[0]=w(0);
            if (w(0)>minMaxX[1])
                minMaxX[1]=w(0);
            if (w(1)<minMaxY[0])
                minMaxY[0]=w(1);
            if (w(1)>minMaxY[1])
                minMaxY[1]=w(1);
        }
        double currSmallest=(minMaxX[1]-minMaxX[0])*(minMaxY[1]-minMaxY[0]);
        xAxisIsLargerThanYAxis=(minMaxX[1]-minMaxX[0]>minMaxY[1]-minMaxY[0]);
        if (currSmallest<smallestArea)
        {
            smallestArea=currSmallest;
            bestSmallestDirection=currentOrientation.getInverse();
        }
    }

    // Third pass (circular):
    bestSmallestDirectionPrevious=bestSmallestDirection;
    smallestArea=999999999999999999.0;
    for (int i=0;i<5;i++)
    {
        double minMaxX[2]={+999999999.0,-999999999.0};
        double minMaxY[2]={+999999999.0,-999999999.0};
        C4Vector currentOrientation((bestSmallestDirectionPrevious*((C4Vector*)orientation2DataPrecision2)[i].getInverse()).getInverse());
        for (size_t j=0;j<vert.size();j++)
        {
            C3Vector w(currentOrientation*vert[j]);
            if (w(0)<minMaxX[0])
                minMaxX[0]=w(0);
            if (w(0)>minMaxX[1])
                minMaxX[1]=w(0);
            if (w(1)<minMaxY[0])
                minMaxY[0]=w(1);
            if (w(1)>minMaxY[1])
                minMaxY[1]=w(1);
        }
        double currSmallest=(minMaxX[1]-minMaxX[0])*(minMaxY[1]-minMaxY[0]);
        xAxisIsLargerThanYAxis=(minMaxX[1]-minMaxX[0]>minMaxY[1]-minMaxY[0]);
        if (currSmallest<smallestArea)
        {
            smallestArea=currSmallest;
            bestSmallestDirection=currentOrientation.getInverse();
        }
    }

    // Forth pass (circular):
    bestSmallestDirectionPrevious=bestSmallestDirection;
    smallestArea=999999999999999999.0;
    for (int i=0;i<5;i++)
    {
        double minMaxX[2]={+999999999.0,-999999999.0};
        double minMaxY[2]={+999999999.0,-999999999.0};
        C4Vector currentOrientation((bestSmallestDirectionPrevious*((C4Vector*)orientation2DataPrecision3)[i].getInverse()).getInverse());
        for (size_t j=0;j<vert.size();j++)
        {
            C3Vector w(currentOrientation*vert[j]);
            if (w(0)<minMaxX[0])
                minMaxX[0]=w(0);
            if (w(0)>minMaxX[1])
                minMaxX[1]=w(0);
            if (w(1)<minMaxY[0])
                minMaxY[0]=w(1);
            if (w(1)>minMaxY[1])
                minMaxY[1]=w(1);
        }
        double currSmallest=(minMaxX[1]-minMaxX[0])*(minMaxY[1]-minMaxY[0]);
        xAxisIsLargerThanYAxis=(minMaxX[1]-minMaxX[0]>minMaxY[1]-minMaxY[0]);
        if (currSmallest<smallestArea)
        {
            smallestArea=currSmallest;
            bestSmallestDirection=currentOrientation.getInverse();
        }
    }

    // Fifth and last pass (circular):
    bestSmallestDirectionPrevious=bestSmallestDirection;
    smallestArea=999999999999999999.0;
    for (int i=0;i<5;i++)
    {
        double minMaxX[2]={+999999999.0,-999999999.0};
        double minMaxY[2]={+999999999.0,-999999999.0};
        C4Vector currentOrientation((bestSmallestDirectionPrevious*((C4Vector*)orientation2DataPrecision4)[i].getInverse()).getInverse());
        for (size_t j=0;j<vert.size();j++)
        {
            C3Vector w(currentOrientation*vert[j]);
            if (w(0)<minMaxX[0])
                minMaxX[0]=w(0);
            if (w(0)>minMaxX[1])
                minMaxX[1]=w(0);
            if (w(1)<minMaxY[0])
                minMaxY[0]=w(1);
            if (w(1)>minMaxY[1])
                minMaxY[1]=w(1);
        }
        double currSmallest=(minMaxX[1]-minMaxX[0])*(minMaxY[1]-minMaxY[0]);
        xAxisIsLargerThanYAxis=(minMaxX[1]-minMaxX[0]>minMaxY[1]-minMaxY[0]);
        if (currSmallest<smallestArea)
        {
            smallestArea=currSmallest;
            bestSmallestDirection=currentOrientation.getInverse();
        }
    }

    C3Vector alternateCenter;
    double alternateSize=FLOAT_MAX;
    C4Vector bestSmallestDirectionAlternative;

    C3X3Matrix alternativeFrame(bestSmallestDirection);
    std::vector<int> tris;
    for (size_t i=0;i<indices.size()/3;i++)
        tris.push_back(i);
    // Now search for the triangle with largest surface:
    double ls=0.0;
    C3Vector ltn;
    for (size_t i=0;i<tris.size();i++)
    {
        const int in[3]={indices[3*tris[i]+0],indices[3*tris[i]+1],indices[3*tris[i]+2]};
        C3Vector a0(vertices.data()+3*in[0]);
        C3Vector a1(vertices.data()+3*in[1]);
        C3Vector a2(vertices.data()+3*in[2]);
        C3Vector e0(a2-a0);
        C3Vector e1(a1-a0);
        C3Vector n(e0^e1);
        double s=n.getLength();
        if (s>ls)
        {
            ls=s;
            ltn=n/s;
        }
    }
    // We now adjust the alternative frame with the ltn:
    C3Vector mostSimilar;
    double scalarResult=0;
    for (int i=0;i<3;i++)
    {
        double l=fabs(alternativeFrame.axis[i]*ltn);
        if (l>scalarResult)
        {
            scalarResult=l;
            mostSimilar=alternativeFrame.axis[i];
        }
    }
    if (mostSimilar*ltn<0.0)
        ltn*=-1.0; // take care of direction
    if (mostSimilar*ltn!=1.0)
    { // make sure they are not already colinear!
        C4Vector q(mostSimilar,ltn); // We build the transformation from mostSimilar to ltn
        alternativeFrame=(q*alternativeFrame.getQuaternion()).getMatrix();
    }

    // We now search for the triangle with larget surface perpendicular to the other one:
    ls=0.0;
    C3Vector ltnp;
    for (size_t i=0;i<tris.size();i++)
    {
        int in[3]={indices[3*tris[i]+0],indices[3*tris[i]+1],indices[3*tris[i]+2]};
        C3Vector a0(vertices.data()+3*in[0]);
        C3Vector a1(vertices.data()+3*in[1]);
        C3Vector a2(vertices.data()+3*in[2]);
        C3Vector e0(a2-a0);
        C3Vector e1(a1-a0);
        C3Vector n(e0^e1);
        double s=n.getLength();
        if (s>ls)
        {
            n/=s;
            if (fabs(n.getAngle(ltn)-1.57079633)<1.0*degToRad)
            {
                ltnp=n;
                ls=s;
            }
        }
    }
    if (ls!=0.0)
    { // ok we found a perpendicular triangle
        // We now adjust the alternative frame with the ltnp:
        C3Vector mostSimilar;
        double scalarResult=0;
        for (int i=0;i<3;i++)
        {
            double l=fabs(alternativeFrame.axis[i]*ltnp);
            if (l>scalarResult)
            {
                scalarResult=l;
                mostSimilar=alternativeFrame.axis[i];
            }
        }
        if (mostSimilar*ltnp<0.0)
            ltnp*=-1.0; // take care of direction
        if (mostSimilar*ltnp!=1.0)
        { // make sure they are not already colinear!
            // now project mostSimilar and ltnp into the plane defined by ltn:
            mostSimilar-=ltn*(mostSimilar*ltn);
            ltnp-=ltn*(ltnp*ltn);
            mostSimilar.normalize();
            ltnp.normalize();
            if (mostSimilar*ltnp!=1.0)
            { // make sure they are not already colinear again!
                C4Vector q(mostSimilar,ltnp); // We build the transformation from mostSimilar to ltnp
                alternativeFrame=(q*alternativeFrame.getQuaternion()).getMatrix();
            }
        }
    }
    bestSmallestDirectionAlternative=alternativeFrame.getQuaternion();
    // We now check the size of the alternate bounding box:
    C3Vector minV(+DBL_MAX,+DBL_MAX,+DBL_MAX);
    C3Vector maxV(-DBL_MAX,-DBL_MAX,-DBL_MAX);
    for (size_t j=0;j<vert.size();j++)
    {
        C3Vector w(bestSmallestDirectionAlternative.getInverse()*vert[j]);
        minV.keepMin(w);
        maxV.keepMax(w);
    }
    alternateCenter=(maxV+minV)*0.5;
    C3Vector s(maxV-minV);
    alternateSize=s(0)*s(1)*s(2);
    // Now order the new frame like: x=smallest size, z=biggest size:
    C3X3Matrix m(bestSmallestDirectionAlternative);
    C3Vector biggest;
    C3Vector smallest;
    double smallestS=FLOAT_MAX;
    double biggestS=0.0;
    for (int i=0;i<3;i++)
    {
        double l=s(i);
        if (l>=biggestS)
        {
            biggestS=l;
            biggest=m.axis[i];
        }
        if (l<=smallestS)
        {
            smallestS=l;
            smallest=m.axis[i];
        }
    }
    m.axis[2]=biggest;
    m.axis[0]=smallest;
    m.axis[1]=(biggest^smallest).getNormalized();
    bestSmallestDirectionAlternative=m.getQuaternion();

    // We search the center:
    minV(0)=+DBL_MAX; minV(1)=+DBL_MAX; minV(2)=+DBL_MAX;
    maxV(0)=-DBL_MAX; maxV(1)=-DBL_MAX; maxV(2)=-DBL_MAX;
    for (size_t j=0;j<vert.size();j++)
    {
        C3Vector w(bestSmallestDirection.getInverse()*vert[j]);
        minV.keepMin(w);
        maxV.keepMax(w);
    }

    C3Vector center=(maxV+minV)*0.5;
    s=maxV-minV;
    double size=s(0)*s(1)*s(2);

    bool rearrange=true;
    if (size>alternateSize)
    { // the alternative frame is better!
        rearrange=false;
        size=alternateSize;
        center=alternateCenter;
        bestSmallestDirection=bestSmallestDirectionAlternative;
    }

    // We reorder axes:
    C4X4Matrix c;
    C3X3Matrix tmp(bestSmallestDirection);

    if (rearrange) 
    {
        if (!xAxisIsLargerThanYAxis)
            tmp.axis[1]=tmp.axis[0];
        tmp.axis[0]=tmp.axis[2];
        tmp.axis[2]=tmp.axis[0]^tmp.axis[1];
    }
    c.M=tmp;
    c.X=center;

    return(c);
}

bool CAlgos::isBoxOutsideVolumeApprox(const C4X4Matrix& tr,const C3Vector& s,const std::vector<double>* planes)
{   // Planes contain a collection of plane definitions:
    // Each plane is defined by 4 values a, b, c & d (consecutive in the array):
    // ax+by+cz+d=0
    // The normal vector for each plane (a,b,c) should point outside of the volume
    // The volume has to be closed and convex
    // boxTransf is the transformation matrix of the box
    // boxSize is the size of the box (in fact half-sizes)
    // Return value is true if the box is definetely outside of the volume.
    // If return value is false, the box might also be outside of the volume!
    C3Vector edges[8];
    C3Vector v0(tr.M.axis[0]*s(0));
    C3Vector v1(tr.M.axis[1]*s(1));
    C3Vector v2(tr.M.axis[2]*s(2));
    edges[0]=v0+v1+v2+tr.X;
    edges[1]=v0+v1-v2+tr.X;
    edges[2]=v0-v1+v2+tr.X;
    edges[3]=v0-v1-v2+tr.X;
    edges[4]=v1+v2+tr.X-v0;
    edges[5]=v1-v2+tr.X-v0;
    edges[6]=v2+tr.X-v0-v1;
    edges[7]=tr.X-v0-v1-v2;
    for (int i=0;i<int(planes->size())/4;i++)
    {
        C3Vector abc(&(*planes)[4*i+0]);
        double d=(*planes)[4*i+3];
        if ((abc*edges[0]+d)>=0.0)
        {
            if ((abc*edges[1]+d)>=0.0)
            {
                if ((abc*edges[2]+d)>=0.0)
                {
                    if ((abc*edges[3]+d)>=0.0)
                    {
                        if ((abc*edges[4]+d)>=0.0)
                        {
                            if ((abc*edges[5]+d)>=0.0)
                            {
                                if ((abc*edges[6]+d)>=0.0)
                                {
                                    if ((abc*edges[7]+d)>=0.0)
                                        return(true);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return(false);
}
