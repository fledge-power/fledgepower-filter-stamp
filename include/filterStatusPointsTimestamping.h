#ifndef INCLUDE_FILTER_STATUS_POINTS_TIMESTAMPING_H_
#define INCLUDE_FILTER_STATUS_POINTS_TIMESTAMPING_H_

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
#include <config_category.h>
#include <filter.h>
#include <mutex>
#include <string>

#include <datapointUtility.h>

struct Constants {
    static const std::string KEY_MESSAGE_PIVOT_JSON_ROOT;
    static const std::string KEY_MESSAGE_PIVOT_JSON_GT;
    static const std::string KEY_MESSAGE_PIVOT_JSON_CDC_SPS;
    static const std::string KEY_MESSAGE_PIVOT_JSON_CDC_DPS;
    static const std::string KEY_MESSAGE_PIVOT_JSON_TS;
    static const std::string KEY_MESSAGE_PIVOT_JSON_SECOND_SINCE_EPOCH;
    static const std::string KEY_MESSAGE_PIVOT_JSON_FRAT_OF_SECOND;
    static const std::string KEY_MESSAGE_PIVOT_JSON_TM_ORG;
    static const std::string KEY_MESSAGE_PIVOT_JSON_TM_VALIDITY;
    static const std::string KEY_MESSAGE_PIVOT_JSON_ST_VAL;
    static const std::string STR_SUBSTITUTED;
    static const std::string STR_VALID;
};

class FilterStatusPointsTimestamping  : public FledgeFilter
{
public:  
    FilterStatusPointsTimestamping(const std::string& filterName,
                        ConfigCategory& filterConfig,
                        OUTPUT_HANDLE *outHandle,
                        OUTPUT_STREAM output);

    ~FilterStatusPointsTimestamping();

    void ingest(READINGSET *readingSet);
    void reconfigure(const std::string& newConfig);

private:
    std::mutex      m_configMutex;

    void createQualityTimestamp(DatapointUtility::Datapoints *dpDict);
};

#endif  // INCLUDE_FILTER_STATUS_POINTS_TIMESTAMPING_H_
