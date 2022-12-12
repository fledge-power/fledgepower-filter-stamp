/*
 * Utility.
 *
 * Copyright (c) 2020, RTE (https://www.rte-france.com)
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Yannick Marchetaux
 * 
 */
#include <utility.h>
#include <cmath>

/**
 * Convert secondSinceEpoch and secondSinceEpoch to timestamp
 * @param secondSinceEpoch : interval in seconds continuously counted from the epoch 1970-01-01 00:00:00 UTC
 * @param fractionOfSecond : represents the fraction of the current second when the value of the TimeStamp has been determined.
 * @return timestamp (ms)
*/
long Utility::toTimestamp(long secondSinceEpoch, long fractionOfSecond) {
    long timestamp = 0;
    long msPart = round((fractionOfSecond * 1000L) / 16777216.0);
    timestamp = (secondSinceEpoch * 1000L) + msPart;
    return timestamp;
}

/**
 * Convert timestamp (ms) in pair of secondSinceEpoch and fractionOfSecond
 * @param timestamp : timestamp (ms) 
 * @return pair of secondSinceEpoch and fractionOfSecond
*/
std::pair<long, long> Utility::fromTimestamp(long timestamp) {
    long remainder = (timestamp % 1000L);
    long fractionOfSecond = (remainder) * 16777 + ((remainder * 216) / 1000);
    return std::make_pair(timestamp / 1000L, fractionOfSecond);
}