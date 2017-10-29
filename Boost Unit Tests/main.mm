//
//  main.mm
//
//  Created by Aldrich Co on 9/13/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#define BOOST_TEST_MODULE "Boost Unit Tests"
#define BOOST_TEST_ALTERNATIVE_INIT_API
#include <boost/test/unit_test.hpp>

#include "cocos2d.h"
#include "EAGLView.h"
#import <UIKit/UIKit.h>

boost::unit_test::test_suite* init_unit_test_suite( int argc, char* argv[] )
{
	FILELog::ReportingLevel() = FILELog::FromString("WARNING");
	
	// the following statements are necessary to initialize the cocos2d context (otherwise some subsequent operations
	// done in tests may fail.
	CGRect bounds = [[UIScreen mainScreen] bounds];
	// swap the width and height around for simulated landscape.
	
	float heightTemp = bounds.size.height;
	bounds.size.height = bounds.size.width;
	bounds.size.width = heightTemp;
	
    [EAGLView viewWithFrame:bounds
				pixelFormat:kEAGLColorFormatRGBA8
				depthFormat:GL_DEPTH_COMPONENT16
		 preserveBackbuffer:NO
				 sharegroup:nil
			  multiSampling:NO
			numberOfSamples:0];
    cocos2d::CCApplication::sharedApplication()->run();
	return nullptr;
}
