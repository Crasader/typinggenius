//
//  DebugSettingsHelperTest.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 9/13/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include <boost/test/unit_test.hpp>
#include "DebugSettingsHelper.h"

namespace ac {
		
	BOOST_AUTO_TEST_SUITE(DebugSettingsHelperTests)
	
	BOOST_AUTO_TEST_CASE(SettingsJSONIsLoaded)
	{
		BOOST_REQUIRE(!DebugSettingsHelper::sharedHelper().hasError());
	}
	
	
	BOOST_AUTO_TEST_CASE(TestKeyInSettingsJSON)
	{
		BOOST_REQUIRE_EQUAL(string("Unit Tests"),
						  DebugSettingsHelper::sharedHelper().stringValueForProperty("unit_test_custom_string"));
	}
	
	BOOST_AUTO_TEST_SUITE_END()

}