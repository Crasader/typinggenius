//
//  CopyTextLoadingTest.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 9/23/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include <boost/test/unit_test.hpp>
#include "CopyTextLoader.h"
#include "DebugSettingsHelper.h"

namespace ac {
	
	struct CopyTextLoadingFixture
	{
		CopyTextLoadingFixture() : loader(CopyTextLoader::getInstance()) {
			filename = DebugSettingsHelper::sharedHelper().
			stringValueForProperty("copy_source_file", "text/lorem-ipsum.json");
		}
		
		~CopyTextLoadingFixture() {
			
		}
		
		// other variables
		CopyTextLoader &loader;
		
		string filename;
	};
	
	
	BOOST_FIXTURE_TEST_SUITE(CopyTextLoadingTests, CopyTextLoadingFixture)
	
	
	BOOST_AUTO_TEST_CASE(CopyTextLoaderDefaultTextIsPresent)
	{
		loader.loadTextFile(filename); // use default params
		BOOST_REQUIRE_EQUAL(loader.hasError(), false);
		
		BOOST_REQUIRE_GT(loader.loadedTextFile().size(), 0);
	}
	
	
	BOOST_AUTO_TEST_CASE(CopyTextLoadedHasNonEmptyLines)
	{
		loader.loadTextFile(filename); // use default params
		const string &firstLine(loader.getLine(0));
		BOOST_REQUIRE(!firstLine.empty());
	}
	
	
	BOOST_AUTO_TEST_CASE(ShortenedCopyTextWorks)
	{
		string mary(getFirstNChars("Mary had a little lamb", 3));
		BOOST_REQUIRE_EQUAL(mary, "Mar");
	}
	
	
	BOOST_AUTO_TEST_SUITE_END()
}