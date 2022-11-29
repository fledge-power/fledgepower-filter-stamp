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
#include <vector>

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
                    return data.toStringValue();
                }
            }
        }
    }
    return "";
}

/**
 * Method to delete elements in a vector
 * 
 * @param dps dict of values 
 * @param key key of dict 
*/
void DatapointUtility::deleteValue(Datapoints *dps, const string & key) {
    vector<Datapoint*>::iterator it1 = dps->end();
    Datapoint * d = nullptr;
    for (vector<Datapoint*>::iterator it = dps->begin(); it != dps->end(); it++){
        if ((*it)->getName() == key) {
            it1 = it;
            d = *it;
            break;
        }
    }

    if (d != nullptr) {
        dps->erase(it1);
        delete d;
    }
}

/**
 * Generate default attribute integer on Datapoint
 * 
 * @param dps dict of values 
 * @param key key of dict
 * @param valueDefault value attribute of dict
 * @return pointer of the created datapoint
 */
Datapoint * DatapointUtility::createIntegerElement(Datapoints * dps, const string & key, long valueDefault) {

    deleteValue(dps, key);

    DatapointValue dv(valueDefault);
    Datapoint * dp = new Datapoint(key, dv);
    dps->push_back(dp);

    return dp;
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

    deleteValue(dps, key);

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

   deleteValue(dps, key);

    Datapoints * newVec = new Datapoints;
	DatapointValue dv(newVec, true);
    Datapoint * dp = new Datapoint(key, dv);
    dps->push_back(dp);

    return dp;
}