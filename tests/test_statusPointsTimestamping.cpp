#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filterStatusPointsTimestamping.h>
#include <jsonToDatapoints.h>
#include <constantsSpTimestamping.h>

using namespace std;
using namespace DatapointUtility;
using namespace JsonToDatapoints;

static string nameReading = "data_test";

static string jsonMessageWithoutSecondSinceEpoch = QUOTE({
	"PIVOT" : { 
		"GTIS": {
			"SpsTyp": {
			  "q": {
				"Source": "process",
				"Validity": "good"
			  },
			  "t": {
				"FractionOfSecond": 1
			  },
			  "mag": {
				"f": 0
			  }
			},
			"Identifier": "ID1"
		}
	}
});

static string jsonMessageWithoutDictTs = QUOTE({
	"PIVOT" : { 
		"GTIS": {
			"SpsTyp": {
			  "q": {
				"Source": "process",
				"Validity": "good"
			  },
			  "mag": {
				"f": 0
			  }
			},
			"Identifier": "ID1"
		}
	}
});

static string jsonMessageSpsTyp = QUOTE({
	"PIVOT" : { 
		"GTIS": {
			"SpsTyp": {
			  "q": {
				"Source": "process",
				"Validity": "good"
			  },
			  "t": {
				"FractionOfSecond": 1,
				"SecondSinceEpoch": 0
			  },
			  "mag": {
				"f": 0
			  }
			},
			"Identifier": "ID1"
		}
	}
});

static string jsonMessageDpsTyp = QUOTE({
	"PIVOT" : { 
		"GTIS": {
			"DpsTyp": {
			  "q": {
				"Source": "process",
				"Validity": "good"
			  },
			  "t": {
				"FractionOfSecond": 1,
				"SecondSinceEpoch": 0
			  },
			  "mag": {
				"f": 0
			  }
			},
			"Identifier": "ID1"
		}
	}
});

extern "C" {
	PLUGIN_INFORMATION *plugin_info();
	void plugin_ingest(void *handle, READINGSET *readingSet);
	PLUGIN_HANDLE plugin_init(ConfigCategory *config,
			  OUTPUT_HANDLE *outHandle,
			  OUTPUT_STREAM output);
	
	void HandlerStatusPointTimestamping(void *handle, READINGSET *readings) {
		*(READINGSET **)handle = readings;
	}
};

class StatusPointsTimestamping : public testing::Test
{
protected:
    FilterStatusPointsTimestamping *filter = nullptr;  // Object on which we call for tests
    ReadingSet *resultReading;

    // Setup is ran for every tests, so each variable are reinitialised
    void SetUp() override
    {
        PLUGIN_INFORMATION *info = plugin_info();
		ConfigCategory *config = new ConfigCategory("status-points-timestamping", info->config);
		
		ASSERT_NE(config, (ConfigCategory *)NULL);		
		config->setItemsValueFromDefault();
		config->setValue("enable", "true");
		
		void *handle = plugin_init(config, &resultReading, HandlerStatusPointTimestamping);
		filter = (FilterStatusPointsTimestamping *) handle;
    }

    // TearDown is ran for every tests, so each variable are destroyed again
    void TearDown() override
    {
        delete filter;
    }

    void startTests(string json, string typeStatusPoint) {
        ASSERT_NE(filter, (void *)NULL);

        // Create Reading
        Datapoints *p = parseJson(json);
        Reading *reading = new Reading(nameReading, *p);
        Readings *readings = new Readings;
        readings->push_back(reading);

        // Create ReadingSet
        ReadingSet *readingSet = new ReadingSet(readings);

        plugin_ingest(filter, (READINGSET*)readingSet);
        Readings results = resultReading->getAllReadings();
        ASSERT_EQ(results.size(), 1);

        Reading *out = results[0];
        ASSERT_STREQ(out->getAssetName().c_str(), nameReading.c_str());
        ASSERT_EQ(out->getDatapointCount(), 1);
        
        Datapoints points = out->getReadingData();
        ASSERT_EQ(points.size(), 1);

        verifyDatapoint(&points, typeStatusPoint);

        delete reading;
    }

