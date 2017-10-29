//
//  DebugSettingsHelperTests.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 8/22/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//


#include <gtest/gtest.h>
#include "DebugSettingsHelper.h"

namespace ac {
	
	using std::cout;
	using std::endl;
	using ::testing::Test;
	
	class DebugSettingsHelperTest : public Test
	{
	protected:
		DebugSettingsHelperTest() {}
		virtual ~DebugSettingsHelperTest() {}
		
		virtual void SetUp() {}
		
		virtual void TearDown() {}
		
	public:
		
	};
	
	TEST_F(DebugSettingsHelperTest, SettingsJSONIsLoaded) {
		EXPECT_FALSE(DebugSettingsHelper::sharedHelper().hasError());
	}
	
	
	TEST_F(DebugSettingsHelperTest, TestKeyInSettingsJSON) {
		// This is hard coded.
		EXPECT_STREQ("Unit Tests", DebugSettingsHelper::sharedHelper().stringValueForProperty("unit_test_custom_string").c_str());
	}
	
	

}