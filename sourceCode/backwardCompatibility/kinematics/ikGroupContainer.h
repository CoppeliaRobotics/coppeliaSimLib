#pragma once

#include <ikGroup_old.h>
#include <_ikGroupContainer_.h>

class CIkGroupContainer : public _CIkGroupContainer_
{
  public:
    CIkGroupContainer();
    virtual ~CIkGroupContainer();

    void buildOrUpdate_oldIk();
    void connect_oldIk();
    void remove_oldIk();

    void simulationAboutToStart();
    void simulationEnded();

    void addIkGroup(CIkGroup_old* anIkGroup, bool objectIsACopy);
    void addIkGroupWithSuffixOffset(CIkGroup_old* anIkGroup, bool objectIsACopy, int suffixOffset);

    void getMinAndMaxNameSuffixes(int& minSuffix, int& maxSuffix) const;
    bool canSuffix1BeSetToSuffix2(int suffix1, int suffix2) const;
    void setSuffix1ToSuffix2(int suffix1, int suffix2);

    void removeIkGroup(int ikGroupHandle);
    void removeAllIkGroups();

    // Overridden from _CIkGroupContainer_:
    bool shiftIkGroup(int ikGroupHandle, bool shiftUp);

    void announceObjectWillBeErased(int objHandle);
    void announceIkGroupWillBeErased(int ikGroupHandle);
    int computeAllIkGroups(bool exceptExplicitHandling);
    void resetCalculationResults();

  protected:
    // Overridden from _CIkGroupContainer_:
    void _addIkGroup(CIkGroup_old* anIkGroup);
    void _removeIkGroup(int ikGroupHandle);
};
