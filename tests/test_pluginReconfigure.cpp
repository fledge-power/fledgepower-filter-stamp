#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filterStatusPointsTimestamping.h>
#include <jsonToDatapoints.h>

using namespace std;
using namespace DatapointUtility;
using namespace JsonToDatapoints;

static string nameReading = "data_test";

static string reconfigure = QUOTE({
    "enable": {
        "value": "false"
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

extern "C" {
	PLUGIN_INFORMATION *plugin_info();
	void plugin_ingest(void *handle, READINGSET *readingSet);
	PLUGIN_HANDLE plugin_init(ConfigCategory *config,
			  OUTPUT_HANDLE *outHandle,
			  OUTPUT_STREAM output);
	
	void HandlerReconfigure(void *handle, READINGSET *readings) {
		*(READINGSET **)handle = readings;
	}

    void plugin_reconfigure(PLUGIN_HANDLE *handle, const string& newConfig);
};

class PluginReconfigure : public testing::Test
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
		
		void *handle = plugin_init(config, &resultReading, HandlerReconfigure);
		filter = (FilterStatusPointsTimestamping *) handle;
    }

    // TearDown is ran for every tests, so each variable are destroyed again
    void TearDown() override
    {
        delete filter;
    }   
};

TEST_F(PluginReconfigure, Reconfigure) 
{
	plugin_reconfigure((PLUGIN_HANDLE*)filter, reconfigure);
    ASSERT_EQ(filter->isEnabled(), false);

    // Create Reading
   	Datapoints *p = parseJson(jsonMessageSpsTyp.c_str());
	Reading *reading = new Reading(nameReading, *p);
    Readings *readings = new Readings;
    readings->push_back(reading);

    // Create ReadingSet
    ReadingSet *readingSet = new ReadingSet(readings);

	plugin_ingest(filter, (READINGSET*)readingSet);
	Readings results = resultReading->getAllReadings();
	ASSERT_EQ(results.size(), 1);

    delete reading;
}