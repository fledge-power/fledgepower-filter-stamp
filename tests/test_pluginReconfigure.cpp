#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filterStatusPointsTimestamping.h>

using namespace std;

static string reconfigure = QUOTE({
    "enable": {
        "value": "false"
    }
});

extern "C" {
	PLUGIN_INFORMATION *plugin_info();
	PLUGIN_HANDLE plugin_init(ConfigCategory* config,
			  OUTPUT_HANDLE *outHandle,
			  OUTPUT_STREAM output);

    void plugin_reconfigure(PLUGIN_HANDLE *handle, const string& newConfig);
};

class PluginReconfigure : public testing::Test
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
		
		void *handle = plugin_init(config, &resultReading, nullptr);
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
}