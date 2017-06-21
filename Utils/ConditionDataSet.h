#ifndef _CONDITIONDATASET_H__
#define _CONDITIONDATASET_H__

#include <vector>
#include <iostream>

#include "../HWDescription/Definition.h"

class ConditionDataSet
{
  public:
    SLinkDebugMode fDebugMode;
    bool fCondData;

    ConditionDataSet (SLinkDebugMode pMode, bool pCondData) :
        fDebugMode (pMode),
        fCondData (pCondData)
    {}

};

#endif
