//
//  main.cpp
//  Unit Tests
//
//  Created by Aldrich Co on 7/22/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//
//	Main unit test runner with sample test fixture.
//	See https://code.google.com/p/googletest/ for usage examples and guidelines
//	For mocking, also see https://code.google.com/p/googlemock/

#include <iostream>
#include <gtest/gtest.h>
#include <string>

namespace aco {
	
	using namespace std;
		
	class Foo {
	public:
		inline int bar(int i) const { return 1 + i; }
	};
	
	// The fixture for testing class Foo. You can create more test fixture classes as needed (see ACMockSample)
	class FooTest : public ::testing::Test {
	protected:
		// You can remove any or all of the following functions if its body
		// is empty.
		
		FooTest() {
			// You can do set-up work for each test here.
		}
		
		virtual ~FooTest() {
			// You can do clean-up work that doesn't throw exceptions here.
		}
		
		// If the constructor and destructor are not enough for setting up
		// and cleaning up each test, you can define the following methods:
		
		virtual void SetUp() {
			// Code here will be called immediately after the constructor (right
			// before each test).
		}
		
		virtual void TearDown() {
			// Code here will be called immediately after each test (right
			// before the destructor).
		}
		
		// Objects declared here can be used by all tests in the test case for Foo.
	};
	
//	// Tests that the Foo::Bar() method does Abc.
//	TEST_F(FooTest, MethodBarDoesAbc) {
//		Foo f;
//		int i = 1;
//		EXPECT_EQ(i + 1, f.bar(1)) << "f.bar(i) should return i+1";
//	}
//	
//	// Tests that Foo does Xyz.
//	TEST_F(FooTest, DoesXyz) {
//		// Exercises the Xyz feature of Foo.
//	}
//	
//	
//	// Failing test 1 + 1 = 3?
//	TEST_F(FooTest, OnePlusOne) {
//		EXPECT_EQ(1 + 1, 3) << "1 + 1 = 3 (just joking!)";
//	}
	
}  // -- end namespace.


// nothing to be changed here, just keep adding tests
int main(int argc, char **argv)
{
	// when running this from a command line, it might look for the gtest framework in /Library/Frameworks.
	// you can override this by setting an environment variable: export DYLD_FRAMEWORK_PATH=$FRAMEWORKS:$DYLD_FRAMEWORK_PATH
	// where $FRAMEWORKS contain gtest.framework.
	
	// https://code.google.com/p/googletest/wiki/AdvancedGuide#Running_Test_Programs:_Advanced_Options
	// set environment variables here. Note that you can also pass command line arguments in place of runtime variables.
	
	::testing::GTEST_FLAG(output) = "xml";
	// or you could specify a hard path; default will go to:
	// <DERIVED_DATA>/Piano_Genius_XXX/Build/Products/Debug/test_detail.xml
	
	::testing::GTEST_FLAG(color) = "no"; // "auto"
	
	// debugger won't take you to the failed EXPECT_ line anyway.
	// ::testing::GTEST_FLAG(throw_on_failure) = true;
	
	
	// all settings should have been done before this line.
	::testing::InitGoogleTest(&argc, argv);
	
	// disabling this doesn't seem to cause problems.
	// ::testing::InitGoogleMock(&argc, argv);
	
	// initialize logging
	FILELog::ReportingLevel() = FILELog::FromString("WARNING");
	
	int ret = RUN_ALL_TESTS();
	
	LogI << "It is finished!";
	return ret;
}
