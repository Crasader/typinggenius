//
//  MockSample.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 7/23/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//
//	An example using Google Mocks


#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>

using namespace std;

namespace ac
{
	// an example of a collaborator class
	class Sample
	{
	public:
		int foo(int i) const // nonvirtual
		{
			cout << "Hello World! foo is called." << endl;
			return i;
		}
	};
	
	
	// this is a class you should create to 'mock' Sample
	class MockSample
	{
	public:
		MOCK_CONST_METHOD1(foo, int(int i));
	};

	
	// Another class being tested: TestedClass, normally would be defined in the code. It uses the class Sample.
	template <class SampleType>
	class TestedClass
	{
	public:
		void useSample(SampleType& sample, int i)
		{
			cout << sample.foo(i) << endl;
		}
	};
	
	
//	// The actual test.
//	TEST(SampleTest, FooReturnsI)
//	{
//		using testing::Return;
//		using testing::AtLeast;
//		
//		cout << "Testing SampleTest::FooReturnsI" << endl;
//		MockSample mockSample;
//		TestedClass<MockSample> sampleUser;
//		
//		EXPECT_CALL(mockSample, foo(12)).WillOnce(Return(12));
//		EXPECT_CALL(mockSample, foo(1)).Times(AtLeast(1));
//		
//		sampleUser.useSample(mockSample, 1);
//		sampleUser.useSample(mockSample, 12);
//	}
	
} // end namespace.
