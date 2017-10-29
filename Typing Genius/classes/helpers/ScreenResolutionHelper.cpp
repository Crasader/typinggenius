//
//  ScreenResolutionHelper.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 9/26/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include "ScreenResolutionHelper.h"

namespace ac { namespace utilities {
	
	using std::string;
	using std::vector;
	
	// this is good enough for iphone
	const char *AssetRefRoot = "assets";
	const char *ConfigRefRoot = "configs";
	const char *SoundRefRoot = "audio";
	
	const char *AssetRefPhone = "phone";
	const char *AssetRefPhone_2 = "phonex2";
	const char *AssetRefPhone_W = "phonex2w";
	const char *AssetRefTablet = "tablet";
	const char *AssetRefTablet_2 = "tabletx2";
	
	void initializeScreenSizeParameters()
	{
		// these pertain to iPhone and iPad, specifically. Haven't given Android resolution issue much thought for now
		
		// if i figure (through CCEGLView::getFrameSize()) that it's likely a phone, this will be the operating coordinate
		// system to be used.
		const CCSize phoneDesignResolution(480, 320); // useful for iPhone 5+ versions
		const CCSize phoneWideDesignResolution(568, 320); // useful for iPhone 5+ versions
		
		// if tablet-like.
		const CCSize tabletDesignResolution(1024, 768);
		
		CCSize absScreenSz(CCEGLView::sharedOpenGLView()->getFrameSize());
		CCSize designResolution;
		
		// warning: works for iDevices for now
		if (isPhoneFormFactor()) {
			designResolution = isWidePhoneFormFactor() ? phoneWideDesignResolution : phoneDesignResolution;
		} else if (isTabletFormFactor()) {
			designResolution = tabletDesignResolution;
		}
		
		// determine the contentScaleFactor to use
		const float contentScaleFactor(absScreenSz.height / designResolution.height);
		
		LogI << boost::format("The content scaling factor to use: %.2f") % contentScaleFactor;
		LogI << boost::format("The computed design resolution: (%d, %d)") % designResolution.width % designResolution.height;
		
		CCDirector::sharedDirector()->setContentScaleFactor(contentScaleFactor);
		CCEGLView::sharedOpenGLView()->setDesignResolutionSize(designResolution.width, designResolution.height,
															   kResolutionFixedHeight
		/* shorter iphones will get cut off at the sides */);
		
		CCSize visSize(visibleSize());
		CCPoint visOrigin(visibleOrigin());
		
		LogI << boost::format("The visible origin: %.2f, %.2f") % visOrigin.x % visOrigin.y;
		LogI << boost::format("The visible size: %.2f, %.2f") % visSize.width % visSize.height;
		
	}
	
	
	void initializeSearchPathsAndResolutionOrder()
	{
		vector<string> searchPaths;
		searchPaths.push_back(AssetRefRoot);
		searchPaths.push_back(ConfigRefRoot);
		searchPaths.push_back(SoundRefRoot);
		
		CCFileUtils::sharedFileUtils()->setSearchPaths(searchPaths);
		
		// now provide search order based on resolution
		searchPaths.clear();
		
		if (isTabletFormFactor()) {
			// retina or not?
			if (contentScale() > 1.3) {
				searchPaths.push_back(AssetRefTablet_2);
				searchPaths.push_back(AssetRefTablet);
				searchPaths.push_back(AssetRefPhone_2);
				searchPaths.push_back(AssetRefPhone_W);
				searchPaths.push_back(AssetRefPhone);
			} else {
				searchPaths.push_back(AssetRefTablet);
				searchPaths.push_back(AssetRefPhone_2);
				searchPaths.push_back(AssetRefPhone_W);
				searchPaths.push_back(AssetRefPhone);
				searchPaths.push_back(AssetRefTablet_2);
			}
		} else { // phone
			if (contentScale() > 1.3) {
				if (isWidePhoneFormFactor()) {
					searchPaths.push_back(AssetRefPhone_W);
					searchPaths.push_back(AssetRefPhone_2);
					searchPaths.push_back(AssetRefPhone);
				} else {
					searchPaths.push_back(AssetRefPhone_2);
					searchPaths.push_back(AssetRefPhone);
					searchPaths.push_back(AssetRefPhone_W);
				}
				searchPaths.push_back(AssetRefTablet_2);
				searchPaths.push_back(AssetRefTablet);
			} else {
				searchPaths.push_back(AssetRefPhone);
				searchPaths.push_back(AssetRefPhone_2);
				searchPaths.push_back(AssetRefPhone_W);
				searchPaths.push_back(AssetRefTablet_2);
				searchPaths.push_back(AssetRefTablet);
			}
		}
		
		CCFileUtils::sharedFileUtils()->setSearchResolutionsOrder(searchPaths);
	}
}}

