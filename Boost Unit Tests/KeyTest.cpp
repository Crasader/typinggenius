//
//  KeyTest.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 9/13/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>
#include "KeyboardModel.h"
#include "ACTypes.h"
#include "Keyboard.h"
#include "KeyModel.h"
#include "TextureHelper.h"

namespace ac {
	
	struct KeyTestFixture
	{
		KeyTestFixture() {
			utilities::loadTextures();
			Keyboard::getInstance().setUp();
		}
		
		~KeyTestFixture() {
			key.reset();
			Keyboard::getInstance().tearDown();
			utilities::unloadTextures();
		}
		
		// other variables
		std::shared_ptr<KeyModel>key;
	};
	
	
	BOOST_FIXTURE_TEST_SUITE(KeyTests, KeyTestFixture)
	
	BOOST_AUTO_TEST_CASE(KeyWhenCreatedIsInStateUp) {
		key.reset(new KeyModel("", Glyph(1)));
		BOOST_REQUIRE(KeyPressState::Up == key->getPressedState());
	}
	
	
	BOOST_AUTO_TEST_CASE(KeyWhenPressedIsInStateDown) {
		key.reset(new KeyModel("", Glyph(1)));
		key->keyPressed();
		BOOST_REQUIRE(KeyPressState::Down == key->getPressedState());
	}
	
	
	BOOST_AUTO_TEST_CASE(KeyCreatedWithNoParamGetsDefaultLabel) {
		key.reset(new KeyModel());
		BOOST_REQUIRE_EQUAL(string(""), key->getLabel());
	}
	
	
	BOOST_AUTO_TEST_CASE(KeyCreatedWithCopyConstructorOrAssignmentOpWillGetSameLabel) {
		key.reset(new KeyModel("key:002", Glyph(2)));
		KeyModel anotherKey(*key);
		BOOST_REQUIRE_EQUAL(key->getLabel(), anotherKey.getLabel());
		
		KeyModel yetAnotherKey = *key;
		BOOST_REQUIRE_EQUAL(key->getLabel(), yetAnotherKey.getLabel());
	}
	
	BOOST_AUTO_TEST_SUITE_END()
}

