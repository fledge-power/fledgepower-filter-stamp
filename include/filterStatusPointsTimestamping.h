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

class FilterStatusPointsTimestamping  : public FledgeFilter
{
public:  
    FilterStatusPointsTimestamping(const std::string& filterName,
                        ConfigCategory& filterConfig,
                        OUTPUT_HANDLE *outHandle,
                        OUTPUT_STREAM output);

    void ingest(READINGSET *readingSet);
    void reconfigure(const std::string& newConfig);

private:
    std::mutex      m_configMutex;

    void createQualityTimestamp(DatapointUtility::Datapoints *dpDict);
};

#endif  // INCLUDE_FILTER_STATUS_POINTS_TIMESTAMPING_H_
