#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filterStatusPointsTimestamping.h>

using namespace std;

extern "C" {
	PLUGIN_INFORMATION *plugin_info();
	PLUGIN_HANDLE plugin_init(ConfigCategory* config,
			  OUTPUT_HANDLE *outHandle,
			  OUTPUT_STREAM output);

    void plugin_shutdown(PLUGIN_HANDLE *handle);
};

class PluginShutdown : public testing::Test
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
		
		void *handle = plugin_init(config, &resultReading, nullptr);
		filter = (FilterStatusPointsTimestamping *) handle;
    }
};

TEST_F(PluginShutdown, Shutdown) 
{
	ASSERT_NO_THROW(plugin_shutdown((PLUGIN_HANDLE*)filter));
}