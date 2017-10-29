//
//  DebugHelper.h
//  Typing Genius
//
//  Created by Aldrich Co on 8/24/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//
//	Settings file should be in `debug-settings.json`.

#pragma once

#include <string>

namespace ac {

	using std::string;
	using std::unique_ptr;

	// fwd declare
	class DebugSettingsHelperImpl;


	class DebugSettingsHelper
	{
	public:
		static DebugSettingsHelper &sharedHelper(); // singleton

		string stringValueForProperty(const string &property, const string &defaultValue = "");

		int intValueForProperty(const string &property, const int defaultValue = 0);
		
		bool boolValueForProperty(const string &property, const bool defaultValue = false);

		/** query this to see if settings file is loaded successfully */
		bool hasError() const;
		
	private:
		DebugSettingsHelper();

		unique_ptr<DebugSettingsHelperImpl> pImpl;
	};


};