/*
 * defaultkeyboardconfiguration.h
 * Typing Genius
 *
 * Created by Aldrich Co on 8/16/13.
 * Copyright (c) 2013 Aldrich Co. All rights reserved.
 *
 * @brief a special kind of keyboard config manager that provides sensible defaults.
 * Has methods to parse the associated JSON file and initialize the Keyboard.
 */

#pragma once

#include "Keyboard.h"
#include "BoostPTreeHelper.h"


namespace ac
{
	class Keyboard;
	class KeyboardConfiguration;

	class RowInfo;
	typedef std::map<string, RowInfo> RowMap;

	using utilities::PropTree;

	class DefaultKeyboardConfigurationImpl;

	const std::string ConfigurationFileName = "keyboard/default-keyboard-configuration.json";

	class DefaultKeyboardConfiguration : public KeyboardConfiguration
	{
	public:

		virtual bool initialize(Keyboard &keyboard);
		virtual const std::string configFileName() const { return ConfigurationFileName; }

	private:
		void initializeKeys(Keyboard &keyboard, const RowMap &rowMap, PropTree &pt);
		void initializeOtherKeys(Keyboard &keyboard, PropTree &pt);

		void initializeKeyColors(Keyboard &keyboard);
		// std::map<string, string> keysToRow; // rows are row_0, row_1, etc

		std::map<string, std::vector<string>> keysInRows;
	};
}

