/*
 * Datapoint utility.
 *
 * Copyright (c) 2020, RTE (https://www.rte-france.com)
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Yannick Marchetaux
 * 
 */
#include <datapointUtility.h>

using namespace std;

/*
*
*/
DatapointUtility::Datapoints* DatapointUtility::findDictElement(Datapoints* dict, const string& key) {
    if (dict != nullptr) {
        for (Datapoint* dp : *dict) {
            if (dp->getName() == key) {
                DatapointValue& data = dp->getData();
                if (data.getType() == DatapointValue::T_DP_DICT) {
                    return data.getDpVec();
                }
            }
        }
    }
    return nullptr;
}

/*
*
*/
DatapointValue* DatapointUtility::findValueElement(Datapoints* dict, const string& key) {
    if (dict != nullptr) {
        for (Datapoint* dp : *dict) {
            if (dp->getName() == key) {
                return &dp->getData();
            }
        }
    }
    return nullptr;
}

/*
*
*/
Datapoint * DatapointUtility::findDatapointElement(Datapoints* dict, const string & key) {
    if (dict != nullptr) {
        for (Datapoint* dp : *dict) {
            if (dp->getName() == key) {
                return dp;
            }
        }
    }
    return nullptr;
}

/*
*
*/
string DatapointUtility::findStringElement(Datapoints* dict, const string& key) {
    if (dict != nullptr) {
        for (Datapoint* dp : *dict) {
            if (dp->getName() == key) {
                DatapointValue& data = dp->getData();
                const DatapointValue::dataTagType dType(data.getType());
                if (dType == DatapointValue::T_STRING) {
                    return data.toString();
                }
            }
        }
    }
    return "";
}

/**
 * Generate default attribute string on Datapoint
 * 
 * @param dps dict of values 
 * @param key key of dict
 * @param valueDefault value attribute of dict
 * @return pointer of the created datapoint
 */
Datapoint * DatapointUtility::createStringElement(Datapoints * dps, const string & key, const string & valueDefault) {
    DatapointValue dv(valueDefault);
    Datapoint * dp = new Datapoint(key, dv);
    dps->push_back(dp);

    return dp;
}

/**
 * Generate default attribute dict on Datapoint
 * 
 * @param dps dict of values 
 * @param key key of dict
 * @return pointer of the created datapoint
 */
Datapoint * DatapointUtility::createDictElement(Datapoints * dps, const string & key) {
    Datapoints * newVec = new Datapoints;
	DatapointValue dv(newVec, true);
    Datapoint * dp = new Datapoint(key, dv);
    dps->push_back(dp);

    return dp;
}