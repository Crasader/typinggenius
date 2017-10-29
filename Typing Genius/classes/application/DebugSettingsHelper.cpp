//
//  DebugHelper.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 8/24/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include "DebugSettingsHelper.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "Utilities.h"


namespace ac {

	static const string SettingsFile = "debug-settings.json";
	
	using std::string;
	using boost::property_tree::ptree;
	using boost::property_tree::json_parser_error;


	struct DebugSettingsHelperImpl
	{
		string settingsFilename;
		ptree pt;
		bool hasError = false;

		DebugSettingsHelperImpl(const string &settingsFile) : settingsFilename(settingsFile) {

			string fullPath = utilities::getFullPathForFilename(settingsFile);

			// get ready to catch an exception if reading the file fails
			try {
				read_json(fullPath, pt);
			} catch (const json_parser_error &obj) {
				LogE << "Problem reading configuration file: " << obj.what();
				hasError = true;
				return;
			}
		}
	};

	
	bool DebugSettingsHelper::hasError() const
	{
		return pImpl->hasError;
	}
	

	DebugSettingsHelper& DebugSettingsHelper::sharedHelper()
	{
		static DebugSettingsHelper helperInstance;
		return helperInstance;
	}


	DebugSettingsHelper::DebugSettingsHelper()
	{
		pImpl.reset(new DebugSettingsHelperImpl(SettingsFile));
	}


	string DebugSettingsHelper::stringValueForProperty(const string &property,
			const string &defaultValue)
	{
		return pImpl->pt.get<string>(property, defaultValue);
	}


	int DebugSettingsHelper::intValueForProperty(const string &property,
			const int defaultValue)
	{
		return pImpl->pt.get<int>(property, defaultValue);
	}
	
	
	bool DebugSettingsHelper::boolValueForProperty(const string &property, const bool defaultValue)
	{
		return pImpl->pt.get<bool>(property);
		// return pImpl->pt.get<bool>(property, defaultValue);
	}

};