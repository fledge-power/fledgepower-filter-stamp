#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filterStatusPointsTimestamping.h>
#include <jsonToDatapoints.h>
#include <constantsSpTimestamping.h>

using namespace std;
using namespace DatapointUtility;
using namespace JsonToDatapoints;

static string nameReading = "data_test";

static string jsonMessagePivotTM = QUOTE({
	"PIVOTTM" : { 
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

static string jsonMessageGiTm = QUOTE({
	"PIVOT" : { 
		"GTIM": {
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

static string jsonMessageMvTyp = QUOTE({
	"PIVOT" : { 
		"GTIS": {
			"MvTyp": {
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

static string jsonMessageWithSincediff0 = QUOTE({
	"PIVOT" : { 
		"GTIS": {
			"SpsTyp": {
			  "q": {
				"Source": "process",
				"Validity": "good"
			  },
			  "t": {
				"FractionOfSecond": 1,
				"SecondSinceEpoch": 1
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
	
	void HandlerNoModifyData(void *handle, READINGSET *readings) {
		*(READINGSET **)handle = readings;
	}
};

class NoModifyData : public testing::Test
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
		
		void *handle = plugin_init(config, &resultReading, HandlerNoModifyData);
		filter = (FilterStatusPointsTimestamping *) handle;
    }

    // TearDown is ran for every tests, so each variable are destroyed again
    void TearDown() override
    {
        delete filter;
    }
};

static double getSecondSinceEpoch(Datapoints *dps, string namePivotData, string nameGi, string nameTyp) {
	Datapoints *dpsPivot = findDictElement(dps, namePivotData);
	if (dpsPivot == nullptr) {
		return -1;
	}
	Datapoints *dpsGi = findDictElement(dpsPivot, nameGi);
	if (dpsGi == nullptr) {
		return -1;
	}
	Datapoints *dpsTyp = findDictElement(dpsGi, nameTyp);
	if (dpsTyp == nullptr) {
		return -1;
	}
	Datapoints *dpsT = findDictElement(dpsTyp, ConstantsSpTimestamping::KeyMessagePivotJsonTS);
	if (dpsT == nullptr) {
		return -1;
	}
	DatapointValue *valueSecondSinceEpoch = findValueElement(dpsT, ConstantsSpTimestamping::KeyMessagePivotJsonSecondSinceEpoch);
	if (valueSecondSinceEpoch == nullptr) {
		return -1;
	}
	return valueSecondSinceEpoch->toInt();
}

TEST_F(NoModifyData, PIVOTTM) 
{
	ASSERT_NE(filter, (void *)NULL);
	
    // Create Reading
    Datapoints *p = parseJson(jsonMessagePivotTM.c_str());
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

	double secondSinceEpoch = getSecondSinceEpoch(&points, "PIVOTTM", "GTIS", "SpsTyp");
	ASSERT_EQ(secondSinceEpoch, 0);

	delete reading;
}

TEST_F(NoModifyData, GTIM) 
{
	ASSERT_NE(filter, (void *)NULL);

    // Create Reading
   	Datapoints *p = parseJson(jsonMessageGiTm.c_str());
	Reading *reading = new Reading(nameReading, *p);
    Readings *readings = new Readings;
    readings->push_back(reading);

    // Create ReadingSet
    ReadingSet *readingSet = new ReadingSet(readings);

	plugin_ingest(filter, (READINGSET*)readingSet);
	Readings results = resultReading->getAllReadings();
	ASSERT_EQ(results.size(), 1);

	Reading *out = results[0];
	ASSERT_STREQ(out->getAssetName().c_str(), "data_test");
	ASSERT_EQ(out->getDatapointCount(), 1);
	
	Datapoints points = out->getReadingData();
	ASSERT_EQ(points.size(), 1);

	double secondSinceEpoch = getSecondSinceEpoch(&points, "PIVOT", "GTIM", "SpsTyp");
	ASSERT_EQ(secondSinceEpoch, 0);

	delete reading;
}

TEST_F(NoModifyData, WithSincediff0) 
{
	ASSERT_NE(filter, (void *)NULL);

    // Create Reading
   	Datapoints *p = parseJson(jsonMessageWithSincediff0.c_str());
	Reading *reading = new Reading(nameReading, *p);
    Readings *readings = new Readings;
    readings->push_back(reading);

    // Create ReadingSet
    ReadingSet *readingSet = new ReadingSet(readings);

	plugin_ingest(filter, (READINGSET*)readingSet);
	Readings results = resultReading->getAllReadings();
	ASSERT_EQ(results.size(), 1);

	Reading *out = results[0];
	ASSERT_STREQ(out->getAssetName().c_str(), "data_test");
	ASSERT_EQ(out->getDatapointCount(), 1);
	
	Datapoints points = out->getReadingData();
	ASSERT_EQ(points.size(), 1);

	double secondSinceEpoch = getSecondSinceEpoch(&points, "PIVOT", "GTIS", "SpsTyp");
	ASSERT_EQ(secondSinceEpoch, 1);

	delete reading;
}


TEST_F(NoModifyData, MvTyp) 
{
	ASSERT_NE(filter, (void *)NULL);

    // Create Reading
    Datapoints *p = parseJson(jsonMessageMvTyp.c_str());
	Reading *reading = new Reading(nameReading, *p);
    Readings *readings = new Readings;
    readings->push_back(reading);

    // Create ReadingSet
    ReadingSet *readingSet = new ReadingSet(readings);

	plugin_ingest(filter, (READINGSET*)readingSet);
	Readings results = resultReading->getAllReadings();
	ASSERT_EQ(results.size(), 1);

	Reading *out = results[0];
	ASSERT_STREQ(out->getAssetName().c_str(), "data_test");
	ASSERT_EQ(out->getDatapointCount(), 1);
	
	Datapoints points = out->getReadingData();
	ASSERT_EQ(points.size(), 1);

	double secondSinceEpoch = getSecondSinceEpoch(&points, "PIVOT", "GTIS", "MvTyp");
	ASSERT_EQ(secondSinceEpoch, 0);

	delete reading;
}