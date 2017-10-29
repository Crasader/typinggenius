//
//  main.m
//  Typing Genius
//
//  Created by Aldrich Co on 7/12/13.
//  Copyright Aldrich Co 2013. All rights reserved.
//

#import <UIKit/UIKit.h>
#include "DebugSettingsHelper.h"

int main(int argc, char *argv[]) {
	
	std::string loggingLevel = ac::DebugSettingsHelper::sharedHelper().
	stringValueForProperty("logging_level_main_app", "DEBUG");
	
	// initialize logging
	FILELog::ReportingLevel() = FILELog::FromString(loggingLevel);
    
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    int retVal = UIApplicationMain(argc, argv, nil, @"AppController");
    [pool release];
    return retVal;
}
