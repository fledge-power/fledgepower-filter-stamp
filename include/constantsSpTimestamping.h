#ifndef INCLUDE_CONSTANTS_SP_TIMESTAMPING_H_
#define INCLUDE_CONSTANTS_SP_TIMESTAMPING_H_

#include <string>

#define FILTER_NAME "status-points-timestamping"

struct ConstantsSpTimestamping {

    static const std::string NamePlugin;

    static const std::string KeyMessagePivotJsonRoot;
    static const std::string KeyMessagePivotJsonGt;
    static const std::string KeyMessagePivotJsonCdcSps;
    static const std::string KeyMessagePivotJsonCdcDps;
    static const std::string KeyMessagePivotJsonTS;
    static const std::string KeyMessagePivotJsonSecondSinceEpoch;
    static const std::string KeyMessagePivotJsonFractionOfSeconds;
    static const std::string KeyMessagePivotJsonTmOrg;
    static const std::string KeyMessagePivotJsonTmValidity;
    static const std::string KeyMessagePivotJsonStVal;
    static const std::string ValueSubstituted;
    static const std::string ValueValid;
};

#endif //INCLUDE_CONSTANTS_SP_TIMESTAMPING_H_