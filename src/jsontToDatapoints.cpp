/*
 * Json to Datapoints
 *
 * Copyright (c) 2020, RTE (https://www.rte-france.com)
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Yannick Marchetaux
 * 
 */
#include <jsonToDatapoints.h>

using namespace std;
using namespace rapidjson;
using namespace DatapointUtility; 

/**
 * Parsing a Json string
 * 
 * @param json : string json 
 * @return vector of datapoints
*/
Datapoints * JsonToDatapoints::parseJson(string json) {
	
	Document document;

    if (document.Parse(const_cast<char*>(json.c_str())).HasParseError()) {
        Logger::getLogger()->fatal("Parsing error in protocol configuration");

        printf("Parsing error in protocol configuration\n");
        return nullptr;
    }

    if (!document.IsObject()) {
        return nullptr;
    }
	return recursivJson(document);
}

/**
 * recursive method to convert a JSON string to a datapoint 
 * 
 * @param document : object rapidjon 
 * @return vector of datapoints
*/
Datapoints * JsonToDatapoints::recursivJson(const Value & document) {
	Datapoints * p = new Datapoints;

	for (Value::ConstMemberIterator itr = document.MemberBegin(); itr != document.MemberEnd(); ++itr)
	{
		Logger::getLogger()->debug("Type of member %s is %d",
			itr->name.GetString(), itr->value.GetType());
        
		if (itr->value.IsObject()) {
			Datapoints * vec = recursivJson(itr->value);
			DatapointValue d(vec, true);
			p->push_back(new Datapoint(itr->name.GetString(), d));
		}
		else if (itr->value.IsString()) {
			DatapointValue d(itr->value.GetString());
			p->push_back(new Datapoint(itr->name.GetString(), d));
		}
		else if (itr->value.IsNumber()) {
			DatapointValue d(itr->value.GetDouble());
			p->push_back(new Datapoint(itr->name.GetString(), d));
		}
	}

	return p;
}