    void verifyDatapoint(Datapoints * dps, string nameTyp) {
        Datapoints *dpsPivot = findDictElement(dps, ConstantsSpTimestamping::KeyMessagePivotJsonRoot);
        ASSERT_NE(dpsPivot, nullptr);

        Datapoints *dpsGi = findDictElement(dpsPivot, ConstantsSpTimestamping::KeyMessagePivotJsonGt);
        ASSERT_NE(dpsGi, nullptr);

        Datapoints *dpsTyp = findDictElement(dpsGi, nameTyp);
        ASSERT_NE(dpsTyp, nullptr);

        Datapoints *dpsT = findDictElement(dpsTyp, ConstantsSpTimestamping::KeyMessagePivotJsonTS);
        ASSERT_NE(dpsT, nullptr);

        // Vérification SecondSinceEpoch
        DatapointValue *valueSecondSinceEpoch = findValueElement(dpsT, ConstantsSpTimestamping::KeyMessagePivotJsonSecondSinceEpoch);
        ASSERT_NE(valueSecondSinceEpoch, nullptr);
        ASSERT_NE(valueSecondSinceEpoch->toInt(), 0);

		// Vérification of FractionOfSecond
        DatapointValue *valueFractionOfSecond = findValueElement(dpsT, ConstantsSpTimestamping::KeyMessagePivotJsonFractionOfSeconds);
        ASSERT_NE(valueFractionOfSecond, nullptr);
        ASSERT_NE(valueFractionOfSecond->toInt(), 0);

		// Vérification of PIVOTTS.GTIS.TmOrg.stVal
        Datapoints *tm_org = findDictElement(dpsGi, ConstantsSpTimestamping::KeyMessagePivotJsonTmOrg);
        ASSERT_NE(tm_org, nullptr);

		DatapointValue *tm_org_st_val = findValueElement(tm_org, ConstantsSpTimestamping::KeyMessagePivotJsonStVal);
        ASSERT_NE(tm_org_st_val, nullptr);

        ASSERT_STREQ(tm_org_st_val->toStringValue().c_str(), ConstantsSpTimestamping::ValueSubstituted.c_str());
        
		// Vérification of PIVOTTS.GTIS.TmOrg.stVal
        Datapoints *tm_validity = findDictElement(dpsGi, ConstantsSpTimestamping::KeyMessagePivotJsonTmValidity);
        ASSERT_NE(tm_validity, nullptr);

		DatapointValue *tm_validity_st_val = findValueElement(tm_validity, ConstantsSpTimestamping::KeyMessagePivotJsonStVal);
        ASSERT_NE(tm_validity_st_val, nullptr);

        ASSERT_STREQ(tm_validity_st_val->toStringValue().c_str(), ConstantsSpTimestamping::ValueValid.c_str());
    }
};

TEST_F(StatusPointsTimestamping, SpsTyp) 
{
	startTests(jsonMessageSpsTyp, ConstantsSpTimestamping::KeyMessagePivotJsonCdcSps);
}

TEST_F(StatusPointsTimestamping, DpsTyp) 
{
	startTests(jsonMessageDpsTyp, ConstantsSpTimestamping::KeyMessagePivotJsonCdcDps);
}

TEST_F(StatusPointsTimestamping, WithoutDictTS) 
{
	startTests(jsonMessageWithoutDictTs, ConstantsSpTimestamping::KeyMessagePivotJsonCdcSps);
}

TEST_F(StatusPointsTimestamping, WithoutSecondSiceEpoch) 
{
	startTests(jsonMessageWithoutSecondSinceEpoch, ConstantsSpTimestamping::KeyMessagePivotJsonCdcSps);
}