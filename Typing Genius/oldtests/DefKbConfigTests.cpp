//
//  DefaultKeyboardConfigurationTests.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 8/21/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include <gtest/gtest.h>
#include <boost/foreach.hpp>
#include <mm_malloc.h>


namespace ac {
	
	using std::cout;
	using std::endl;
	using ::testing::Test;

	class DefaultKeyboardConfigurationTest : public Test
	{
	protected:
		DefaultKeyboardConfigurationTest() : defaultKeyLabels() {

			defaultKeyLabels.push_back("key:q");
			defaultKeyLabels.push_back("key:w");
			defaultKeyLabels.push_back("key:e");
			defaultKeyLabels.push_back("key:r");
			defaultKeyLabels.push_back("key:t");

			configuration.reset(new DefaultKeyboardConfiguration());
			KeyboardConfigurationManager::initializeKeyboardConfiguration(configuration);
		}
		
		virtual ~DefaultKeyboardConfigurationTest() {
			configuration = nullptr; // releases it.
		}
		
		virtual void SetUp() {}
		virtual void TearDown() {}
	
	public:
		std::shared_ptr<DefaultKeyboardConfiguration>configuration;

		std::vector<std::string> defaultKeyLabels;
	};
	
	
	TEST_F(DefaultKeyboardConfigurationTest, DefaultKeyLabelsAreFound) {
		// true only for key:q -- key:t
		BOOST_FOREACH(const std::string &label, defaultKeyLabels) {
			EXPECT_TRUE(configuration->hasLabel(label));
		}
	}
	
	
	TEST_F(DefaultKeyboardConfigurationTest, NonZeroKeyLabelsOnStart) {
		EXPECT_GT(configuration->getKeyLabels().size(), 0);
		EXPECT_GT(configuration->numberOfKeys(), 0);
	}
	
	
	TEST_F(DefaultKeyboardConfigurationTest, PrintablesFollowKeySeparatorFormat) {
		// at least, for the first few
		vector<string> printables = configuration->getPrintable("key:q");
		EXPECT_STREQ("q", printables[0].c_str());
		EXPECT_STREQ("Q", printables[1].c_str());
		
		printables = configuration->getPrintable("key:w");
		EXPECT_STREQ("w", printables[0].c_str());
		EXPECT_STREQ("W", printables[1].c_str());
	}
	
	
	TEST_F(DefaultKeyboardConfigurationTest, AddPrintableCreatesKeyAndUpdatesInternalState) {
		
		size_t numKeyInMapOld = configuration->numberOfKeys();
		
		vector<string> printables { "1", "!" };
		configuration->addPrintable("key:1", printables);
		
		printables = configuration->getPrintable("key:1");
		EXPECT_STREQ("1", printables[0].c_str());
		EXPECT_STREQ("!", printables[1].c_str());

		// size grows by 1
		EXPECT_EQ(numKeyInMapOld + 1, configuration->numberOfKeys());
	}
	
	
	TEST_F(DefaultKeyboardConfigurationTest, getPrintableWillThrowOnNonExistentLabel) {
		string badLabel = "badLabel"; // not among those in defaultKeyLabels
		EXPECT_THROW(configuration->getKeyWithLabel(badLabel), std::out_of_range);
	}


	TEST_F(DefaultKeyboardConfigurationTest, defaultKeysHaveNonZeroSizes) {
		BOOST_FOREACH(const std::string &label, defaultKeyLabels) {
			KeySize size = configuration->getKeySize(label);
			EXPECT_GT(size.width, 0);
			EXPECT_GT(size.height, 0);
		}
	}


	// a zero position (0, 0) is possible, but not likely.
	TEST_F(DefaultKeyboardConfigurationTest, defaultKeysHaveNonZeroPositions) {
		BOOST_FOREACH(std::string &label, defaultKeyLabels) {
			KeyboardPoint point = configuration->getKeyPosition(label);
			EXPECT_GT(point.x, 0);
			EXPECT_GT(point.y, 0);
		}
	}


	TEST_F(DefaultKeyboardConfigurationTest, keyboardHasNonZeroSize) {
		KeyboardSize size = configuration->getKeyboardSize();
		EXPECT_GT(size.width, 0);
		EXPECT_GT(size.height, 0);
	}
}