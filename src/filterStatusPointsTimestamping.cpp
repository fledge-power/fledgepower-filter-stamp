/*
 * Fledge filter status points timestamping.
 *
 * Copyright (c) 2020, RTE (https://www.rte-france.com)
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Yannick Marchetaux
 * 
 */
#include <filterStatusPointsTimestamping.h>

using namespace std;
using namespace DatapointUtility;

const string Constants::KEY_MESSAGE_PIVOT_JSON_ROOT                 = "PIVOTTS";
const string Constants::KEY_MESSAGE_PIVOT_JSON_GT                   = "GTIS";
const string Constants::KEY_MESSAGE_PIVOT_JSON_CDC_SPS              = "SPSTyp";
const string Constants::KEY_MESSAGE_PIVOT_JSON_CDC_DPS              = "DPSTyp";
const string Constants::KEY_MESSAGE_PIVOT_JSON_TS                   = "t";
const string Constants::KEY_MESSAGE_PIVOT_JSON_SECOND_SINCE_EPOCH   = "SecondSinceEpoch";
const string Constants::KEY_MESSAGE_PIVOT_JSON_TM_ORG               = "TmOrg";
const string Constants::KEY_MESSAGE_PIVOT_JSON_TM_VALIDITY          = "TmValidity";
const string Constants::KEY_MESSAGE_PIVOT_JSON_ST_VAL               = "stVal";
const string Constants::STR_SUBSTITUTED                             = "substituted";
const string Constants::STR_VALID                                   = "valid";

/**
 * Constructor for the LogFilter.
 *
 * We call the constructor of the base class and handle the initial
 * configuration of the filter.
 *
 * @param    filterName      The name of the filter
 * @param    filterConfig    The configuration category for this filter
 * @param    outHandle       The handle of the next filter in the chain
 * @param    output          A function pointer to call to output data to the next filter
 */
FilterStatusPointsTimestamping::FilterStatusPointsTimestamping(const std::string& filterName,
                        ConfigCategory& filterConfig,
                        OUTPUT_HANDLE *outHandle,
                        OUTPUT_STREAM output) :
                                FledgeFilter(filterName, filterConfig, outHandle, output)
{
}

/**
 * Destructor for this filter class
 */
FilterStatusPointsTimestamping::~FilterStatusPointsTimestamping() {}

/**
 * The actual filtering code
 *
 * @param readingSet The reading data to filter
 */
void FilterStatusPointsTimestamping::ingest(READINGSET *readingSet) 
{
    lock_guard<mutex> guard(m_configMutex);
	
    // Filter enable, process the readings 
    if (isEnabled()) {
        // Just get all the readings in the readingset
        const Readings & readings = readingSet->getAllReadings();
        for (auto reading = readings.cbegin(); reading != readings.cend(); reading++) {
            
            // Get datapoints on readings
            Datapoints & dataPoints = (*reading)->getReadingData();
            string assetName = (*reading)->getAssetName();

            Datapoints * dpPivotTS = findDictElement(&dataPoints, Constants::KEY_MESSAGE_PIVOT_JSON_ROOT);
            if (dpPivotTS == nullptr) {
                continue;
            }

            Datapoints * dpGiTs = findDictElement(dpPivotTS, Constants::KEY_MESSAGE_PIVOT_JSON_GT);
            if (dpGiTs == nullptr) {
                continue;
            }

            Datapoints * dpTyp = findDictElement(dpGiTs, Constants::KEY_MESSAGE_PIVOT_JSON_CDC_SPS);
            if (dpTyp == nullptr) {
                dpTyp = findDictElement(dpGiTs, Constants::KEY_MESSAGE_PIVOT_JSON_CDC_DPS);
                
                if (dpTyp == nullptr) {
                    continue;
                }
            }

            Datapoints * dpTimestamp = findDictElement(dpTyp, Constants::KEY_MESSAGE_PIVOT_JSON_TS);
            if (dpTimestamp == nullptr) {
                // Create timestamp dict
                Datapoints * vecT = new Datapoints;
                
                DatapointValue dvSecond((long)0);
                vecT->push_back(new Datapoint(Constants::KEY_MESSAGE_PIVOT_JSON_SECOND_SINCE_EPOCH, dvSecond));

                DatapointValue dvTimestamp(vecT, true);
                Datapoint * dpT = new Datapoint(Constants::KEY_MESSAGE_PIVOT_JSON_TS, dvTimestamp);
                dpTyp->push_back(dpT);

                dpTimestamp = dpT->getData().getDpVec();
            }

            DatapointValue * valueSecondSinceEpoch = findValueElement(dpTimestamp, Constants::KEY_MESSAGE_PIVOT_JSON_SECOND_SINCE_EPOCH);
            if (valueSecondSinceEpoch == nullptr) {
                // Create value SecondSinceEpoch
                DatapointValue dvSecond((long)0);
                Datapoint * dpSecondSinceEpoch = new Datapoint(Constants::KEY_MESSAGE_PIVOT_JSON_SECOND_SINCE_EPOCH, dvSecond);
                dpTimestamp->push_back(dpSecondSinceEpoch);

                valueSecondSinceEpoch = &(dpSecondSinceEpoch->getData());
            }

            long secondSinceEpoch = valueSecondSinceEpoch->toInt();
            if (secondSinceEpoch == 0) {
                // Generate timestamp for status points
                struct timeval timestamp;
                gettimeofday(&timestamp, NULL);				
                secondSinceEpoch = *((int64_t*)&timestamp.tv_sec);
                valueSecondSinceEpoch->setValue(secondSinceEpoch);
        
                // Generate quality of timestamp on Gatewway
                this->createQualityTimestamp(dpGiTs);
            }
        }
    }
    (*m_func)(m_data, readingSet);    
}

/**
 * Reconfiguration entry point to the filter.
 *
 * This method runs holding the configMutex to prevent
 * ingest using the regex class that may be destroyed by this
 * call.
 *
 * Pass the configuration to the base FilterPlugin class and
 * then call the private method to handle the filter specific
 * configuration.
 *
 * @param newConfig  The JSON of the new configuration
 */
void FilterStatusPointsTimestamping::reconfigure(const std::string& newConfig) {
        lock_guard<mutex> guard(m_configMutex);
        setConfig(newConfig);
}

/*
 * Create dict value of timestamp quality 
 * Timestamp quality is set to valid
*/
void FilterStatusPointsTimestamping::createQualityTimestamp(Datapoints * dpDict) {
    Datapoint * dpTmOrg = createDictElement(dpDict, Constants::KEY_MESSAGE_PIVOT_JSON_TM_ORG);
    createStringElement(dpTmOrg->getData().getDpVec(), Constants::KEY_MESSAGE_PIVOT_JSON_ST_VAL, Constants::STR_SUBSTITUTED);
    
    Datapoint * dpTmValidity = createDictElement(dpDict, Constants::KEY_MESSAGE_PIVOT_JSON_TM_VALIDITY);
    createStringElement(dpTmValidity->getData().getDpVec(), Constants::KEY_MESSAGE_PIVOT_JSON_ST_VAL, Constants::STR_VALID);
}