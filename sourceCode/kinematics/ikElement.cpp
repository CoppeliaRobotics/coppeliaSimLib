#include "simInternal.h"
#include "ikElement.h"
#include "app.h"
#include "tt.h"
#include "pluginContainer.h"

CIkElement::CIkElement()
{
    _commonInit();
}

CIkElement::CIkElement(int theTooltip)
{
    _commonInit();
    _tipHandle=theTooltip;
}

void CIkElement::_commonInit()
{
    _tipHandle=-1;
    _ikElementPluginCounterpartHandle=-1;
    _ikGroupPluginCounterpartHandle=-1;
}

CIkElement::~CIkElement()
{
}

void CIkElement::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
}

void CIkElement::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
}

CIkElement* CIkElement::copyYourself() const
{
    CIkElement* newEl=new CIkElement();
    newEl->_objectHandle=_objectHandle; // important for copy operations connections
    newEl->_constraintBaseHandle=_constraintBaseHandle;
    newEl->_tipHandle=_tipHandle;
    newEl->_baseHandle=_baseHandle;
    newEl->_minAngularPrecision=_minAngularPrecision;
    newEl->_minLinearPrecision=_minLinearPrecision;
    newEl->_enabled=_enabled;
    newEl->_constraints=_constraints;
    newEl->_positionWeight=_positionWeight;
    newEl->_orientationWeight=_orientationWeight;
    return(newEl);
}

