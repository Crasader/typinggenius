//
//  KeyTests.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 8/22/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//


#include <gtest/gtest.h>
#include "KeyModel.h"

namespace ac {
	
	using std::cout;
	using std::endl;
	using ::testing::Test;
	
	class KeyTest : public Test
	{
	protected:
		KeyTest() {}
		virtual ~KeyTest() {}
		
		virtual void SetUp() {}
		
		virtual void TearDown() {
			key = nullptr;
		}
		
	public:
		std::shared_ptr<KeyModel>key;
	};
	
	
	TEST_F(KeyTest, KeyWhenCreatedIsInStateUp) {
		key.reset(new KeyModel("whatever"));
		EXPECT_EQ(KeyPressState::Up, key->getPressedState());
	}
	
	
	TEST_F(KeyTest, KeyWhenPressedIsInStateDown) {
		key.reset(new KeyModel("whatever"));
		key->keyPressed();
		EXPECT_EQ(KeyPressState::Down, key->getPressedState());
	}
	
	
	TEST_F(KeyTest, KeyCreatedWithNoParamGetsDefaultLabel) {
		key.reset(new KeyModel());
		EXPECT_STREQ("unbeknowst", key->getLabel().c_str());
	}
	
	
	TEST_F(KeyTest, KeyCreatedWithCopyConstructorOrAssignmentOpWillGetSameLabel) {
		key.reset(new KeyModel("key.7"));
		KeyModel anotherKey(*key);
		EXPECT_STREQ(key->getLabel().c_str(), anotherKey.getLabel().c_str());
		
		KeyModel yetAnotherKey = *key;
		EXPECT_STREQ(key->getLabel().c_str(), yetAnotherKey.getLabel().c_str());
	}
}