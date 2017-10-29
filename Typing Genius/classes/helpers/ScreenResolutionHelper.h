//
//  ScreenResolutionHelper.h
//  Typing Genius
//
//  Created by Aldrich Co on 9/26/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#pragma once
#include "cocos2d.h"

namespace ac { namespace utilities {
	
	USING_NS_CC;
	
	// sets up some useful variables in CCDirector
	void initializeScreenSizeParameters();
	void initializeSearchPathsAndResolutionOrder();
	
	// currently works only for iPhone and iPad resolutions
	inline bool isTabletFormFactor() {
		CCSize absScreenSz(CCEGLView::sharedOpenGLView()->getFrameSize());
		return (absScreenSz.height == 1536 || absScreenSz.height == 768);
	}
	
	// particularly referring to iPhone 5
	inline bool isWidePhoneFormFactor() {
		CCSize absScreenSz(CCEGLView::sharedOpenGLView()->getFrameSize());
		return absScreenSz.width == 1136;
	}

	inline bool isPhoneFormFactor() { return !isTabletFormFactor(); }
	
	inline float contentScale() { return CCDirector::sharedDirector()->getContentScaleFactor(); }
	
	inline CCSize operatingSpace() { return CCEGLView::sharedOpenGLView()->getDesignResolutionSize(); }
	
	inline CCSize visibleSize() { return CCDirector::sharedDirector()->getVisibleSize(); }
	
	inline CCPoint visibleOrigin() { return CCDirector::sharedDirector()->getVisibleOrigin(); }
	
	inline bool useLargeScale() { return contentScale() >= 1.5; }
	
	// you're assumed to have picked the closest asset base already. Now scale it to match exact requirements
	// Note: intendedSize is in your design (operating) coordinate space. ContentScaleFactor seemingly isn't involved.
	// BEST PRACTICE: prepare the images so they are already at or near the size you want it, and call it with the intended
	// size soon after creating the node. This is recommended even if working exclusively with iOS since the file path
	// resolution system may not have provided the exact image size you hoped for.
	inline void scaleNodeToSize(CCNode *sprite, const CCSize &intendedSize)
	{
		const CCSize &origSize(sprite->getContentSize());
		
		const float ratioX = intendedSize.width / origSize.width;
		const float ratioY = intendedSize.height / origSize.height;
		
		if (ratioX == ratioY) {
			// LogD << "new scale: " << ratioX;
			sprite->setScale(ratioX); // maybe better scaling?
		} else {
			// LogD << "new scale: " << ratioX << ", " << ratioY;
			sprite->setScaleX(ratioX);
			sprite->setScaleY(ratioY);
		}
	}
	
	
	inline void scaleNodeToSize(CCNode *sprite, const CCSize &intendedSizePhone, const CCSize &intendedSizeTablet)
	{
		scaleNodeToSize(sprite, isTabletFormFactor() ? intendedSizeTablet : intendedSizePhone);
	}
	
	
	
}}