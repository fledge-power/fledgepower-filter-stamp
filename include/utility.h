#ifndef INCLUDE_UTILITY_H_
#define INCLUDE_UTILITY_H_
/*
 * Utility
 *
 * Copyright (c) 2020, RTE (https://www.rte-france.com)
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Yannick Marchetaux
 * 
 */
#include<string>

namespace Utility {  
    // Function for search value
    long                    toTimestamp     (long secondSinceEpoch, long fractionOfSecond);
    std::pair<long, long>   fromTimestamp   (long timestamp);
};

#endif  // INCLUDE_UTILITY_H_