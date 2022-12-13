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
#include <utilityPivot.h>
#include <constantsSpTimestamping.h>

using namespace std;
using namespace DatapointUtility;

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
 * The actual filtering code
 *
 * @param readingSet The reading data to filter
 */
void FilterStatusPointsTimestamping::ingest(READINGSET *readingSet) 
{
    lock_guard<mutex> guard(m_configMutex);
	
    // Filter enable, process the readings 
    if (!isEnabled()) {
        (*m_func)(m_data, readingSet); 
        return;
    }
        
    // Just get all the readings in the readingset
    const Readings& readings = readingSet->getAllReadings();
    for (auto reading = readings.cbegin(); reading != readings.cend(); reading++) {
        
        // Get datapoints on readings
        Datapoints& dataPoints = (*reading)->getReadingData();
        string assetName = (*reading)->getAssetName();

        string beforeLog = ConstantsSpTimestamping::NamePlugin + " - " + assetName + " - ingest : ";

        Datapoints *dpPivotTS = findDictElement(&dataPoints, ConstantsSpTimestamping::KeyMessagePivotJsonRoot);
        if (dpPivotTS == nullptr) {
            Logger::getLogger()->debug("%s Missing %s attribute, it is ignored", beforeLog.c_str(), ConstantsSpTimestamping::KeyMessagePivotJsonRoot.c_str());
            continue;
        }

        Datapoints *dpGiTs = findDictElement(dpPivotTS, ConstantsSpTimestamping::KeyMessagePivotJsonGt);
        if (dpGiTs == nullptr) {
            Logger::getLogger()->debug("%s Missing %s attribute, it is ignored", beforeLog.c_str(), ConstantsSpTimestamping::KeyMessagePivotJsonGt.c_str());
            continue;
        }

        Datapoints *dpTyp = findDictElement(dpGiTs, ConstantsSpTimestamping::KeyMessagePivotJsonCdcSps);
        if (dpTyp == nullptr) {
            dpTyp = findDictElement(dpGiTs, ConstantsSpTimestamping::KeyMessagePivotJsonCdcDps);
            
            if (dpTyp == nullptr) {
                Logger::getLogger()->debug("%s Missing CDC (%s and %s missing) attribute, it is ignored", beforeLog.c_str(), ConstantsSpTimestamping::KeyMessagePivotJsonCdcSps.c_str(), ConstantsSpTimestamping::KeyMessagePivotJsonCdcDps.c_str());
                continue;
            }
        }

        Datapoints *dpTimestamp = findDictElement(dpTyp, ConstantsSpTimestamping::KeyMessagePivotJsonTS);
        if (dpTimestamp == nullptr) {

            // Create timestamp dict
            auto vecT = new Datapoints;
            
            DatapointValue dvSecond((long)0);
            vecT->push_back(new Datapoint(ConstantsSpTimestamping::KeyMessagePivotJsonSecondSinceEpoch, dvSecond));

            DatapointValue dvTimestamp(vecT, true);
            auto dpT = new Datapoint(ConstantsSpTimestamping::KeyMessagePivotJsonTS, dvTimestamp);
            dpTyp->push_back(dpT);

            dpTimestamp = dpT->getData().getDpVec();
        }

        DatapointValue *valueSecondSinceEpoch = findValueElement(dpTimestamp, ConstantsSpTimestamping::KeyMessagePivotJsonSecondSinceEpoch);
        if (valueSecondSinceEpoch == nullptr) {
            
            // Create value SecondSinceEpoch
            DatapointValue dvSecond((long)0);
            auto dpSecondSinceEpoch = new Datapoint(ConstantsSpTimestamping::KeyMessagePivotJsonSecondSinceEpoch, dvSecond);
            dpTimestamp->push_back(dpSecondSinceEpoch);

            valueSecondSinceEpoch = &(dpSecondSinceEpoch->getData());
        }

        long secondSinceEpoch = valueSecondSinceEpoch->toInt();
        
        if (secondSinceEpoch != 0) {
            continue;
        }

        // Generate timestamp for status points
        struct timeval timestamp;
        gettimeofday(&timestamp, nullptr);
        long tsMs = timestamp.tv_sec + (timestamp.tv_usec / 1000L);

        DatapointValue *valueFractionOfSecond = findValueElement(dpTimestamp, ConstantsSpTimestamping::KeyMessagePivotJsonFractionOfSeconds);
        if (valueFractionOfSecond == nullptr) {
            DatapointValue dvFrac((long)0);
            auto dpFractionOfSecond = new Datapoint(ConstantsSpTimestamping::KeyMessagePivotJsonFractionOfSeconds, dvFrac);
            dpTimestamp->push_back(dpFractionOfSecond);

            valueFractionOfSecond = &(dpFractionOfSecond->getData());
        }

        pair<long, long> tsCalc = UtilityPivot::fromTimestamp(tsMs);
        valueSecondSinceEpoch->setValue(tsCalc.first);
        valueFractionOfSecond->setValue(tsCalc.second);

        Logger::getLogger()->debug("%s Status point timestamping", beforeLog.c_str());

        // Generate quality of timestamp on Gatewway
        this->createQualityTimestamp(dpGiTs);
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
void FilterStatusPointsTimestamping::createQualityTimestamp(Datapoints *dpDict) {
    Datapoint *dpTmOrg = createDictElement(dpDict, ConstantsSpTimestamping::KeyMessagePivotJsonTmOrg);
    createStringElement(dpTmOrg->getData().getDpVec(), ConstantsSpTimestamping::KeyMessagePivotJsonStVal, ConstantsSpTimestamping::ValueSubstituted);
    
    Datapoint *dpTmValidity = createDictElement(dpDict, ConstantsSpTimestamping::KeyMessagePivotJsonTmValidity);
    createStringElement(dpTmValidity->getData().getDpVec(), ConstantsSpTimestamping::KeyMessagePivotJsonStVal, ConstantsSpTimestamping::ValueValid);
}