void CIkElement::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Oid");
            ar << _objectHandle;
            ar.flush();

            ar.storeDataName("Ik2");
            ar << _tipHandle << _baseHandle;
            ar.flush();

            ar.storeDataName("Abc");
            ar << _constraintBaseHandle;
            ar.flush();

            ar.storeDataName("Pr2");
            ar << _minAngularPrecision << _minLinearPrecision;
            ar.flush();

            ar.storeDataName("Ctr");
            ar << _constraints;
            ar.flush();

            ar.storeDataName("Wgt");
            ar << _positionWeight << _orientationWeight;
            ar.flush();

            ar.storeDataName("Var");
            unsigned char nothing=0;
            nothing=nothing+1*_enabled;
            ar << nothing;
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            int byteQuantity;
            std::string theName="";
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("Oid")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _objectHandle;
                    }
                    if (theName.compare("Ik2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _tipHandle >> _baseHandle;
                    }
                    if (theName.compare("Abc")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _constraintBaseHandle;
                    }
                    if (theName.compare("Prc")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _minAngularPrecision >> _minLinearPrecision;
                        _minAngularPrecision*=degToRad_f;
                    }
                    if (theName.compare("Pr2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _minAngularPrecision >> _minLinearPrecision;
                    }
                    if (theName.compare("Ctr")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _constraints;
                    }
                    if (theName.compare("Wgt")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _positionWeight >> _orientationWeight;
                    }
                    if (theName.compare("Var")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _enabled=((nothing&1)!=0);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
    else
    {
        bool exhaustiveXml=( (ar.getFileType()!=CSer::filetype_csim_xml_simplescene_file)&&(ar.getFileType()!=CSer::filetype_csim_xml_simplemodel_file) );
        if (ar.isStoring())
        {
            if (exhaustiveXml)
                ar.xmlAddNode_int("id",_objectHandle);

            if (exhaustiveXml)
            {
                ar.xmlAddNode_int("tipHandle",_tipHandle);
                ar.xmlAddNode_int("baseHandle",_baseHandle);
                ar.xmlAddNode_int("alternateBaseHandle",_constraintBaseHandle);
            }
            else
            {
                std::string str;
                CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(_tipHandle);
                if (it!=nullptr)
                    str=it->getObjectName();
                ar.xmlAddNode_comment(" 'tip' tag: is required and has to be the name of an existing scene object ",exhaustiveXml);
                ar.xmlAddNode_string("tip",str.c_str());
                str.clear();
                it=App::currentWorld->sceneObjects->getObjectFromHandle(_baseHandle);
                if (it!=nullptr)
                    str=it->getObjectName();
                ar.xmlAddNode_string("base",str.c_str());
                str.clear();
                it=App::currentWorld->sceneObjects->getObjectFromHandle(_constraintBaseHandle);
                if (it!=nullptr)
                    str=it->getObjectName();
                ar.xmlAddNode_string("alternateBase",str.c_str());
            }

            ar.xmlPushNewNode("precision");
            ar.xmlAddNode_float("linear",_minLinearPrecision);
            ar.xmlAddNode_float("angular",_minAngularPrecision*180.0f/piValue_f);
            ar.xmlPopNode();

            ar.xmlPushNewNode("weight");
            ar.xmlAddNode_float("position",_positionWeight);
            ar.xmlAddNode_float("orientation",_orientationWeight);
            ar.xmlPopNode();

            ar.xmlPushNewNode("constraints");
            ar.xmlAddNode_bool("x",_constraints&sim_ik_x_constraint);
            ar.xmlAddNode_bool("y",_constraints&sim_ik_y_constraint);
            ar.xmlAddNode_bool("z",_constraints&sim_ik_z_constraint);
            ar.xmlAddNode_bool("alpha_beta",_constraints&sim_ik_alpha_beta_constraint);
            ar.xmlAddNode_bool("gamma",_constraints&sim_ik_gamma_constraint);
            ar.xmlPopNode();

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("enabled",_enabled);
            ar.xmlPopNode();
        }
        else
        {
            if (exhaustiveXml)
                ar.xmlGetNode_int("id",_objectHandle);

            if (exhaustiveXml)
            {
                ar.xmlGetNode_int("tipHandle",_tipHandle);
                ar.xmlGetNode_int("baseHandle",_baseHandle);
                ar.xmlGetNode_int("alternateBaseHandle",_constraintBaseHandle);
            }
            else
            {
                ar.xmlGetNode_string("tip",_tipLoadName,exhaustiveXml);
                ar.xmlGetNode_string("base",_baseLoadName,exhaustiveXml);
                ar.xmlGetNode_string("alternateBase",_altBaseLoadName,exhaustiveXml);
            }

            if (ar.xmlPushChildNode("precision",exhaustiveXml))
            {
                ar.xmlGetNode_float("linear",_minLinearPrecision,exhaustiveXml);
                if (ar.xmlGetNode_float("angular",_minAngularPrecision,exhaustiveXml))
                    _minAngularPrecision*=piValue_f/180.0f;
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("weight",exhaustiveXml))
            {
                ar.xmlGetNode_float("position",_positionWeight,exhaustiveXml);
                ar.xmlGetNode_float("orientation",_orientationWeight,exhaustiveXml);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("constraints",exhaustiveXml))
            {
                _constraints=0;
                bool tmp;
                if (ar.xmlGetNode_bool("x",tmp,exhaustiveXml)&&tmp)
                    _constraints|=sim_ik_x_constraint;
                if (ar.xmlGetNode_bool("y",tmp,exhaustiveXml)&&tmp)
                    _constraints|=sim_ik_y_constraint;
                if (ar.xmlGetNode_bool("z",tmp,exhaustiveXml)&&tmp)
                    _constraints|=sim_ik_z_constraint;
                if (ar.xmlGetNode_bool("alpha_beta",tmp,exhaustiveXml)&&tmp)
                    _constraints|=sim_ik_alpha_beta_constraint;
                if (ar.xmlGetNode_bool("gamma",tmp,exhaustiveXml)&&tmp)
                    _constraints|=sim_ik_gamma_constraint;
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("switches",exhaustiveXml))
            {
                ar.xmlGetNode_bool("enabled",_enabled,exhaustiveXml);
                ar.xmlPopNode();
            }
        }
    }
}

void CIkElement::serializeWExtIk(CExtIkSer& ar) const
{
    ar.writeInt(_objectHandle);

    ar.writeInt(_tipHandle);
    ar.writeInt(_baseHandle);

    ar.writeInt(_constraintBaseHandle);

    ar.writeFloat(_minAngularPrecision);
    ar.writeFloat(_minLinearPrecision);

    ar.writeInt(_constraints);

    ar.writeFloat(_positionWeight);
    ar.writeFloat(_orientationWeight);

    unsigned char nothing=0;
    nothing=nothing+1*_enabled;
    ar.writeByte(nothing);
}

bool CIkElement::announceObjectWillBeErased(int objID,bool copyBuffer)
{ // Return value true means that this IK el object should be destroyed
    if ((_baseHandle==objID)||(_constraintBaseHandle==objID)||(_tipHandle==objID))
        return(true); // This element has to be erased!
    return(false);
}

void CIkElement::performObjectLoadingMapping(const std::vector<int>* map)
{
    _tipHandle=CWorld::getLoadingMapping(map,_tipHandle);
    _baseHandle=CWorld::getLoadingMapping(map,_baseHandle);
    _constraintBaseHandle=CWorld::getLoadingMapping(map,_constraintBaseHandle);
}

int CIkElement::getTargetHandle() const
{
    CDummy* tip=App::currentWorld->sceneObjects->getDummyFromHandle(_tipHandle);
    if (tip==nullptr)
        return(-1);
    int linkedDummyHandle=tip->getLinkedDummyHandle();
    if (linkedDummyHandle==-1)
        return(-1);
    if (tip->getLinkType()!=sim_dummy_linktype_ik_tip_target)
        return(-1);
    return(linkedDummyHandle); // this should be the target!
}

bool CIkElement::setMinLinearPrecision(float prec)
{ // Overridden from _CIkElement_
    tt::limitValue(0.00001f,1.0f,prec);
    return(_CIkElement_::setMinLinearPrecision(prec));
}

bool CIkElement::setMinAngularPrecision(float prec)
{ // Overridden from _CIkElement_
    tt::limitValue(0.001f*degToRad_f,180.0f*degToRad_f,prec);
    return(_CIkElement_::setMinAngularPrecision(prec));
}

bool CIkElement::setPositionWeight(float weight)
{ // Overridden from _CIkElement_
    tt::limitValue(0.001f,1.0f,weight);
    return(_CIkElement_::setPositionWeight(weight));
}

bool CIkElement::setOrientationWeight(float weight)
{ // Overridden from _CIkElement_
    tt::limitValue(0.001f,1.0f,weight);
    return(_CIkElement_::setOrientationWeight(weight));
}

void CIkElement::setAllInvolvedJointsToIkPluginPositions() const
{
    if (_enabled)
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getDummyFromHandle(_tipHandle);
        CSceneObject* baseObj=App::currentWorld->sceneObjects->getObjectFromHandle(_baseHandle);
        while ((it!=baseObj)&&(it!=nullptr))
        {
            it=it->getParent();
            if ( (it!=nullptr)&&(it!=baseObj) )
            {
                if (it->getObjectType()==sim_object_joint_type)
                {
                    CJoint* joint=(CJoint*)it;
                    if ( (joint->getJointMode()==sim_jointmode_ik_deprecated)||(joint->getJointMode()==sim_jointmode_reserved_previously_ikdependent) )
                    {
                        int h=joint->getIkPluginCounterpartHandle();
                        if (joint->getJointType()==sim_joint_spherical_subtype)
                            joint->setSphericalTransformation(CPluginContainer::ikPlugin_getSphericalJointQuaternion(h));
                        else
                            joint->setPosition(CPluginContainer::ikPlugin_getJointPosition(h));
                    }
                }
            }
        }
    }
}

void CIkElement::setAllInvolvedJointsToNewJointMode(int jointMode) const
{
    CSceneObject* iterat=App::currentWorld->sceneObjects->getDummyFromHandle(_tipHandle);
    CSceneObject* baseObj=App::currentWorld->sceneObjects->getObjectFromHandle(_baseHandle);
    while ((iterat!=baseObj)&&(iterat!=nullptr))
    {
        iterat=iterat->getParent();
        if ( (iterat!=nullptr)&&(iterat!=baseObj) )
        {
            if (iterat->getObjectType()==sim_object_joint_type)
                ((CJoint*)iterat)->setJointMode_noDynMotorTargetPosCorrection(jointMode);
        }
    }
}

void CIkElement::_setEnabled_send(bool e) const
{ // Overridden from _CIkElement_
    _CIkElement_::_setEnabled_send(e);

    // Synchronize with IK plugin:
    if (_ikElementPluginCounterpartHandle!=-1)
    {
        int flags=0;
        if (e)
            flags=flags|1;
        CPluginContainer::ikPlugin_setIkElementFlags(_ikGroupPluginCounterpartHandle,_ikElementPluginCounterpartHandle,flags);
    }
}

void CIkElement::_setBase_send(int h) const
{ // Overridden from _CIkElement_
    _CIkElement_::_setBase_send(h);

    // Synchronize with IK plugin:
    if (_ikElementPluginCounterpartHandle!=-1)
    {
        int bh=-1;
        CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(h);
        if (obj!=nullptr)
            bh=obj->getIkPluginCounterpartHandle();
        int abh=-1;
        obj=App::currentWorld->sceneObjects->getObjectFromHandle(_constraintBaseHandle);
        if (obj!=nullptr)
            abh=obj->getIkPluginCounterpartHandle();
        CPluginContainer::ikPlugin_setIkElementBase(_ikGroupPluginCounterpartHandle,_ikElementPluginCounterpartHandle,bh,abh);
    }
}

void CIkElement::_setAlternativeBaseForConstraints_send(int h) const
{ // Overridden from _CIkElement_
    _CIkElement_::_setAlternativeBaseForConstraints_send(h);

    // Synchronize with IK plugin:
    if (_ikElementPluginCounterpartHandle!=-1)
    {
        int bh=-1;
        CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(_baseHandle);
        if (obj!=nullptr)
            bh=obj->getIkPluginCounterpartHandle();
        int abh=-1;
        obj=App::currentWorld->sceneObjects->getObjectFromHandle(h);
        if (obj!=nullptr)
            abh=obj->getIkPluginCounterpartHandle();
        CPluginContainer::ikPlugin_setIkElementBase(_ikGroupPluginCounterpartHandle,_ikElementPluginCounterpartHandle,bh,abh);
    }
}

void CIkElement::_setMinLinearPrecision_send(float f) const
{ // Overridden from _CIkElement_
    _CIkElement_::_setMinLinearPrecision_send(f);

    // Synchronize with IK plugin:
    if (_ikElementPluginCounterpartHandle!=-1)
        CPluginContainer::ikPlugin_setIkElementPrecision(_ikGroupPluginCounterpartHandle,_ikElementPluginCounterpartHandle,f,_minAngularPrecision);
}

void CIkElement::_setMinAngularPrecision_send(float f) const
{ // Overridden from _CIkElement_
    _CIkElement_::_setMinAngularPrecision_send(f);

    // Synchronize with IK plugin:
    if (_ikElementPluginCounterpartHandle!=-1)
        CPluginContainer::ikPlugin_setIkElementPrecision(_ikGroupPluginCounterpartHandle,_ikElementPluginCounterpartHandle,_minLinearPrecision,f);
}

void CIkElement::_setPositionWeight_send(float f) const
{ // Overridden from _CIkElement_
    _CIkElement_::_setPositionWeight_send(f);

    // Synchronize with IK plugin:
    if (_ikElementPluginCounterpartHandle!=-1)
        CPluginContainer::ikPlugin_setIkElementWeights(_ikGroupPluginCounterpartHandle,_ikElementPluginCounterpartHandle,f,_orientationWeight);
}

void CIkElement::_setOrientationWeight_send(float f) const
{ // Overridden from _CIkElement_
    _CIkElement_::_setOrientationWeight_send(f);

    // Synchronize with IK plugin:
    if (_ikElementPluginCounterpartHandle!=-1)
        CPluginContainer::ikPlugin_setIkElementWeights(_ikGroupPluginCounterpartHandle,_ikElementPluginCounterpartHandle,_positionWeight,f);
}

void CIkElement::_setConstraints_send(int c) const
{ // Overridden from _CIkElement_
    _CIkElement_::_setConstraints_send(c);

    // Synchronize with IK plugin:
    if (_ikElementPluginCounterpartHandle!=-1)
        CPluginContainer::ikPlugin_setIkElementConstraints(_ikGroupPluginCounterpartHandle,_ikElementPluginCounterpartHandle,c);
}

std::string CIkElement::getTipLoadName() const
{
    return(_tipLoadName);
}

std::string CIkElement::getBaseLoadName() const
{
    return(_baseLoadName);
}

std::string CIkElement::getAltBaseLoadName() const
{
    return(_altBaseLoadName);
}

void CIkElement::setIkGroupPluginCounterpartHandle(int h)
{
    _ikGroupPluginCounterpartHandle=h;
}

int CIkElement::getIkPluginCounterpartHandle() const
{
    return(_ikElementPluginCounterpartHandle);
}

void CIkElement::buildUpdateAndPopulateSynchronizationObject(const std::vector<SSyncRoute>* parentRouting)
{ // Overridden from CSyncObject
    if (setObjectCanSync(true))
    {
        // Set routing:
        SSyncRoute r;
        r.objHandle=_objectHandle;
        r.objType=sim_syncobj_ikelement;
        setSyncMsgRouting(parentRouting,r);

        // Build remote IK element:
        sendInt32(_tipHandle,sim_syncobj_ikelement_create);

        // Build remote IK element in IK plugin:
        int counterpartHandle=-1;
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(_tipHandle);
        if (it!=nullptr)
            counterpartHandle=it->getIkPluginCounterpartHandle();
        _ikElementPluginCounterpartHandle=CPluginContainer::ikPlugin_addIkElement(_ikGroupPluginCounterpartHandle,counterpartHandle);

        // Update remote IK element:
        _setEnabled_send(_enabled);
        _setBase_send(_baseHandle);
        _setAlternativeBaseForConstraints_send(_constraintBaseHandle);
        _setMinLinearPrecision_send(_minLinearPrecision);
        _setMinAngularPrecision_send(_minAngularPrecision);
        _setPositionWeight_send(_positionWeight);
        _setOrientationWeight_send(_orientationWeight);
        _setConstraints_send(_constraints);
    }
}

void CIkElement::connectSynchronizationObject()
{ // Overridden from CSyncObject
    if (getObjectCanSync())
    {
    }
}

void CIkElement::removeSynchronizationObject(bool localReferencesToItOnly)
{ // Overridden from CSyncObject
    if (getObjectCanSync())
    {
        setObjectCanSync(false);

        if (!localReferencesToItOnly)
        {
            // Delete remote IK element:
            sendVoid(sim_syncobj_ikelement_delete);

            // Synchronize with IK plugin:
            if ( (_ikGroupPluginCounterpartHandle!=-1)&&(_ikElementPluginCounterpartHandle!=-1) )
                CPluginContainer::ikPlugin_eraseIkElement(_ikGroupPluginCounterpartHandle,_ikElementPluginCounterpartHandle);
        }
    }
    // IK plugin part:
    _ikGroupPluginCounterpartHandle=-1;
    _ikElementPluginCounterpartHandle=-1;
}

