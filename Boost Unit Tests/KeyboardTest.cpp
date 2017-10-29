//
//  FixturesSample.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 9/13/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>
#include "KeyboardModel.h"
#include "Keyboard.h"
#include "ACTypes.h"
#include "ScreenResolutionHelper.h"
#include "TextureHelper.h"
#include "KeyboardView.h"

namespace ac {
	
	struct KeyboardModelFixture
	{
		KeyboardModelFixture() {
			utilities::loadTextures();
			Keyboard::getInstance().setUp();
			
			// these should all be present
			defaultKeyLabels.push_back("key:001");
			defaultKeyLabels.push_back("key:002");
			defaultKeyLabels.push_back("key:003");
			defaultKeyLabels.push_back("key:004");
			defaultKeyLabels.push_back("key:005");
		}
		
		~KeyboardModelFixture() {
			Keyboard::getInstance().tearDown();
			utilities::unloadTextures();
		}
		
		// other variables
		std::vector<std::string> defaultKeyLabels;
		
		
	};
	
	
	BOOST_FIXTURE_TEST_SUITE(KeyboardModelTests, KeyboardModelFixture)
	
	BOOST_AUTO_TEST_CASE(DefaultKeyLabelsAreFound)
	{
		BOOST_FOREACH(const std::string &label, defaultKeyLabels) {
			BOOST_REQUIRE(Keyboard::getInstance().model()->hasLabel(label));
		}
	}
	
	
	BOOST_AUTO_TEST_CASE(NonZeroKeyLabelsOnStart)
	{
		BOOST_REQUIRE_GT(Keyboard::getInstance().model()->getKeyLabels().size(), 0);
		BOOST_REQUIRE_GT(Keyboard::getInstance().model()->numberOfKeys(), 0);
	}
	
	
//	BOOST_AUTO_TEST_CASE(PrintablesFollowKeySeparatorFormat) {
//		// at least, for the first few
//		vector<string> printables = Keyboard::getInstance().model()->getPrintable("key:q");
//		BOOST_REQUIRE(string("q") == printables[0]);
//		BOOST_REQUIRE(string("Q") == printables[1]);
//	
//		printables = Keyboard::getInstance().model()->getPrintable("key:w");
//		BOOST_REQUIRE(string("w") == printables[0]);
//		BOOST_REQUIRE(string("W") == printables[1]);
//	}

	
//	BOOST_AUTO_TEST_CASE(AddPrintableCreatesKeyAndUpdatesInternalState) {
//		
//		size_t numKeyInMapOld = Keyboard::getInstance().model()->numberOfKeys();
//		
//		vector<string> printables { "1", "!" };
//		Keyboard::getInstance().model()->addPrintable("key:1", printables);
//
//		printables = Keyboard::getInstance().model()->getPrintable("key:1");
//		BOOST_REQUIRE_EQUAL(string("1"), printables[0]);
//		BOOST_REQUIRE_EQUAL(string("!"), printables[1]);
//
//		// size grows by 1
//		BOOST_REQUIRE_EQUAL(numKeyInMapOld + 1, Keyboard::getInstance().model()->numberOfKeys());
//	}

	
//	BOOST_AUTO_TEST_CASE(getPrintableWillThrowOnNonExistentLabel) {
//		string badLabel = "badLabel"; // not among those in defaultKeyLabels
//		BOOST_REQUIRE_THROW(Keyboard::getInstance().model()->getKeyWithLabel(badLabel), std::out_of_range);
//	}

	
	BOOST_AUTO_TEST_CASE(defaultKeysHaveNonZeroSizes) {
		BOOST_FOREACH(const std::string &label, defaultKeyLabels) {
			KeySize size = Keyboard::getInstance().model()->getKeySize(label);
			BOOST_REQUIRE_GT(size.width, 0);
			BOOST_REQUIRE_GT(size.height, 0);
		}
	}
	
	
	BOOST_AUTO_TEST_CASE(defaultKeysHaveNonZeroPositions) {
		BOOST_FOREACH(std::string &label, defaultKeyLabels) {
			KeyboardPoint point = Keyboard::getInstance().model()->getKeyPosition(label);
			BOOST_REQUIRE_GT(point.x, 0);
			BOOST_REQUIRE_GT(point.y, 0);
		}
	}
	
	
	BOOST_AUTO_TEST_CASE(keyboardHasNonZeroSize) {
		KeyboardSize size = Keyboard::getInstance().model()->getKeyboardSize();
		BOOST_REQUIRE_GT(size.width, 0);
		BOOST_REQUIRE_GT(size.height, 0);
	}
	


	// This is no longer the case as of 2013-10-31. KbView will update model through signal
	// but not the other way around
	//
//	BOOST_AUTO_TEST_CASE(CallingKeyPressMethodsResultsInKeyStateUpdate) {
//
//		CCTouch *touch = new CCTouch();
//
//		string label = "key:q"; // should be in there!
//		TouchType state = TouchType::TouchBegan;
//
//		BOOST_TEST_MESSAGE("Simulating keypress event");
//
//		Keyboard::getInstance().model()->keyTouchEvent(label, touch, state);
//				
//		KeyModel &key = Keyboard::getInstance().model()->getKeyWithLabel(label);
//		BOOST_REQUIRE(KeyPressState::Down == key.getPressedState());
//		
//		BOOST_TEST_MESSAGE("Simulating key release event");
//		state = TouchType::TouchEnded;
//		Keyboard::getInstance().model()->keyTouchEvent(label, touch, state); // same key released
//
//		CC_SAFE_DELETE(touch);
//
//		BOOST_REQUIRE(KeyPressState::Up == key.getPressedState());
//	}

	
	BOOST_AUTO_TEST_CASE(KeyboardIsHorizontallyCentered)
	{
		KeyboardView *kbView(Keyboard::getInstance().view());
		
		const CCPoint &anchorPt(kbView->getAnchorPoint());
		
		BOOST_REQUIRE_EQUAL(anchorPt.x, 0.5);
		BOOST_REQUIRE_EQUAL(anchorPt.y, 0);
		
		const CCPoint &kbPosition(kbView->getPosition());
		const CCSize &sz(utilities::visibleSize());
		BOOST_REQUIRE_EQUAL(kbView->getScale(), 1);
	
		// position.x + 1/2 of width = midpoint of the screen, but note that since anchorPoint.x is 0.5
		BOOST_REQUIRE_EQUAL(kbPosition.x, sz.width / 2);
	}
		
	BOOST_AUTO_TEST_SUITE_END()
}