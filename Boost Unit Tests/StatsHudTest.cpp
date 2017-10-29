//
//  StatsHudTest.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 9/24/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//


#include <boost/test/unit_test.hpp>
#include "StatsHUD.h"
#include "StatsHUDView.h"
#include "VisualEffectsHelper.h"

namespace ac {
	
	using std::shared_ptr;
	
	struct StatsHudTestFixture
	{
		StatsHudTestFixture() : hud(StatsHUD::getInstance()) {
			hud.setUp();
		}
		
		~StatsHudTestFixture() {
			hud.tearDown();
		}
		
		StatsHUD &hud;
		
		// m and v are pters
		const shared_ptr<StatsHUDModel> hudModel() { return hud.model(); }
		StatsHUDView *hudView() { return hud.view(); }
		
		// let's assume that tag = line#
		inline CCLabelBMFont *getFontLabel(int tag) const
		{
			CCLabelBMFont *label = nullptr;
			CCNode *node = hud.view()->getChildByTag(tag);
			if (node) {
				label = dynamic_cast<CCLabelBMFont *>(node);
			}
			BOOST_REQUIRE(label);
			return label;
		}
	};
	
	
	BOOST_FIXTURE_TEST_SUITE(StatsHudTests, StatsHudTestFixture)

	BOOST_AUTO_TEST_CASE(StepwiseNumericValuesBasedOnRange) {

		// negatives
		vector<string> ranges = VisualEffectsHelper::stepwiseNumericValuesBasedOnRange(-10, -3, 100, "%d");
		vector<string> supposed = { "-10", "-9", "-8", "-7", "-6", "-5", "-4", "-3" };
		BOOST_REQUIRE_EQUAL_COLLECTIONS(ranges.begin(), ranges.end(), supposed.begin(), supposed.end());

		// common case
		ranges = VisualEffectsHelper::stepwiseNumericValuesBasedOnRange(20, 23, 8, "%d");
		supposed = { "20", "21", "22", "23" };
		BOOST_REQUIRE_EQUAL_COLLECTIONS(ranges.begin(), ranges.end(), supposed.begin(), supposed.end());

		// bad range
		ranges = VisualEffectsHelper::stepwiseNumericValuesBasedOnRange(0, -1, 5, "%d");
		BOOST_REQUIRE_EQUAL(ranges.size(), 0);

		ranges = VisualEffectsHelper::stepwiseNumericValuesBasedOnRange(0, 0, 5, "%d");
		supposed = { "0" };
		BOOST_REQUIRE_EQUAL_COLLECTIONS(ranges.begin(), ranges.end(), supposed.begin(), supposed.end());

		ranges = VisualEffectsHelper::stepwiseNumericValuesBasedOnRange(20, 20, 5, "%d");
		supposed = { "20" };
		BOOST_REQUIRE_EQUAL_COLLECTIONS(ranges.begin(), ranges.end(), supposed.begin(), supposed.end());

		// with format string (float)
		ranges = VisualEffectsHelper::stepwiseNumericValuesBasedOnRange(10.0, 23.0, 5, "[%.1f]");
		supposed = { "[10.0]", "[12.6]", "[15.2]", "[17.8]", "[20.4]", "[23.0]" };
		BOOST_REQUIRE_EQUAL_COLLECTIONS(ranges.begin(), ranges.end(), supposed.begin(), supposed.end());
	}
	
	BOOST_AUTO_TEST_SUITE_END()
}

