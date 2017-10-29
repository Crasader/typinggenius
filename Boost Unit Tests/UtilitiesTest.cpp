//
//  CarriageTest.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 9/14/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include <boost/test/unit_test.hpp>
#include "Utilities.h"
// #include "BlockTypesetter.h"

namespace ac {
	
	using namespace utilities;
	
	struct UtilitiesTestFixture
	{
		UtilitiesTestFixture() {
		}
		~UtilitiesTestFixture() {
		}
		
		// other variables
	};
	
	// BOOST_GLOBAL_FIXTURE(UtilitiesTestFixture)
	
	BOOST_FIXTURE_TEST_SUITE(CarriageTests, UtilitiesTestFixture)
	
	BOOST_AUTO_TEST_CASE(RandomRGBByteReturnsValidRandomCCC3) {
		RGBByte rgb = randomRGBByte();
		
		BOOST_MESSAGE("RGB(" << (unsigned int)rgb.r << ", " << (unsigned int)rgb.g << ", " << (unsigned int)rgb.b << ")");
		
		BOOST_REQUIRE_GE(rgb.r, 0);
		BOOST_REQUIRE_LT(rgb.r, 256);
		
		BOOST_REQUIRE_GE(rgb.g, 0);
		BOOST_REQUIRE_LT(rgb.g, 256);
		
		BOOST_REQUIRE_GE(rgb.b, 0);
		BOOST_REQUIRE_LT(rgb.b, 256);
	}
	
	
	BOOST_AUTO_TEST_CASE(GetFullPathReturnsValidPath) {
		
		const std::string file = "debug-settings.json";
		const std::string path = getFullPathForFilename(file);
		
		BOOST_MESSAGE("The path: " << path );
		
		BOOST_MESSAGE("Looking for 'iPhone Simulator'");
		size_t result = path.find("iPhone Simulator");
		BOOST_REQUIRE_NE(result, string::npos);

		// is the original file there
		BOOST_MESSAGE("Looking for the original file name in the path");
		result = path.find(file);
		BOOST_REQUIRE_NE(result, string::npos);
	}


	BOOST_AUTO_TEST_CASE(KeyIsAModifierFunction) {
		const std::string modifierLabel = "mod:whatever";
		const std::string nonModifierLabel = "key:whatever";
		const std::string alsoNonModifierLabel = "key:mod:huh";

		BOOST_REQUIRE(keyIsAModifier(modifierLabel));
		BOOST_REQUIRE(!keyIsAModifier(nonModifierLabel));
		BOOST_REQUIRE(!keyIsAModifier(alsoNonModifierLabel));
	}

	
	BOOST_AUTO_TEST_SUITE_END()
	
	
}

