
#include "vrepMainHeader.h"
#include "algos.h"

C7Vector CAlgos::alignAndCenterGeometryAndGetTransformation(float* vert,int vertLength,int* ind,int indLength,float* norm,int normLength,bool alignWithGeomMainAxis)
{
    C7Vector tr;
    tr.setIdentity();
    if (alignWithGeomMainAxis)
    {
        tr=getMainAxis(vert,vertLength,ind,indLength,nullptr,0,true,true);
        C7Vector transfInverse(tr.getInverse());

        for (int i=0;i<vertLength/3;i++)
        {
            C3Vector tmp(vert+3*i);
            tmp=transfInverse*tmp;
            vert[3*i+0]=tmp(0);
            vert[3*i+1]=tmp(1);
            vert[3*i+2]=tmp(2);
        }
        if (norm!=nullptr)
        {
            for (int i=0;i<normLength/3;i++)
            {
                C3Vector tmp(norm+3*i);
                tmp=transfInverse.Q*tmp; // No translation!
                norm[3*i+0]=tmp(0);
                norm[3*i+1]=tmp(1);
                norm[3*i+2]=tmp(2);
            }
        }
    }

    // Compute the bounding box for these vertices
    C3Vector bbMin;
    C3Vector bbMax;
    for (int i=0;i<vertLength/3;i++)
    {
        C3Vector curr(vert+3*i);
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
    C3Vector mid((bbMin+bbMax)*0.5f);
    for (int i=0;i<vertLength/3;i++)
    {
        vert[3*i+0]-=mid(0);
        vert[3*i+1]-=mid(1);
        vert[3*i+2]-=mid(2);
    }
    C7Vector translation;
    translation.setIdentity();
    translation.X=mid;
    tr=tr*translation;
    return(tr);
}

C4X4Matrix CAlgos::getMainAxis(const std::vector<float>* vertices,const std::vector<int>* triangles,const std::vector<int>* trianglesIndices,bool useAllVerticesForce,bool veryPreciseWithTriangles)
{   // Triangles can be nullptr, in that case all vertices are used. trianglesIndices can be nullptr, in that case triangles are used!
    // if useAllVerticesForce is true, then all vertices are used anyway (forced)
    // if veryPreciseWithTriangles is true, then a more precise orientation is calculated using "triangles" (i.e. largest triangle could be one face of the bounding box)
    if (vertices->size()==0)
    {
        C4X4Matrix m;
        m.setIdentity();
        return(m);
    }
    if (triangles!=nullptr)
    {
        if (triangles->size()==0)
        {
            C4X4Matrix m;
            m.setIdentity();
            return(m);
        }
        if (trianglesIndices!=nullptr)
        {
            if (trianglesIndices->size()==0)
            {
                C4X4Matrix m;
                m.setIdentity();
                return(m);
            }
            return(getMainAxis(&(*vertices)[0],(int)vertices->size(),&(*triangles)[0],(int)triangles->size(),&(*trianglesIndices)[0],(int)trianglesIndices->size(),useAllVerticesForce,veryPreciseWithTriangles));
        }
        else
        {
            return(getMainAxis(&(*vertices)[0],(int)vertices->size(),&(*triangles)[0],(int)triangles->size(),nullptr,0,useAllVerticesForce,veryPreciseWithTriangles));
        }
    }
    else
        return(getMainAxis(&(*vertices)[0],(int)vertices->size(),nullptr,0,nullptr,0,useAllVerticesForce,veryPreciseWithTriangles)); // all vertices are used
}

C4X4Matrix CAlgos::getMainAxis(const float* vertices,int verticesLength,const int* indices,int indicesLength,const int* triIndices,int triIndicesLength,bool useAllVerticesForce,bool veryPreciseWithTriangles)
{   // Only referenced vertices are taken into account (or all if indices is nullptr)
    // if indices are null, then all vertices are used
    // if indices are not null and triIndices are null, then vertices referenced by indices are used
    // if triIndices are not null, then only referenced triangles are used
    // if useAllVerticesForce is true, then all vertices are used (forcing)
    // if veryPreciseWithTriangles is true, then a more precise orientation is calculated using "indices" (i.e. largest triangle could be one face of the bounding box)
    // Returned frame's axes are as follow: x: smalles axis and z: biggest axis.

    /* // Use following code to generate spherical orientation patterns:
    // ********************************************************************
    // *********************** KEEEEEEEEP!!!! *****************************
    // ********************************************************************
    static bool firstPass=true;
    if (firstPass)
    {
        firstPass=false;
        ogl::setColor(1.0f,1.0f,0.0f,GL_EMISSION);
        C4Vector v;
        v.setIdentity();
        const int divNb[4]={1,6,13,19};//,25,31,38,44,50,57};
        for (int i=0;i<div;i++)
        {
            C4Vector savedV(v);
            C4Vector rot(piValTimes2_f/float(divNb[i]),C3Vector::unitZVector);
            for (int j=0;j<divNb[i];j++)
            {
                C4Vector vi(v.getInverse());
                printf("%ff,%ff,%ff,%ff,\n",vi(0),vi(1),vi(2),vi(3));
                C3Vector w(v*C3Vector::unitZVector);
                v=rot*v;
            }
            v=savedV;
            float aV=piValue_f*0.5f/(float(div)-0.5f);
            float aV2=aV*0.5f/(float(div)-0.5f);
            float aV3=aV2*0.5f/(float(div)-0.5f);
            float aV4=aV3*0.5f/(float(div)-0.5f);
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
        ogl::setColor(1.0f,1.0f,0.0f,GL_EMISSION);
        C4Vector v;
        v.setIdentity();
        float aV=piValue_f*0.5f/float(div+1);
        float aV2=aV/float(div+1);
        float aV3=aV2/float(div+1);
        float aV4=aV3/float(div+1);
        float aV5=aV4/float(div+1);
        float aV6=aV5/float(div+1);
        float usedAv=aV;// use aV, aV2, aV3, aV4, aV5 or aV6 depending on the desired spacing
        C4Vector rot(usedAv,C3Vector::unitZVector);
        C4Vector rotNeg(-usedAv*float(div)*0.5f,C3Vector::unitZVector);
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
        ogl::setColor(1.0f,1.0f,0.0f,GL_EMISSION);
        for (int i=0;i<39;i++)
        {
            C3Vector w(((C4Vector*)orientationDataPrecision0)[i].getInverse()*C3Vector::unitZVector);
            ogl::setColor(1.0f,1.0f,0.0f,GL_DIFFUSE);

            C3Vector w2(((C4Vector*)orientationDataPrecision1)[i].getInverse()*C3Vector::unitZVector);
            ogl::setColor(0.0f,1.0f,1.0f,GL_DIFFUSE);

            C3Vector w3(((C4Vector*)orientationDataPrecision2)[i].getInverse()*C3Vector::unitZVector);
            ogl::setColor(0.1f,0.0f,1.0f,GL_DIFFUSE);

            C3Vector w4(((C4Vector*)orientationDataPrecision3)[i].getInverse()*C3Vector::unitZVector);
            ogl::setColor(0.1f,0.0f,1.0f,GL_DIFFUSE);

        }
//*/

/* // Use following code to display generated circular orientation patterns:
    // ********************************************************************
    // *********************** KEEEEEEEEP!!!! *****************************
    // ********************************************************************
        ogl::setColor(1.0f,1.0f,0.0f,GL_EMISSION);
        for (int i=0;i<5;i++)
        {
            C3Vector w(((C4Vector*)orientation2DataPrecision0)[i].getInverse()*C3Vector::unitXVector*2.0f);
            ogl::setColor(1.0f,1.0f,0.0f,GL_DIFFUSE);

            C3Vector w2(((C4Vector*)orientation2DataPrecision1)[i].getInverse()*C3Vector::unitXVector*1.5f);
            ogl::setColor(0.0f,1.0f,1.0f,GL_DIFFUSE);

            C3Vector w3(((C4Vector*)orientation2DataPrecision2)[i].getInverse()*C3Vector::unitXVector*1.25f);
            ogl::setColor(0.1f,0.0f,1.0f,GL_DIFFUSE);

            C3Vector w4(((C4Vector*)orientation2DataPrecision3)[i].getInverse()*C3Vector::unitXVector);
            ogl::setColor(0.1f,0.0f,1.0f,GL_DIFFUSE);

            C3Vector w5(((C4Vector*)orientation2DataPrecision4)[i].getInverse()*C3Vector::unitXVector);
            ogl::setColor(0.1f,0.0f,1.0f,GL_DIFFUSE);

        }
//*/

    // Prepare only vertices that are referenced:
    std::vector<C3Vector> vert;
    if ((indices==nullptr)||useAllVerticesForce)
    { // we use all vertices!
        for (int i=0;i<verticesLength/3;i++)
            vert.push_back(C3Vector(vertices+3*i+0));
    }
    else
    {
        std::vector<unsigned char> addedVertices(verticesLength/3,0);
        if (triIndices==nullptr)
        {
            for (int i=0;i<indicesLength;i++)
                    addedVertices[indices[i]]=1;
        }
        else
        {
            for (int i=0;i<triIndicesLength;i++)
            {
                addedVertices[indices[3*triIndices[i]+0]]=1;
                addedVertices[indices[3*triIndices[i]+1]]=1;
                addedVertices[indices[3*triIndices[i]+2]]=1;
            }
        }
        for (int i=0;i<int(addedVertices.size());i++)
        {
            if (addedVertices[i]!=0)
                vert.push_back(C3Vector(vertices+3*i+0));
        }
    }

    // Following are 4 sets of spherical orientation patterns. Patterns are same, scale is different.
    static const float orientationDataPrecision0[39*4]=
    { // 25.71 deg. precision
        1.000000f,0.000000f,0.000000f,0.000000f,
        0.974928f,-0.222521f,0.000000f,0.000000f,
        0.844312f,-0.192709f,-0.111260f,-0.487464f,
        0.487464f,-0.111260f,-0.192709f,-0.844312f,
        -0.000000f,0.000000f,-0.222521f,-0.974928f,
        -0.487464f,0.111260f,-0.192709f,-0.844312f,
        -0.844312f,0.192709f,-0.111260f,-0.487464f,
        0.900969f,-0.433884f,0.000000f,0.000000f,
        0.874788f,-0.421276f,-0.103835f,-0.215616f,
        0.797768f,-0.384185f,-0.201636f,-0.418701f,
        0.674385f,-0.324767f,-0.287718f,-0.597453f,
        0.511809f,-0.246474f,-0.357079f,-0.741483f,
        0.319488f,-0.153857f,-0.405688f,-0.842421f,
        0.108600f,-0.052299f,-0.430720f,-0.894400f,
        -0.108600f,0.052299f,-0.430720f,-0.894400f,
        -0.319488f,0.153857f,-0.405688f,-0.842421f,
        -0.511809f,0.246474f,-0.357079f,-0.741483f,
        -0.674385f,0.324767f,-0.287718f,-0.597453f,
        -0.797769f,0.384185f,-0.201636f,-0.418701f,
        -0.874789f,0.421276f,-0.103835f,-0.215616f,
        0.781831f,-0.623490f,0.000000f,0.000000f,
        0.771168f,-0.614986f,-0.102623f,-0.128685f,
        0.739470f,-0.589707f,-0.202447f,-0.253860f,
        0.687600f,-0.548343f,-0.296748f,-0.372111f,
        0.616975f,-0.492021f,-0.382955f,-0.480211f,
        0.529520f,-0.422278f,-0.458716f,-0.575212f,
        0.427621f,-0.341017f,-0.521965f,-0.654523f,
        0.314058f,-0.250453f,-0.570975f,-0.715981f,
        0.191928f,-0.153058f,-0.604411f,-0.757908f,
        0.064563f,-0.051487f,-0.621360f,-0.779161f,
        -0.064563f,0.051487f,-0.621360f,-0.779161f,
        -0.191928f,0.153058f,-0.604411f,-0.757908f,
        -0.314058f,0.250453f,-0.570975f,-0.715981f,
        -0.427621f,0.341017f,-0.521965f,-0.654523f,
        -0.529520f,0.422278f,-0.458717f,-0.575212f,
        -0.616975f,0.492021f,-0.382956f,-0.480211f,
        -0.687600f,0.548343f,-0.296748f,-0.372111f,
        -0.739470f,0.589708f,-0.202447f,-0.253860f,
        -0.771169f,0.614986f,-0.102623f,-0.128685f
    };

    static const float orientationDataPrecision1[39*4]=
    { // 3.67 deg. precision
        1.000000f,0.000000f,0.000000f,0.000000f,
        0.999486f,-0.032052f,0.000000f,0.000000f,
        0.865580f,-0.027757f,-0.016026f,-0.499743f,
        0.499743f,-0.016026f,-0.027757f,-0.865580f,
        -0.000000f,0.000000f,-0.032052f,-0.999486f,
        -0.499743f,0.016026f,-0.027757f,-0.865580f,
        -0.865580f,0.027757f,-0.016026f,-0.499743f,
        0.997945f,-0.064070f,0.000000f,0.000000f,
        0.968947f,-0.062208f,-0.015333f,-0.238824f,
        0.883637f,-0.056731f,-0.029775f,-0.463768f,
        0.746973f,-0.047957f,-0.042486f,-0.661760f,
        0.566898f,-0.036396f,-0.052729f,-0.821293f,
        0.353876f,-0.022720f,-0.059907f,-0.933095f,
        0.120289f,-0.007723f,-0.063603f,-0.990669f,
        -0.120289f,0.007723f,-0.063603f,-0.990669f,
        -0.353876f,0.022720f,-0.059907f,-0.933095f,
        -0.566898f,0.036396f,-0.052729f,-0.821293f,
        -0.746973f,0.047957f,-0.042486f,-0.661760f,
        -0.883637f,0.056731f,-0.029775f,-0.463768f,
        -0.968947f,0.062208f,-0.015333f,-0.238824f,
        0.995379f,-0.096023f,0.000000f,0.000000f,
        0.981803f,-0.094713f,-0.015805f,-0.163834f,
        0.941447f,-0.090820f,-0.031179f,-0.323199f,
        0.875410f,-0.084450f,-0.045702f,-0.473748f,
        0.785494f,-0.075776f,-0.058979f,-0.611374f,
        0.674152f,-0.065035f,-0.070646f,-0.732324f,
        0.544421f,-0.052520f,-0.080387f,-0.833298f,
        0.399839f,-0.038572f,-0.087935f,-0.911542f,
        0.244351f,-0.023572f,-0.093085f,-0.964921f,
        0.082198f,-0.007930f,-0.095695f,-0.991980f,
        -0.082198f,0.007930f,-0.095695f,-0.991980f,
        -0.244351f,0.023572f,-0.093085f,-0.964921f,
        -0.399839f,0.038572f,-0.087935f,-0.911542f,
        -0.544421f,0.052520f,-0.080387f,-0.833298f,
        -0.674152f,0.065035f,-0.070646f,-0.732324f,
        -0.785494f,0.075776f,-0.058979f,-0.611375f,
        -0.875410f,0.084450f,-0.045702f,-0.473748f,
        -0.941447f,0.090820f,-0.031179f,-0.323199f,
        -0.981804f,0.094713f,-0.015805f,-0.163834f
    };

    static const float orientationDataPrecision2[39*4]=
    { // 0.52 deg. precision
        1.000000f,0.000000f,0.000000f,0.000000f,
        0.999990f,-0.004580f,0.000000f,0.000000f,
        0.866016f,-0.003966f,-0.002290f,-0.499995f,
        0.499995f,-0.002290f,-0.003966f,-0.866016f,
        -0.000000f,0.000000f,-0.004580f,-0.999990f,
        -0.499995f,0.002290f,-0.003966f,-0.866016f,
        -0.866016f,0.003966f,-0.002290f,-0.499995f,
        0.999958f,-0.009159f,0.000000f,0.000000f,
        0.970901f,-0.008893f,-0.002192f,-0.239306f,
        0.885419f,-0.008110f,-0.004256f,-0.464704f,
        0.748479f,-0.006856f,-0.006074f,-0.663095f,
        0.568041f,-0.005203f,-0.007538f,-0.822949f,
        0.354590f,-0.003248f,-0.008564f,-0.934977f,
        0.120532f,-0.001104f,-0.009092f,-0.992667f,
        -0.120532f,0.001104f,-0.009092f,-0.992667f,
        -0.354590f,0.003248f,-0.008564f,-0.934977f,
        -0.568041f,0.005203f,-0.007538f,-0.822949f,
        -0.748480f,0.006856f,-0.006074f,-0.663095f,
        -0.885419f,0.008110f,-0.004256f,-0.464704f,
        -0.970901f,0.008893f,-0.002192f,-0.239306f,
        0.999906f,-0.013738f,0.000000f,0.000000f,
        0.986268f,-0.013551f,-0.002261f,-0.164579f,
        0.945728f,-0.012994f,-0.004461f,-0.324669f,
        0.879391f,-0.012082f,-0.006539f,-0.475902f,
        0.789066f,-0.010841f,-0.008438f,-0.614155f,
        0.677218f,-0.009305f,-0.010108f,-0.735655f,
        0.546897f,-0.007514f,-0.011501f,-0.837088f,
        0.401658f,-0.005519f,-0.012581f,-0.915687f,
        0.245462f,-0.003373f,-0.013318f,-0.969309f,
        0.082572f,-0.001135f,-0.013691f,-0.996491f,
        -0.082572f,0.001135f,-0.013691f,-0.996491f,
        -0.245462f,0.003373f,-0.013318f,-0.969309f,
        -0.401658f,0.005519f,-0.012581f,-0.915687f,
        -0.546897f,0.007514f,-0.011501f,-0.837088f,
        -0.677218f,0.009305f,-0.010108f,-0.735655f,
        -0.789066f,0.010841f,-0.008438f,-0.614155f,
        -0.879391f,0.012082f,-0.006539f,-0.475903f,
        -0.945728f,0.012994f,-0.004461f,-0.324669f,
        -0.986269f,0.013551f,-0.002261f,-0.164579f
    };

    static const float orientationDataPrecision3[39*4]=
    { // 0.074 deg. precision
        1.000000f,0.000000f,0.000000f,0.000000f,
        1.000000f,-0.000654f,0.000000f,0.000000f,
        0.866025f,-0.000567f,-0.000327f,-0.500000f,
        0.500000f,-0.000327f,-0.000567f,-0.866025f,
        -0.000000f,0.000000f,-0.000654f,-1.000000f,
        -0.500000f,0.000327f,-0.000567f,-0.866025f,
        -0.866025f,0.000567f,-0.000327f,-0.500000f,
        0.999999f,-0.001308f,0.000000f,0.000000f,
        0.970941f,-0.001270f,-0.000313f,-0.239315f,
        0.885455f,-0.001159f,-0.000608f,-0.464723f,
        0.748510f,-0.000979f,-0.000868f,-0.663122f,
        0.568064f,-0.000743f,-0.001077f,-0.822983f,
        0.354605f,-0.000464f,-0.001223f,-0.935015f,
        0.120537f,-0.000158f,-0.001299f,-0.992708f,
        -0.120537f,0.000158f,-0.001299f,-0.992708f,
        -0.354605f,0.000464f,-0.001223f,-0.935015f,
        -0.568064f,0.000743f,-0.001077f,-0.822983f,
        -0.748510f,0.000979f,-0.000868f,-0.663122f,
        -0.885455f,0.001159f,-0.000608f,-0.464723f,
        -0.970941f,0.001270f,-0.000313f,-0.239315f,
        0.999998f,-0.001963f,0.000000f,0.000000f,
        0.986359f,-0.001936f,-0.000323f,-0.164594f,
        0.945815f,-0.001856f,-0.000637f,-0.324699f,
        0.879472f,-0.001726f,-0.000934f,-0.475946f,
        0.789139f,-0.001549f,-0.001206f,-0.614211f,
        0.677280f,-0.001329f,-0.001444f,-0.735722f,
        0.546947f,-0.001073f,-0.001643f,-0.837165f,
        0.401695f,-0.000788f,-0.001797f,-0.915772f,
        0.245485f,-0.000482f,-0.001903f,-0.969398f,
        0.082579f,-0.000162f,-0.001956f,-0.996583f,
        -0.082579f,0.000162f,-0.001956f,-0.996583f,
        -0.245485f,0.000482f,-0.001903f,-0.969398f,
        -0.401695f,0.000788f,-0.001797f,-0.915772f,
        -0.546947f,0.001073f,-0.001643f,-0.837165f,
        -0.677280f,0.001329f,-0.001444f,-0.735723f,
        -0.789139f,0.001549f,-0.001206f,-0.614212f,
        -0.879472f,0.001726f,-0.000934f,-0.475947f,
        -0.945816f,0.001856f,-0.000637f,-0.324699f,
        -0.986360f,0.001936f,-0.000323f,-0.164594f
    };

    // Following are 5 sets of circular orientation patterns. Patterns are same, scale is different.
    static const float orientation2DataPrecision0[5*4]=
    { // 18 deg. precision
        0.951057f,0.000000f,0.000000f,0.309017f,
        0.987688f,0.000000f,0.000000f,0.156434f,
        1.000000f,0.000000f,0.000000f,-0.000000f,
        0.987688f,0.000000f,0.000000f,-0.156434f,
        0.951057f,0.000000f,0.000000f,-0.309017f
    };
    static const float orientation2DataPrecision1[5*4]=
    { // 3.6 deg. precision
        0.998027f,0.000000f,0.000000f,0.062791f,
        0.999507f,0.000000f,0.000000f,0.031411f,
        1.000000f,0.000000f,0.000000f,-0.000000f,
        0.999506f,0.000000f,0.000000f,-0.031411f,
        0.998027f,0.000000f,0.000000f,-0.062791f
    };
    static const float orientation2DataPrecision2[5*4]=
    { // 0.72 deg. precision
        0.999921f,0.000000f,0.000000f,0.012566f,
        0.999980f,0.000000f,0.000000f,0.006283f,
        1.000000f,0.000000f,0.000000f,0.000000f,
        0.999980f,0.000000f,0.000000f,-0.006283f,
        0.999921f,0.000000f,0.000000f,-0.012566f
    };
    static const float orientation2DataPrecision3[5*4]=
    { // 0.144 deg. precision
        0.999997f,0.000000f,0.000000f,0.002513f,
        0.999999f,0.000000f,0.000000f,0.001257f,
        1.000000f,0.000000f,0.000000f,0.000000f,
        0.999999f,0.000000f,0.000000f,-0.001257f,
        0.999997f,0.000000f,0.000000f,-0.002513f
    };
    static const float orientation2DataPrecision4[5*4]=
    { // 0.0288 deg. precision
        1.000000f,0.000000f,0.000000f,0.000503f,
        1.000000f,0.000000f,0.000000f,0.000251f,
        1.000000f,0.000000f,0.000000f,0.000000f,
        1.000000f,0.000000f,0.000000f,-0.000251f,
        1.000000f,0.000000f,0.000000f,-0.000503f
    };

    // First pass (roughest) (spherical):
    C4Vector bestSmallestDirection;
    float smallestDimension=999999999.0f;
    for (int i=0;i<39;i++)
    {
        float minMax[2]={+999999999.0f,-999999999.0f};
        for (int j=0;j<int(vert.size());j++)
        {
            C3Vector w(((C4Vector*)orientationDataPrecision0)[i]*vert[j]);
            if (w(2)<minMax[0])
                minMax[0]=w(2);
            if (w(2)>minMax[1])
                minMax[1]=w(2);
            if (minMax[1]-minMax[0]>smallestDimension)
                break;
        }
        float currSmallest=minMax[1]-minMax[0];
        if (currSmallest<smallestDimension)
        {
            smallestDimension=currSmallest;
            bestSmallestDirection=((C4Vector*)orientationDataPrecision0)[i].getInverse();
        }
    }

    // Second pass (spherical):
    C4Vector bestSmallestDirectionPrevious(bestSmallestDirection);
    smallestDimension=999999999.0f;
    for (int i=0;i<39;i++)
    {
        float minMax[2]={+999999999.0f,-999999999.0f};
        C4Vector currentOrientation((bestSmallestDirectionPrevious*((C4Vector*)orientationDataPrecision1)[i].getInverse()).getInverse());
        for (int j=0;j<int(vert.size());j++)
        {
            C3Vector w(currentOrientation*vert[j]);
            if (w(2)<minMax[0])
                minMax[0]=w(2);
            if (w(2)>minMax[1])
                minMax[1]=w(2);
            if (minMax[1]-minMax[0]>smallestDimension)
                break;
        }
        float currSmallest=minMax[1]-minMax[0];
        if (currSmallest<smallestDimension)
        {
            smallestDimension=currSmallest;
            bestSmallestDirection=currentOrientation.getInverse();
        }
    }

    // Third pass (spherical):
    bestSmallestDirectionPrevious=bestSmallestDirection;
    smallestDimension=999999999.0f;
    for (int i=0;i<39;i++)
    {
        float minMax[2]={+999999999.0f,-999999999.0f};
        C4Vector currentOrientation((bestSmallestDirectionPrevious*((C4Vector*)orientationDataPrecision2)[i].getInverse()).getInverse());
        for (int j=0;j<int(vert.size());j++)
        {
            C3Vector w(currentOrientation*vert[j]);
            if (w(2)<minMax[0])
                minMax[0]=w(2);
            if (w(2)>minMax[1])
                minMax[1]=w(2);
            if (minMax[1]-minMax[0]>smallestDimension)
                break;
        }
        float currSmallest=minMax[1]-minMax[0];
        if (currSmallest<smallestDimension)
        {
            smallestDimension=currSmallest;
            bestSmallestDirection=currentOrientation.getInverse();
        }
    }

    // Forth and last pass (most precise one) (spherical):
    bestSmallestDirectionPrevious=bestSmallestDirection;
    smallestDimension=999999999.0f;
    for (int i=0;i<39;i++)
    {
        float minMax[2]={+999999999.0f,-999999999.0f};
        C4Vector currentOrientation((bestSmallestDirectionPrevious*((C4Vector*)orientationDataPrecision3)[i].getInverse()).getInverse());
        for (int j=0;j<int(vert.size());j++)
        {
            C3Vector w(currentOrientation*vert[j]);
            if (w(2)<minMax[0])
                minMax[0]=w(2);
            if (w(2)>minMax[1])
                minMax[1]=w(2);
            if (minMax[1]-minMax[0]>smallestDimension)
                break;
        }
        float currSmallest=minMax[1]-minMax[0];
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
    float smallestArea=999999999999999999.0f;
    for (int i=0;i<5;i++)
    {
        float minMaxX[2]={+999999999.0f,-999999999.0f};
        float minMaxY[2]={+999999999.0f,-999999999.0f};
        C4Vector currentOrientation((bestSmallestDirectionPrevious*((C4Vector*)orientation2DataPrecision0)[i].getInverse()).getInverse());
        for (int j=0;j<int(vert.size());j++)
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
        float currSmallest=(minMaxX[1]-minMaxX[0])*(minMaxY[1]-minMaxY[0]);
        xAxisIsLargerThanYAxis=(minMaxX[1]-minMaxX[0]>minMaxY[1]-minMaxY[0]);
        if (currSmallest<smallestArea)
        {
            smallestArea=currSmallest;
            bestSmallestDirection=currentOrientation.getInverse();
        }
    }

    // Second pass (circular):
    bestSmallestDirectionPrevious=bestSmallestDirection;
    smallestArea=999999999999999999.0f;
    for (int i=0;i<5;i++)
    {
        float minMaxX[2]={+999999999.0f,-999999999.0f};
        float minMaxY[2]={+999999999.0f,-999999999.0f};
        C4Vector currentOrientation((bestSmallestDirectionPrevious*((C4Vector*)orientation2DataPrecision1)[i].getInverse()).getInverse());
        for (int j=0;j<int(vert.size());j++)
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
        float currSmallest=(minMaxX[1]-minMaxX[0])*(minMaxY[1]-minMaxY[0]);
        xAxisIsLargerThanYAxis=(minMaxX[1]-minMaxX[0]>minMaxY[1]-minMaxY[0]);
        if (currSmallest<smallestArea)
        {
            smallestArea=currSmallest;
            bestSmallestDirection=currentOrientation.getInverse();
        }
    }

    // Third pass (circular):
    bestSmallestDirectionPrevious=bestSmallestDirection;
    smallestArea=999999999999999999.0f;
    for (int i=0;i<5;i++)
    {
        float minMaxX[2]={+999999999.0f,-999999999.0f};
        float minMaxY[2]={+999999999.0f,-999999999.0f};
        C4Vector currentOrientation((bestSmallestDirectionPrevious*((C4Vector*)orientation2DataPrecision2)[i].getInverse()).getInverse());
        for (int j=0;j<int(vert.size());j++)
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
        float currSmallest=(minMaxX[1]-minMaxX[0])*(minMaxY[1]-minMaxY[0]);
        xAxisIsLargerThanYAxis=(minMaxX[1]-minMaxX[0]>minMaxY[1]-minMaxY[0]);
        if (currSmallest<smallestArea)
        {
            smallestArea=currSmallest;
            bestSmallestDirection=currentOrientation.getInverse();
        }
    }

    // Forth pass (circular):
    bestSmallestDirectionPrevious=bestSmallestDirection;
    smallestArea=999999999999999999.0f;
    for (int i=0;i<5;i++)
    {
        float minMaxX[2]={+999999999.0f,-999999999.0f};
        float minMaxY[2]={+999999999.0f,-999999999.0f};
        C4Vector currentOrientation((bestSmallestDirectionPrevious*((C4Vector*)orientation2DataPrecision3)[i].getInverse()).getInverse());
        for (int j=0;j<int(vert.size());j++)
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
        float currSmallest=(minMaxX[1]-minMaxX[0])*(minMaxY[1]-minMaxY[0]);
        xAxisIsLargerThanYAxis=(minMaxX[1]-minMaxX[0]>minMaxY[1]-minMaxY[0]);
        if (currSmallest<smallestArea)
        {
            smallestArea=currSmallest;
            bestSmallestDirection=currentOrientation.getInverse();
        }
    }

    // Fifth and last pass (circular):
    bestSmallestDirectionPrevious=bestSmallestDirection;
    smallestArea=999999999999999999.0f;
    for (int i=0;i<5;i++)
    {
        float minMaxX[2]={+999999999.0f,-999999999.0f};
        float minMaxY[2]={+999999999.0f,-999999999.0f};
        C4Vector currentOrientation((bestSmallestDirectionPrevious*((C4Vector*)orientation2DataPrecision4)[i].getInverse()).getInverse());
        for (int j=0;j<int(vert.size());j++)
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
        float currSmallest=(minMaxX[1]-minMaxX[0])*(minMaxY[1]-minMaxY[0]);
        xAxisIsLargerThanYAxis=(minMaxX[1]-minMaxX[0]>minMaxY[1]-minMaxY[0]);
        if (currSmallest<smallestArea)
        {
            smallestArea=currSmallest;
            bestSmallestDirection=currentOrientation.getInverse();
        }
    }

    C3Vector alternateCenter;
    float alternateSize=SIM_MAX_FLOAT;
    C4Vector bestSmallestDirectionAlternative;
    if (veryPreciseWithTriangles&(indices!=nullptr))
    {
        C3X3Matrix alternativeFrame(bestSmallestDirection);
        std::vector<int> tris;
        if (triIndices!=nullptr)
        {
            tris.reserve(triIndicesLength);
            for (int i=0;i<int(triIndicesLength);i++)
                tris.push_back(triIndices[i]);
        }
        else
        {
            tris.reserve(indicesLength/3);
            for (int i=0;i<int(indicesLength/3);i++)
                tris.push_back(i);
        }
        // Now search for the triangle with largest surface:
        float ls=0.0f;
        C3Vector ltn;
        for (int i=0;i<int(tris.size());i++)
        {
            int in[3]={indices[3*tris[i]+0],indices[3*tris[i]+1],indices[3*tris[i]+2]};
            C3Vector a0(&vertices[3*in[0]+0]);
            C3Vector a1(&vertices[3*in[1]+0]);
            C3Vector a2(&vertices[3*in[2]+0]);
            C3Vector e0(a2-a0);
            C3Vector e1(a1-a0);
            C3Vector n(e0^e1);
            float s=n.getLength();
            if (s>ls)
            {
                ls=s;
                ltn=n/s;
            }
        }
        // We now adjust the alternative frame with the ltn:
        C3Vector mostSimilar;
        float scalarResult=0;
        for (int i=0;i<3;i++)
        {
            float l=fabs(alternativeFrame.axis[i]*ltn);
            if (l>scalarResult)
            {
                scalarResult=l;
                mostSimilar=alternativeFrame.axis[i];
            }
        }
        if (mostSimilar*ltn<0.0f)
            ltn*=-1.0f; // take care of direction
        if (mostSimilar*ltn!=1.0f)
        { // make sure they are not already colinear!
            C4Vector q(mostSimilar,ltn); // We build the transformation from mostSimilar to ltn
            alternativeFrame=(q*alternativeFrame.getQuaternion()).getMatrix();
        }

        // We now search for the triangle with larget surface perpendicular to the other one:
        ls=0.0f;
        C3Vector ltnp;
        for (int i=0;i<int(tris.size());i++)
        {
            int in[3]={indices[3*tris[i]+0],indices[3*tris[i]+1],indices[3*tris[i]+2]};
            C3Vector a0(&vertices[3*in[0]+0]);
            C3Vector a1(&vertices[3*in[1]+0]);
            C3Vector a2(&vertices[3*in[2]+0]);
            C3Vector e0(a2-a0);
            C3Vector e1(a1-a0);
            C3Vector n(e0^e1);
            float s=n.getLength();
            if (s>ls)
            {
                n/=s;
                if (fabs(n.getAngle(ltn)-1.57079633f)<1.0f*degToRad_f)
                {
                    ltnp=n;
                    ls=s;
                }
            }
        }
        if (ls!=0.0f)
        { // ok we found a perpendicular triangle
            // We now adjust the alternative frame with the ltnp:
            C3Vector mostSimilar;
            float scalarResult=0;
            for (int i=0;i<3;i++)
            {
                float l=fabs(alternativeFrame.axis[i]*ltnp);
                if (l>scalarResult)
                {
                    scalarResult=l;
                    mostSimilar=alternativeFrame.axis[i];
                }
            }
            if (mostSimilar*ltnp<0.0f)
                ltnp*=-1.0f; // take care of direction
            if (mostSimilar*ltnp!=1.0f)
            { // make sure they are not already colinear!
                // now project mostSimilar and ltnp into the plane defined by ltn:
                mostSimilar-=ltn*(mostSimilar*ltn);
                ltnp-=ltn*(ltnp*ltn);
                mostSimilar.normalize();
                ltnp.normalize();
                if (mostSimilar*ltnp!=1.0f)
                { // make sure they are not already colinear again!
                    C4Vector q(mostSimilar,ltnp); // We build the transformation from mostSimilar to ltnp
                    alternativeFrame=(q*alternativeFrame.getQuaternion()).getMatrix();
                }
            }
        }
        bestSmallestDirectionAlternative=alternativeFrame.getQuaternion();
        // We now check the size of the alternate bounding box:
        C3Vector minV(+999999999.0f,+999999999.0f,+999999999.0f);
        C3Vector maxV(-999999999.0f,-999999999.0f,-999999999.0f);
        for (int j=0;j<int(vert.size());j++)
        {
            C3Vector w(bestSmallestDirectionAlternative.getInverse()*vert[j]);
            minV.keepMin(w);
            maxV.keepMax(w);
        }
        alternateCenter=(maxV+minV)*0.5f;
        C3Vector s(maxV-minV);
        alternateSize=s(0)*s(1)*s(2);
        // Now order the new frame like: x=smallest size, z=biggest size:
        C3X3Matrix m(bestSmallestDirectionAlternative);
        C3Vector biggest;
        C3Vector smallest;
        float smallestS=SIM_MAX_FLOAT;
        float biggestS=0.0f;
        for (int i=0;i<3;i++)
        {
            float l=s(i);
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
    }

    // We search the center:
    C3Vector minV(+999999999.0f,+999999999.0f,+999999999.0f);
    C3Vector maxV(-999999999.0f,-999999999.0f,-999999999.0f);
    for (int j=0;j<int(vert.size());j++)
    {
        C3Vector w(bestSmallestDirection.getInverse()*vert[j]);
        minV.keepMin(w);
        maxV.keepMax(w);
    }

    C3Vector center=(maxV+minV)*0.5f;
    C3Vector s(maxV-minV);
    float size=s(0)*s(1)*s(2);

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

bool CAlgos::isBoxOutsideVolumeApprox(const C4X4Matrix& tr,const C3Vector& s,const std::vector<float>* planes)
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
        float d=(*planes)[4*i+3];
        if ((abc*edges[0]+d)>=0.0f)
        {
            if ((abc*edges[1]+d)>=0.0f)
            {
                if ((abc*edges[2]+d)>=0.0f)
                {
                    if ((abc*edges[3]+d)>=0.0f)
                    {
                        if ((abc*edges[4]+d)>=0.0f)
                        {
                            if ((abc*edges[5]+d)>=0.0f)
                            {
                                if ((abc*edges[6]+d)>=0.0f)
                                {
                                    if ((abc*edges[7]+d)>=0.0f)
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
