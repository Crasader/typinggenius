//
//  VisualEffectsHelper.h
//  Typing Genius
//
//  Created by Aldrich Co on 11/28/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#pragma once

#include "cocos2d.h"

namespace ac {
	
	USING_NS_CC;
	using namespace std;
		
	class VisualEffectsHelper : public CCObject
	{
	public:
		/**
		 *	@brief Note: should only be run on CCLabelBMFonts! To use the full version you must supply all 3 params
		 *	@param delay time gap between each value. None inserted at start or end
		 *	@param fmt should have a single %s format argument
		 *	@param enableGrow during animation the label can be made to grow and shrink
		 */
		CCFiniteTimeAction *trickleEffectOnNumericLabel(vector<string> values, float delay = 0.05, bool enableGrow = false);
		
		/**
		 *	@brief animate a progress timer down to zero at a fixed time
		 *	@param duration seconds needed to complete operation?
		 */
		void runDrainEffectOnTimer(CCProgressTimer *progressTimer, float duration);
		
		/**
		 *	@brief performs one unit of "work" on behalf of doTrickleEffectOnNumericLabel
		 *	Node should have been Label or it will crash
		 */
		void setLabelUnitStringCallback(CCNode *node, void *data);


		/** 
		 *	@brief required: min <= max
		 *	@param iterations the upper bound of the number of strings generated (at most max - min + 1 instances)
		 *	@param formatString should have the appropriate single %d or %f format specifier.
		 */
//		static vector<string> stepwiseNumericValuesBasedOnRange(int min, int max, size_t iterations, const string &formatString = "");

		static vector<string> stepwiseNumericValuesBasedOnRange(float min, float max,
																size_t iterations, const string &formatString = "");

		/** 
		 *	@brief this makes the label scroll from right to left. Won't return the headline to original position!
		 */
		static void runGalleryHeadline(CCLabelBMFont *label, const string &labelContent,
									   float duration, float showTimePct);
	};
}