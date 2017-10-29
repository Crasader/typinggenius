//
//  KeyboardTests.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 8/22/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include "KeyboardModel.h"
#include <gtest/gtest.h>
#include "DefaultKeyboardConfiguration.h"
// #include "KeyboardConfigurationManager.h"

namespace ac {
	
	using std::cout;
	using std::endl;
	using ::testing::Test;
	
	class KeyboardTest : public Test
	{
	protected:
		KeyboardTest() {
			keyboardConfig.reset(new DefaultKeyboardConfiguration());
			KeyboardConfigurationManager::initializeKeyboardConfiguration(keyboardConfig);
			keyboard.reset(new KeyboardModel(*keyboardConfig));
		}
		
		virtual ~KeyboardTest() {
			keyboard = nullptr;
			keyboardConfig = nullptr;
		}
		
		virtual void SetUp() {}
		virtual void TearDown() {}
		
	public:
		shared_ptr<KeyboardConfiguration> keyboardConfig;
		shared_ptr<KeyboardModel> keyboard;
	};
	

	TEST_F(KeyboardTest, CallingKeyPressMethodsResultsInKeyStateUpdate) {
		string label = "key:q"; // should be in there!
		keyboard->keyPressed(label);
		KeyModel &key = keyboard->getConfiguration().getKeyWithLabel(label);
		EXPECT_EQ(KeyPressState::Down, key.getPressedState());
		
		keyboard->keyReleased(label); // same key released
		EXPECT_EQ(KeyPressState::Up, key.getPressedState());
	}
}