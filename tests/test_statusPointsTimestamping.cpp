#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filterStatusPointsTimestamping.h>
#include <jsonToDatapoints.h>

using namespace std;
using namespace DatapointUtility;
using namespace JsonToDatapoints;

static string nameReading = "data_test";

static string jsonMessageWithoutSecondSinceEpoch = QUOTE({
	"PIVOTTS" : { 
		"GTIS": {
			"SPSTyp": {
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
	"PIVOTTS" : { 
		"GTIS": {
			"SPSTyp": {
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
	"PIVOTTS" : { 
		"GTIS": {
			"SPSTyp": {
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
	"PIVOTTS" : { 
		"GTIS": {
			"DPSTyp": {
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
	PLUGIN_HANDLE plugin_init(ConfigCategory* config,
			  OUTPUT_HANDLE *outHandle,
			  OUTPUT_STREAM output);
	
	void HandlerStatusPointTimestamping(void *handle, READINGSET *readings) {
		*(READINGSET **)handle = readings;
	}
};

class StatusPointsTimestamping : public testing::Test
{
protected:
    FilterStatusPointsTimestamping * filter = nullptr;  // Object on which we call for tests
    ReadingSet * resultReading;

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
        Datapoints * p = parseJson(json);
        Reading* reading = new Reading(nameReading, *p);
        Readings *readings = new Readings;
        readings->push_back(reading);

        // Create ReadingSet
        ReadingSet * readingSet = new ReadingSet(readings);

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
        Datapoints * dpsPivot = findDictElement(dps, Constants::KEY_MESSAGE_PIVOT_JSON_ROOT);
        ASSERT_NE(dpsPivot, nullptr);

        Datapoints * dpsGi = findDictElement(dpsPivot, Constants::KEY_MESSAGE_PIVOT_JSON_GT);
        ASSERT_NE(dpsGi, nullptr);

        Datapoints * dpsTyp = findDictElement(dpsGi, nameTyp);
        ASSERT_NE(dpsTyp, nullptr);

        Datapoints * dpsT = findDictElement(dpsTyp, Constants::KEY_MESSAGE_PIVOT_JSON_TS);
        ASSERT_NE(dpsT, nullptr);

        // Vérification SecondSinceEpoch
        DatapointValue * valueSecondSinceEpoch = findValueElement(dpsT, Constants::KEY_MESSAGE_PIVOT_JSON_SECOND_SINCE_EPOCH);
        ASSERT_NE(valueSecondSinceEpoch, nullptr);
        ASSERT_NE(valueSecondSinceEpoch->toInt(), 0);

		// Vérification of PIVOTTS.GTIS.TmOrg.stVal
        Datapoints * tm_org = findDictElement(dpsGi, Constants::KEY_MESSAGE_PIVOT_JSON_TM_ORG);
        ASSERT_NE(tm_org, nullptr);

		DatapointValue * tm_org_st_val = findValueElement(tm_org, Constants::KEY_MESSAGE_PIVOT_JSON_ST_VAL);
        ASSERT_NE(tm_org_st_val, nullptr);

        ASSERT_STREQ(tm_org_st_val->toStringValue().c_str(), Constants::STR_SUBSTITUTED.c_str());
        
		// Vérification of PIVOTTS.GTIS.TmOrg.stVal
        Datapoints * tm_validity = findDictElement(dpsGi, Constants::KEY_MESSAGE_PIVOT_JSON_TM_VALIDITY);
        ASSERT_NE(tm_validity, nullptr);

		DatapointValue * tm_validity_st_val = findValueElement(tm_validity, Constants::KEY_MESSAGE_PIVOT_JSON_ST_VAL);
        ASSERT_NE(tm_validity_st_val, nullptr);

        ASSERT_STREQ(tm_validity_st_val->toStringValue().c_str(), Constants::STR_VALID.c_str());
    }
};

TEST_F(StatusPointsTimestamping, SpsTyp) 
{
	startTests(jsonMessageSpsTyp, Constants::KEY_MESSAGE_PIVOT_JSON_CDC_SPS);
}

TEST_F(StatusPointsTimestamping, DpsTyp) 
{
	startTests(jsonMessageDpsTyp, Constants::KEY_MESSAGE_PIVOT_JSON_CDC_DPS);
}

TEST_F(StatusPointsTimestamping, WithoutDictTS) 
{
	startTests(jsonMessageWithoutDictTs, Constants::KEY_MESSAGE_PIVOT_JSON_CDC_SPS);
}

TEST_F(StatusPointsTimestamping, WithoutSecondSiceEpoch) 
{
	startTests(jsonMessageWithoutSecondSinceEpoch, Constants::KEY_MESSAGE_PIVOT_JSON_CDC_SPS);
}