//
//  GlobalNotifTests.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 1/5/14.
//  Copyright (c) 2014 Aldrich Co. All rights reserved.
//

#include <boost/test/unit_test.hpp>
#include "Notif.h"

namespace ac {
	
	struct SampleStruct1
	{
		int i;
		float f;
	};
	
	// A Notif Listener subclass.
	// an example of how notifications should be handled
	class SampleNotifListener : public NotifListener
	{
	public:
		
		SampleNotifListener() : i(0), f(0) {}
		
		// the critical part
		void notifCallback(const string &code, std::shared_ptr<void> data)
		{
			if (code == "GlobalNotifTests_SampleEvent1") {
				std::shared_ptr<SampleStruct1> ss = std::static_pointer_cast<SampleStruct1>(data);
				if (ss) {
					this->i = ss->i;
					this->f = ss->f;
				}
			} // ...
		}
		
		inline int getI() const { return i; }
		inline float getF() const { return f; }
		
	private:
		int i;
		float f;
	};
	
	
	struct GlobalNotifTestFixture
	{
		GlobalNotifTestFixture() {
		}
		
		~GlobalNotifTestFixture() {
		}
		
		SampleNotifListener listener;
	};
		
	
	BOOST_FIXTURE_TEST_SUITE(GlobalNotifTests, GlobalNotifTestFixture)
	
	BOOST_AUTO_TEST_CASE(NotificationsAreProcessedCorrectly)
	{
		// create a struct, put it on a smart-pointer
		std::shared_ptr<SampleStruct1> sap(new SampleStruct1);
		sap->i = 3;
		sap->f = 33.1;
		Notif::send("GlobalNotifTests_SampleEvent1", sap);
		
		// prove that listener objects got the message. how?
		BOOST_REQUIRE_EQUAL(listener.getF(), sap->f);
		BOOST_REQUIRE_EQUAL(listener.getI(), sap->i);
	}
	
	
	BOOST_AUTO_TEST_CASE(NotificationCodesNotHandledAreIgnored)
	{
		// create a struct, put it on a smart-pointer
		std::shared_ptr<SampleStruct1> sap(new SampleStruct1);
		sap->i = 3;
		sap->f = 33.1;
		Notif::send("GlobalNotifTests_SampleEvent2", sap); // listener didn't implement this
		
		// prove that listener objects got the message. how?
		BOOST_REQUIRE_EQUAL(listener.getF(), 0);
		BOOST_REQUIRE_EQUAL(listener.getI(), 0);
	}
	
	BOOST_AUTO_TEST_SUITE_END()
}
