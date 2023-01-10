#ifndef INCLUDE_CONSTANTS_SP_TIMESTAMPING_H_
#define INCLUDE_CONSTANTS_SP_TIMESTAMPING_H_

#include <string>

#define FILTER_NAME "status-points-timestamping"

namespace ConstantsSpTimestamping {

    static const std::string NamePlugin             = FILTER_NAME;
    
    static const std::string KeyMessagePivotJsonRoot                   = "PIVOT";
    static const std::string KeyMessagePivotJsonGt                     = "GTIS";
    static const std::string KeyMessagePivotJsonCdcSps                 = "SpsTyp";
    static const std::string KeyMessagePivotJsonCdcDps                 = "DpsTyp";
    static const std::string KeyMessagePivotJsonTS                     = "t";
    static const std::string KeyMessagePivotJsonSecondSinceEpoch       = "SecondSinceEpoch";
    static const std::string KeyMessagePivotJsonFractionOfSeconds      = "FractionOfSecond";
    static const std::string KeyMessagePivotJsonTmOrg                  = "TmOrg";
    static const std::string KeyMessagePivotJsonTmValidity             = "TmValidity";
    static const std::string KeyMessagePivotJsonStVal                  = "stVal";
    static const std::string ValueSubstituted                          = "substituted";
    static const std::string ValueValid                                = "valid";
};

#endif //INCLUDE_CONSTANTS_SP_TIMESTAMPING_H_